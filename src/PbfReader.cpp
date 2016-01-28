#include <iostream>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <cstdint>
#include <cstdlib>
#include <string>
#include <cstring>
#include <list>
#include <netinet/in.h>
#include <boost/shared_array.hpp>
#include <boost/lexical_cast.hpp>
#include <osmpbf/osmpbf.h>
#include <zlib.h>
#include "PbfReader.hpp"
#include "DatablockWorklist.hpp"

namespace osmpbf2apidb
{
    PbfReader::PbfReader(const std::string& pbfFilename ):
        m_pbfFileSizeInBytes(0),
        m_pMemoryMappedBuffer(nullptr)
    {
        // Open file
        int fd = -1;

        if ( (fd = open( pbfFilename.c_str(), O_RDONLY) ) == -1 )
        {
            throw ( "Could not open" + pbfFilename );
        }

        // Run stat to get filesize
        struct stat sb;

        if ( fstat(fd, &sb) == -1 )
        {
            throw ("Could not run fstat on " + pbfFilename );
        }

        m_pbfFileSizeInBytes = sb.st_size;

        //std::cout << "File is " << m_pbfFileSizeInBytes << " bytes long" << std::endl;

        // Memmap file into our program's address space
        if ( (m_pMemoryMappedBuffer = reinterpret_cast<char*>(
                                          mmap(0, m_pbfFileSizeInBytes, PROT_READ, MAP_SHARED, fd, 0))) == MAP_FAILED )
        {
            throw ( "Could not mmap " + pbfFilename );
        }

        // Can close file descriptor, not needed anymore that file is in our address space
        if ( close(fd) == -1 )
        {
            throw ( "Could not close file descriptor after mapping file" );
        }

        std::cout << "Bytes in file: " << getFileSizeInBytes() << std::endl;
    }

    void PbfReader::generateDatablockWorklists(
        boost::shared_array<DatablockWorklist>    pWorklists,
        const unsigned int                       numWorklists )
    {
        char*       pCurrentBufferCursor = m_pMemoryMappedBuffer;
        uint32_t*   pBlobHeaderLength = reinterpret_cast<uint32_t*>
                                        (pCurrentBufferCursor);

        // Find out how many bytes in the blob header
        //std::cout << "BlobHeader length: " << ntohl(*pBlobHeaderLength) << std::endl;

        // Move pointer to next piece of data
        pCurrentBufferCursor += sizeof(uint32_t);

        // Read blobheader
        OSMPBF::BlobHeader blobHeader;

        if ( blobHeader.ParseFromArray(pCurrentBufferCursor,
                                       ntohl(*pBlobHeaderLength)) == false )
        {
            throw ( "Unable to parse blob header" );
        }

        if ( blobHeader.type() != "OSMHeader" )
        {
            throw ( "File did not start with an OSMHeader section" );
        }

        std::cout << std::endl << "OSM File Header (" << ntohl(
                      *pBlobHeaderLength) << " bytes)" << std::endl;
        std::cout << "\tHas index data: " << blobHeader.has_indexdata() << std::endl;
        std::cout << "\tData size: " << blobHeader.datasize() << std::endl;

        // fast forward past header
        pCurrentBufferCursor += ntohl(*pBlobHeaderLength) + blobHeader.datasize();

        unsigned int currWorklist = 0;

        // Iterate over datablocks
        while ( pCurrentBufferCursor < m_pMemoryMappedBuffer + getFileSizeInBytes() )
        {
            /*
            std::cout << std::endl << std::endl << "Trying to read datablock starting at offset 0x" << std::hex <<
            _calculateFileOffset(pCurrentBufferCursor) << std::endl;
            */

            pBlobHeaderLength = reinterpret_cast<uint32_t*>(pCurrentBufferCursor);

            // Move pointer to next piece of data (start of header data)
            pCurrentBufferCursor += sizeof(uint32_t);

            // Read blobheader
            OSMPBF::BlobHeader blobHeader;

            /*
            std::cout << "Trying to read datablock header from offset 0x" << std::hex <<
                _calculateFileOffset(pCurrentBufferCursor) << std::endl;
            */

            if ( blobHeader.ParseFromArray(pCurrentBufferCursor,
                                           ntohl(*pBlobHeaderLength)) == false )
            {
                throw ( "Unable to parse section header" );
            }

            if ( blobHeader.type() != "OSMData" )
            {
                throw ( "Unknown datablock type \"" + blobHeader.type() + "\"" );
            }

            std::cout << std::endl << "OSMData section\n\tOffset: 0x" << std::hex <<
                      _calculateFileOffset(pCurrentBufferCursor) << std::endl << "\tHeader: "
                      << std::dec << ntohl(*pBlobHeaderLength) << " bytes" << std::endl <<
                      "\tPayload: " << blobHeader.datasize() << " bytes" << std::endl;

            const DatablockWorklist::CompressedDatablock newDatablock =
            {
                _calculateFileOffset(pCurrentBufferCursor + ntohl(*pBlobHeaderLength)),     // start offset
                _calculateFileOffset(pCurrentBufferCursor + ntohl(*pBlobHeaderLength) + blobHeader.datasize() - 1),
                blobHeader.datasize()
            };

            pWorklists[currWorklist].addDatablock(newDatablock);

            std::cout << "\tAdded datablock from offset 0x" << std::hex <<
                      newDatablock.offsetStart << " to 0x" <<
                      newDatablock.offsetEnd << " (" <<
                      std::dec << newDatablock.sizeInBytes << " bytes) to worklist " <<
                      boost::lexical_cast<std::string>(currWorklist) << std::endl;

            // Update current worklist, wrapping back to zero if we've hit the last one
            currWorklist = (currWorklist + 1) % numWorklists;

            pCurrentBufferCursor += ntohl(*pBlobHeaderLength) + blobHeader.datasize();
        }

        std::cout << std::endl << std::endl << "Stopped reading at offset 0x"
                  << std::hex << _calculateFileOffset(pCurrentBufferCursor) << std::endl;
    }



    std::list<int> PbfReader::getOsmEntitiesFromCompressedDatablock(
        const DatablockWorklist::CompressedDatablock&   compressedData )
    {
        OSMPBF::Blob currDataPayload;

        if ( currDataPayload.ParseFromArray(m_pMemoryMappedBuffer +
                                            compressedData.offsetStart, compressedData.sizeInBytes) ==
                false )
        {
            throw ( "Could not read data payload" );
        }

        std::cout << "\tRead payload successfully!" << std::endl;

        const std::size_t inflatedSize = currDataPayload.raw_size();

        std::cout << "\tCompressed payload: " <<
                  boost::lexical_cast<std::string>(compressedData.sizeInBytes) <<
                  ", inflated data size: " <<
                  boost::lexical_cast<std::string>(inflatedSize) <<
                  std::endl;

        unsigned char* pDecompressedPayload = new unsigned char[inflatedSize];

        ::std::memset( pDecompressedPayload, 0, inflatedSize);

        _inflateCompressedPayload( currDataPayload,
                                   pDecompressedPayload );

        // Now need to convert decompressed data to a primitive block
        OSMPBF::PrimitiveBlock primitiveBlock;

        if ( primitiveBlock.ParseFromArray(pDecompressedPayload, inflatedSize) ==
                false )
        {
            throw ( "Could not decode decompressed data to primitive block" );
        }

        std::cout << "\tSuccessfully decoded primitive block" << std::endl;



        delete [] pDecompressedPayload;
        pDecompressedPayload = nullptr;

        std::list<int> myList;

        return myList;
    }

    PbfReader::~PbfReader()
    {
        munmap( m_pMemoryMappedBuffer, m_pbfFileSizeInBytes );
        m_pMemoryMappedBuffer = nullptr;
        m_pbfFileSizeInBytes = 0;
    }

    std::uint64_t PbfReader::getFileSizeInBytes() const
    {
        return m_pbfFileSizeInBytes;
    }

    std::uint64_t PbfReader::_calculateFileOffset(
        char const* const   pFilePtr) const
    {
        return (pFilePtr - m_pMemoryMappedBuffer);
    }

    void PbfReader::_inflateCompressedPayload(
        const OSMPBF::Blob&     currDataPayload,
        unsigned char*          pInflateBuffer )
    {
        const std::size_t inflatedSize = currDataPayload.raw_size();

        // Leverage zlib to perform inflate
        z_stream        zlibStream;
        zlibStream.zalloc   = Z_NULL;
        zlibStream.zfree    = Z_NULL;
        zlibStream.opaque   = Z_NULL;
        inflateInit(&zlibStream);
        zlibStream.next_in = (Bytef*)currDataPayload.zlib_data().data();
        zlibStream.avail_in = currDataPayload.zlib_data().size();
        zlibStream.avail_out = inflatedSize;
        zlibStream.next_out = pInflateBuffer;
        const int inflateResult = inflate(&zlibStream, Z_NO_FLUSH);
        inflateEnd(&zlibStream);

        if ( inflateResult != Z_STREAM_END )
        {
            throw ( "Not at end of stream when we bailed out" );
        }
    }
}

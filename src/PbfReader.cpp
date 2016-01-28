#include <iostream>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <cstdint>
#include <string>
#include <netinet/in.h>
#include <boost/shared_array.hpp>
#include <osmpbf/osmpbf.h>
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

            const DatablockWorklist::CompressedDatablock newDatablock =
            {
                pCurrentBufferCursor + ntohl(*pBlobHeaderLength),     // Starting byte of payload
                pCurrentBufferCursor + ntohl(*pBlobHeaderLength) + blobHeader.datasize() - 1,
                blobHeader.datasize()
            };

            pWorklists[currWorklist].addDatablock(newDatablock);

            std::cout << "\tAdded datablock from offset 0x" << std::hex <<
                      _calculateFileOffset(newDatablock.pByteStart) << " to 0x" <<
                      _calculateFileOffset(newDatablock.pByteEnd) << " (" <<
                      std::dec << newDatablock.sizeInBytes << " bytes) to worklist " << std::endl;

            // Update current worklist, wrapping back to zero if we've hit the last one
            currWorklist = (currWorklist + 1) % numWorklists;

            std::cout << std::endl << "OSMData section\n\tOffset: 0x" << std::hex <<
                      _calculateFileOffset(pCurrentBufferCursor) << std::endl << "\tHeader: "
                      << std::dec << ntohl(*pBlobHeaderLength) << " bytes" << std::endl <<
                      "\tPayload: "
                      << blobHeader.datasize() << " bytes" << std::endl;

            pCurrentBufferCursor += ntohl(*pBlobHeaderLength) + blobHeader.datasize();
        }

        std::cout << std::endl << std::endl << "Stopped reading at offset 0x"
                  << std::hex << _calculateFileOffset(pCurrentBufferCursor) << std::endl;
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
}

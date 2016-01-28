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
#include <vector>
#include <netinet/in.h>
#include <boost/shared_array.hpp>
#include <boost/lexical_cast.hpp>
#include <utf8.h>
#include <osmpbf/osmpbf.h>
#include <zlib.h>
#include "PbfReader.hpp"
#include "DatablockWorklist.hpp"
#include "OsmEntityPrimitive.hpp"
#include "LonLatCoordinate.hpp"

namespace osmpbf2apidb
{
    PbfReader::PbfReader(const std::string& pbfFilename ):
        m_pbfFileSizeInBytes(0),
        m_pMemoryMappedBuffer(nullptr),
        m_createdOsmElements()
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

        // Don't need buffer anymore, been parsed into object
        delete [] pDecompressedPayload;
        pDecompressedPayload = nullptr;

        std::cout << "\tSuccessfully decoded primitive block" << std::endl;

        _processOsmPrimitiveBlock(primitiveBlock);

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

    void PbfReader::_processOsmPrimitiveBlock(
        const OSMPBF::PrimitiveBlock&   primitiveBlock )
    {
        // NOTE: strings are stored in PBF in UTF8, we store in UTF16 internally,
        //      serializing out to UTF8 again on the way out
        const std::vector<Utf16String> stringList =
            _generateStringList( primitiveBlock );

        /*
         * Per PBF spec, the primitive group will have all its elements in the same
         *  datatype (five possible: Nodes, DenseNodes, Way, Relation, Changeset)
         *
         * Location: http://wiki.openstreetmap.org/wiki/PBF_Format
         * Retrived: 2016-01-28
         *
         * "A PrimitiveGroup MUST NEVER contain different types of objects. So either it contains many Node messages,
         * or a DenseNode message, or many Way messages, or many Relation messages, or many ChangeSet messages. But it
         * can never contain any mixture of those."
         */

        std::cout << "\tPrimitive block contains " <<
                  boost::lexical_cast<std::string>(primitiveBlock.primitivegroup().size()) <<
                  " primitive groups" << std::endl;

        // Iterate over all the primtivegroups in the block
        for ( int primitiveGroupIndex = 0;
                primitiveGroupIndex < primitiveBlock.primitivegroup().size();
                ++primitiveGroupIndex )
        {
            const OSMPBF::PrimitiveGroup& primitiveGroup =
                primitiveBlock.primitivegroup().Get(primitiveGroupIndex);

            if ( primitiveGroup.nodes_size() > 0 )
            {
                std::cout << "\tPrimitive group " <<
                          boost::lexical_cast<std::string>(primitiveGroupIndex + 1) <<
                          " contains (regular) node entries" << std::endl;

                throw ( "Nodes not implemented yet" );

            }
            else if ( primitiveGroup.has_dense() == true )
            {
                std::cout << "\tPrimitive group " <<
                          boost::lexical_cast<std::string>(primitiveGroupIndex + 1) <<
                          " contains dense node entries" << std::endl;

                _processDenseNodes( primitiveGroup.dense(), primitiveBlock );
            }
            else if ( primitiveGroup.ways_size() > 0 )
            {
                std::cout << "\tPrimitive group " <<
                          boost::lexical_cast<std::string>(primitiveGroupIndex + 1) <<
                          " contains way entries" << std::endl;

                throw ( "Ways not implemented yet" );

            }
            else if ( primitiveGroup.relations_size() > 0 )
            {
                std::cout << "\tPrimitive group " <<
                          boost::lexical_cast<std::string>(primitiveGroupIndex + 1) <<
                          " contains relation entries" << std::endl;

                throw ( "Relations not implemented yet" );
            }
            else if ( primitiveGroup.changesets_size() > 0 )
            {
                std::cout << "\tPrimitive group " <<
                          boost::lexical_cast<std::string>(primitiveGroupIndex + 1) <<
                          " contains changeset entries" <<
                          std::endl;

                throw ( "Never seen changesets in any PBF files to date, no code to handle" );
            }
            else
            {
                throw ( "We read a primitive group, but it doesn't contain any entries in any of the valid types" );
            }

            //break;
        }
    }

    std::vector<Utf16String> PbfReader::_generateStringList(
        const OSMPBF::PrimitiveBlock&   primitiveBlock )
    {
        const OSMPBF::StringTable& st = primitiveBlock.stringtable();

        // Can reserve exact space needed for vector up front
        const unsigned int numStrings = st.s_size();

        std::cout << "\tNumber string table entries: " <<
                  boost::lexical_cast<std::string>(numStrings) << std::endl;

        // *** IMPORTANT NOTE ***
        //      PBF Strings are in UTF-8, we convert to UTF-16 internally,
        //      then serialize back out to UTF-8 on the way out
        std::vector<Utf16String> stringList(numStrings);

        Utf16String utf16String;

        for ( unsigned int i = 0; i < numStrings; ++i )
        {
            const std::string currString = st.s().Get(i);

            utf16String.clear();

            try
            {
                utf16String.setFromUtf8Bytes(currString);
            }
            catch ( utf8::invalid_utf8 const& e )
            {
                throw ( "String from string table contained invalid UTF-8 sequence" );
            }

            stringList.push_back(utf16String);
        }

        return stringList;
    }

    void PbfReader::_processDenseNodes(
        const OSMPBF::DenseNodes&       denseNodes,
        const OSMPBF::PrimitiveBlock&   primitiveBlock )
    {
        // Need id, lat, lon parallel lists to be the same size as data isn't sane without it
        const int listSize = denseNodes.id_size();

        if ( (denseNodes.lat_size() != listSize) ||
                (denseNodes.lon_size() != listSize) )
        {
            throw ( "Found dense node entry with unbalanced list sizes" );
        }

        // Request change in capacity for list of generated elements
        m_createdOsmElements.reserve( m_createdOsmElements.size() + listSize );

        // Uses delta encoding (only stores difference in id/location from previous node to save space with
        //      variable-length integers)
        LonLatCoordinate    lonLat(0, 0);
        std::int64_t        id(0);

        for ( int coordIndex = 0; coordIndex < listSize; ++coordIndex )
        {


        }
    }
}

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
#include <algorithm>
#include <iterator>
#include <thread>
#include <netinet/in.h>
#include <boost/shared_array.hpp>
#include <boost/lexical_cast.hpp>
#include <utf8.h>
#include <osmpbf/osmpbf.h>
#include <zlib.h>
#include "PbfReader.hpp"
#include "LonLatCoordinate.hpp"
#include "DatablockWorklist.hpp"
#include "Primitive.hpp"
#include "PrimitiveVisitor.hpp"
#include "Node.hpp"

namespace OsmFileParser
{
    PbfReader::PbfReader():
        m_pbfFileSizeInBytes(0),
        m_pMemoryMappedBuffer(nullptr),
        m_stringTable(),
        m_pPrimitiveVisitor(nullptr),
        m_visitNodes(false),
        m_visitWays(false),
        m_visitRelations(false),
        m_visitChangesets(false)
    {
        ;
    }

    void PbfReader::parse(
        const std::string&                  pbfFilename,
        ::OsmFileParser::PrimitiveVisitor*  pPrimitiveVisitor )
    {
        // Without number of workers specified, use one worker thread
        parse(pbfFilename, pPrimitiveVisitor, 1);
    }

    void PbfReader::parse(
        const std::string&                  pbfFilename,
        ::OsmFileParser::PrimitiveVisitor*  pPrimitiveVisitor,
        const unsigned int numberOfWorkerThreads )
    {
        m_pPrimitiveVisitor = pPrimitiveVisitor;
        m_visitNodes = m_pPrimitiveVisitor->shouldVisitNodes();

        _memoryMapPbfFile(pbfFilename);

        std::vector<DatablockWorklist> worklists =
            _generateDatablockWorklists(numberOfWorkerThreads);

        std::vector<std::thread> workerThreads(numberOfWorkerThreads);

        for ( unsigned int i = 0; i < numberOfWorkerThreads; ++i )
        {
            /*
            workerThreads[i] = std::thread(
                _processWorklist, i, std::ref(worklists[i]));
            */
            _processWorklist(i, worklists.at(i));
        }

        for ( unsigned int i = 0; i < numberOfWorkerThreads; ++i )
        {
            //workerThreads[i].join();
        }
    }

    void PbfReader::_memoryMapPbfFile(
        const std::string&  pbfFilename )
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

        std::cout << "Memory map successful, file size: " << getFileSizeInBytes() <<
                  std::endl;
    }

    std::vector<DatablockWorklist> PbfReader::_generateDatablockWorklists(
        const unsigned int                       numWorklists )
    {
        char*       pCurrentBufferCursor = m_pMemoryMappedBuffer;
        uint32_t*   pBlobHeaderLength =
            reinterpret_cast<uint32_t*>(pCurrentBufferCursor);
        std::vector<DatablockWorklist> pWorklists(numWorklists);

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
            std::cout << std::endl << std::endl <<
                      "Trying to read datablock starting at offset 0x" << std::hex <<
                      _calculateFileOffset(pCurrentBufferCursor) << std::endl;

            pBlobHeaderLength = reinterpret_cast<uint32_t*>(pCurrentBufferCursor);

            // Move pointer to next piece of data (start of header data)
            pCurrentBufferCursor += sizeof(uint32_t);

            // Read blobheader
            OSMPBF::BlobHeader blobHeader;

            std::cout << "Trying to read datablock header from offset 0x" << std::hex <<
                      _calculateFileOffset(pCurrentBufferCursor) << std::endl;

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

            pWorklists.at(currWorklist).addDatablock(newDatablock);

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

        return pWorklists;
    }


    void PbfReader::_parseCompressedDatablock(
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
        _generateStringTable( primitiveBlock );

        /**
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

        // Iterate over all the primitivegroups in the block
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

                if ( m_pPrimitiveVisitor->shouldVisitNodes() == true )
                {
                    throw ( "Nodes not implemented yet" );
                }
                else
                {
                    std::cout << "\t\tSkipping as visitor doesn't care about nodes" << std::endl;
                }
            }
            else if ( primitiveGroup.has_dense() == true )
            {
                std::cout << "\tPrimitive group " <<
                          boost::lexical_cast<std::string>(primitiveGroupIndex + 1) <<
                          " contains dense node entries" << std::endl;

                if ( m_pPrimitiveVisitor->shouldVisitNodes() == true )
                {
                    _processDenseNodes( primitiveGroup.dense(), primitiveBlock );
                }
                else
                {
                    std::cout << "\t\tSkipping as visitor doesn't care about nodes" << std::endl;
                }
            }
            else if ( primitiveGroup.ways_size() > 0 )
            {
                std::cout << "\tPrimitive group " <<
                          boost::lexical_cast<std::string>(primitiveGroupIndex + 1) <<
                          " contains way entries" << std::endl;

                if ( m_pPrimitiveVisitor->shouldVisitWays() == true )
                {
                    throw ( "Ways not implemented yet" );
                }
                else
                {
                    std::cout << "\t\tSkipping as visitor doesn't care about ways" << std::endl;
                }

            }
            else if ( primitiveGroup.relations_size() > 0 )
            {
                std::cout << "\tPrimitive group " <<
                          boost::lexical_cast<std::string>(primitiveGroupIndex + 1) <<
                          " contains relation entries" << std::endl;

                if ( m_pPrimitiveVisitor->shouldVisitRelations() == true )
                {
                    throw ( "Relations not implemented yet" );
                }
                else
                {
                    std::cout << "\t\tSkipping as visitor doesn't care about relations" <<
                              std::endl;
                }
            }
            else if ( primitiveGroup.changesets_size() > 0 )
            {
                std::cout << "\tPrimitive group " <<
                          boost::lexical_cast<std::string>(primitiveGroupIndex + 1) <<
                          " contains changeset entries" <<
                          std::endl;

                if ( m_pPrimitiveVisitor->shouldVisitChangesets() == true )
                {
                    throw ( "Changesets not implemented yet");
                }
                else
                {
                    std::cout << "\t\tSkipping as visitor doesn't care about changesets" <<
                              std::endl;
                }
            }
            else
            {
                throw ( "We read a primitive group, but it doesn't contain any entries in any of the valid types" );
            }

            //break;
        }

        // Drop block-specific resources as quickly as possible
        m_stringTable.clear();
    }

    void PbfReader::_generateStringTable(
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
        m_stringTable.clear();
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

            m_stringTable.push_back(utf16String);
        }
    }

    void PbfReader::_processDenseNodes(
        const OSMPBF::DenseNodes&       denseNodes,
        const OSMPBF::PrimitiveBlock&   primitiveBlock )
    {
        // Make sure we have dense info as we need info from it (changeset, timestamp, etc.)
        if ( denseNodes.has_denseinfo() == false )
        {
            throw ( "Found dense nodes entry without accompanying dense info" );
        }

        const OSMPBF::DenseInfo denseInfo = denseNodes.denseinfo();

        // Need all parallel lists to be the same size as data isn't sane without it
        const int listSize = denseNodes.id_size();

        if (
            (denseInfo.version_size()   != listSize) ||
            (denseInfo.timestamp_size() != listSize) ||
            (denseInfo.changeset_size() != listSize) ||
            (denseInfo.uid_size()       != listSize) ||
            (denseInfo.user_sid_size()  != listSize) ||
            (denseNodes.lat_size()      != listSize) ||
            (denseNodes.lon_size()      != listSize)
        )
        {
            throw ( "Found dense node entry with unbalanced list sizes" );
        }

        // Uses delta encoding (only stores difference from corresponding value in
        //      previous node to save space due to variable-length integers in format)
        ::OsmFileParser::OsmPrimitive::Identifier   id(0);
        ::OsmFileParser::OsmPrimitive::Version      version(0);
        ::OsmFileParser::OsmPrimitive::Timestamp    timestamp(0);
        ::OsmFileParser::OsmPrimitive::Identifier   changesetId(0);
        ::OsmFileParser::OsmPrimitive::UserId       userId(0);
        ::std::int32_t                              usernameStringTableIndex(
            0);
        ::OsmFileParser::LonLatCoordinate           lonLat(0, 0);

        for ( int coordIndex = 0; coordIndex < listSize; ++coordIndex )
        {
            id                          += denseNodes.id().Get(coordIndex);
            // Version is absolute value, not a delta
            version                     = denseInfo.version().Get(coordIndex);
            timestamp                   += denseInfo.timestamp().Get(coordIndex);
            changesetId                 += denseInfo.changeset().Get(coordIndex);
            userId                      += denseInfo.uid().Get(coordIndex);
            usernameStringTableIndex    += denseInfo.user_sid().Get(coordIndex);

            lonLat.deltaUpdate(
                denseNodes.lon().Get(coordIndex),
                denseNodes.lat().Get(coordIndex) );

            m_pPrimitiveVisitor->visit(
                ::OsmFileParser::OsmPrimitive::Node(
                    id,
                    version,
                    timestamp,
                    changesetId,
                    userId,
                    m_stringTable.at(usernameStringTableIndex),
                    lonLat) );
        }
    }

    void PbfReader::_processWorklist(
        const unsigned int  workerId,
        DatablockWorklist&  worklist )
    {
        try
        {
            std::cout << "Worker thread " <<
                      boost::lexical_cast<std::string>(workerId) << " started!" <<
                      std::endl;

            while ( worklist.empty() == false )
            {

                // Get next chunk of work
                DatablockWorklist::CompressedDatablock currBlock =
                    worklist.getNextDatablock();

                std::cout << "Worker thread " <<
                          boost::lexical_cast<std::string>(workerId) <<
                          " working datablock starting at offset 0x" << std::hex <<
                          currBlock.offsetStart << std::endl;

                // Parse entities out of compressed block
                _parseCompressedDatablock( currBlock );
            }


            std::cout << "Worker thread " <<
                      boost::lexical_cast<std::string>(workerId) <<
                      " terminating normally!" << std::endl;
        }
        catch ( std::string const&      e )
        {
            std::cerr << "Worker thread threw exception: " << e << std::endl;
        }
        catch ( char const*    e )
        {
            std::cerr << "Worker thread threw exception: " << e << std::endl;
        }
        catch ( ... )
        {
            std::cerr << "Worker thread threw unknown exception" << std::endl;
        }
    }

}

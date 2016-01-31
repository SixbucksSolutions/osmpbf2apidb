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
#include "Way.hpp"
#include "Relation.hpp"

namespace OsmFileParser
{
    PbfReader::PbfReader():
        m_pbfFileSizeInBytes(0),
        m_pMemoryMappedBuffer(nullptr),
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
        //
        // NOTE: do not try to store string table as a member variable of the class.
        //      Think multi-threaded.  Multiple execution threads, each working their
        //      own blocks, the table is block-specific... yeah you get it.
        std::vector<::OsmFileParser::Utf16String> stringTable =
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
                std::cout << "\t\tPrimitive group " <<
                          boost::lexical_cast<std::string>(primitiveGroupIndex + 1) <<
                          " contains (regular) node entries" << std::endl;

                if ( m_pPrimitiveVisitor->shouldVisitNodes() == true )
                {
                    throw ( "Nodes not implemented yet" );
                }
                else
                {
                    std::cout << "\t\t\tSkipping as visitor doesn't care about nodes" <<
                              std::endl;
                }
            }
            else if ( primitiveGroup.has_dense() == true )
            {
                std::cout << "\t\tPrimitive group " <<
                          boost::lexical_cast<std::string>(primitiveGroupIndex + 1) <<
                          " contains dense node entries" << std::endl;

                if ( m_pPrimitiveVisitor->shouldVisitNodes() == true )
                {
                    _processDenseNodes( primitiveGroup.dense(), stringTable );
                }
                else
                {
                    std::cout << "\t\t\tSkipping as visitor doesn't care about nodes" <<
                              std::endl;
                }
            }
            else if ( primitiveGroup.ways_size() > 0 )
            {
                std::cout << "\t\tPrimitive group " <<
                          boost::lexical_cast<std::string>(primitiveGroupIndex + 1) <<
                          " contains way entries" << std::endl;

                if ( m_pPrimitiveVisitor->shouldVisitWays() == true )
                {
                    _processWays( primitiveGroup, stringTable );
                }
                else
                {
                    std::cout << "\t\t\tSkipping as visitor doesn't care about ways" << std::endl;
                }

            }
            else if ( primitiveGroup.relations_size() > 0 )
            {
                std::cout << "\t\tPrimitive group " <<
                          boost::lexical_cast<std::string>(primitiveGroupIndex + 1) <<
                          " contains relation entries" << std::endl;

                if ( m_pPrimitiveVisitor->shouldVisitRelations() == true )
                {
                    _processRelations( primitiveGroup, stringTable );
                }
                else
                {
                    std::cout << "\t\t\tSkipping as visitor doesn't care about relations" <<
                              std::endl;
                }
            }
            else if ( primitiveGroup.changesets_size() > 0 )
            {
                std::cout << "\t\tPrimitive group " <<
                          boost::lexical_cast<std::string>(primitiveGroupIndex + 1) <<
                          " contains changeset entries" <<
                          std::endl;

                if ( m_pPrimitiveVisitor->shouldVisitChangesets() == true )
                {
                    throw ( "Changesets not implemented yet");
                }
                else
                {
                    std::cout << "\t\t\tSkipping as visitor doesn't care about changesets" <<
                              std::endl;
                }
            }
            else
            {
                throw ( "\t\tWe read a primitive group, but it doesn't contain any entries in any of the valid types" );
            }

        }
    }


    ::std::vector<::OsmFileParser::Utf16String> PbfReader::_generateStringTable(
        const OSMPBF::PrimitiveBlock&   primitiveBlock )
    {
        const OSMPBF::StringTable& pbfStringTable = primitiveBlock.stringtable();
        ::std::vector<::OsmFileParser::Utf16String> utf16StringTable;

        // Can reserve exact space needed for vector up front
        const unsigned int numStrings = pbfStringTable.s_size();

        std::cout << "\tNumber string table entries: " <<
                  boost::lexical_cast<std::string>(numStrings) << std::endl;

        // *** IMPORTANT NOTE ***
        //      PBF Strings are in UTF-8, we convert to UTF-16 internally,
        //      then serialize back out to UTF-8 on the way out
        Utf16String utf16String;

        for ( unsigned int i = 0; i < numStrings; ++i )
        {
            const std::string currString = pbfStringTable.s().Get(i);

            utf16String.clear();

            try
            {
                utf16String.setFromUtf8Bytes(currString);
            }
            catch ( utf8::invalid_utf8 const& e )
            {
                throw ( "String from string table contained invalid UTF-8 sequence" );
            }

            utf16StringTable.push_back(utf16String);
        }

        return utf16StringTable;
    }

    void PbfReader::_processDenseNodes(
        const OSMPBF::DenseNodes&                           denseNodes,
        const ::std::vector<::OsmFileParser::Utf16String>&  stringTable  )
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
            (denseInfo.version_size()       != listSize) ||
            (denseInfo.timestamp_size()     != listSize) ||
            (denseInfo.changeset_size()     != listSize) ||
            (denseInfo.uid_size()           != listSize) ||
            (denseInfo.user_sid_size()      != listSize) ||
            (denseNodes.lat_size()          != listSize) ||
            (denseNodes.lon_size()          != listSize)
        )
        {
            throw ( "Found dense node entry with unbalanced list sizes" );
        }

        // Uses delta encoding (only stores difference from corresponding value in
        //      previous node to save space due to variable-length integers in format)
        ::OsmFileParser::OsmPrimitive::Identifier       id(0);
        ::OsmFileParser::OsmPrimitive::Version          version(0);
        ::OsmFileParser::OsmPrimitive::Timestamp        timestamp(0);
        ::OsmFileParser::OsmPrimitive::Identifier       changesetId(0);
        ::OsmFileParser::OsmPrimitive::UserId           userId(0);
        ::std::int32_t                                  usernameStringTableIndex(0);
        ::OsmFileParser::LonLatCoordinate               lonLat(0, 0);
        ::OsmFileParser::OsmPrimitive::PrimitiveTags    tags;

        const int numberKeysVals( denseNodes.keys_vals_size() );
        // Vector of pairs showing **COORD INDEX** -> (tag key/value pair)
        ::std::vector <
        ::std::pair <
        int, ::OsmFileParser::OsmPrimitive::PrimitiveTags >>
                nodeTags;

        // Length of keys_vals entry has to be LONGER than list size.
        //      Every coordinate has at least one entry even if it's blank,
        //      and every key value pair above that adds even more
        if ( numberKeysVals < listSize )
        {
            throw ( "Invalid tags entries for dense nodes" );
        }

        /*
           std::cout << "\t\tDense nodes section contains " << std::dec <<
           denseNodes.keys_vals_size() << " entries in keys_vals" << std::endl;
         */

        // Have to iterate over tags first as they have to be ready to be add into nodes
        int tagCoordIndex(0);
        int keysValsIndex = 0;
        ::OsmFileParser::OsmPrimitive::PrimitiveTags currNodeTags;

        while ( keysValsIndex < numberKeysVals )
        {
            // End of tags for a given node will be signaled by a string ID of 0;
            const int stringId = denseNodes.keys_vals(keysValsIndex);

            if ( stringId == 0 )
            {
                // If there are values in the list of tags for current node, add to
                //      master set.
                if ( currNodeTags.size() > 0 )
                {
                    nodeTags.push_back(
                        std::pair <
                        int,
                        ::OsmFileParser::OsmPrimitive::PrimitiveTags > (
                            tagCoordIndex, currNodeTags) );

                    // Clear out contents of curr tags as we're switching to next node
                    currNodeTags.clear();
                }

                // Added all the tags for current coord, update COORD index
                tagCoordIndex++;

                // Have processed one value from keys_vals, so update that index as well
                keysValsIndex++;
            }
            else
            {
                // Next two values in sequence are string ID of key and value, respectively
                const ::OsmFileParser::Utf16String    key =
                    stringTable.at(stringId);
                const ::OsmFileParser::Utf16String    value =
                    stringTable.at(denseNodes.keys_vals(keysValsIndex + 1));

                /*
                std::cout << "\t\tTrying to parse tag\n" <<
                          "\t\t\tKey   at string table index " << stringId << "\n" <<
                          "\t\t\tValue at string table index " <<
                          denseNodes.keys_vals(keysValsIndex + 1) << "\n";

                std::cout << "\t\tPossible key   string: " <<
                          key.toUtf8() << "\n" <<
                          "\t\tPossible value string: " <<
                          value.toUtf8() << "\n";
                */

                // Create key/tag value
                const ::OsmFileParser::OsmPrimitive::Tag newTag(key, value);
                /*
                std::cout << "\t\tKey   in tag: " << newTag.getKey().toUtf8() << "\n" <<
                          "\t\tValue in tag: " << newTag.getValue().toUtf8() << "\n" <<
                          std::endl;
                */

                currNodeTags.push_back( newTag );

                // We've read two values out of keys vals sequence
                keysValsIndex += 2;
            }
        }

        ::std::vector<::std::pair<int, ::OsmFileParser::OsmPrimitive::PrimitiveTags>>::const_iterator
                tagIterator = nodeTags.begin();

        /*
        std::cout << "\t\tFirst coordinate index with tags: " << std::dec <<
                  tagIterator->first << std::endl;
        */

        // Iterate over node entries in the tables
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

            // Do we have tags for this node?
            if ( coordIndex == tagIterator->first )
            {
                tags = tagIterator->second;
                tagIterator++;
            }

            m_pPrimitiveVisitor->visit(
                ::OsmFileParser::OsmPrimitive::Node(
                    id,
                    version,
                    timestamp,
                    changesetId,
                    userId,
                    stringTable.at(usernameStringTableIndex),
                    tags,
                    lonLat) );

            if ( tags.size() > 0 )
            {
                tags.clear();
            }
        }

        std::cout << "\t\t\tAll " << std::dec << listSize <<
                  " nodes in primitive group visited" << std::endl;
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

                std::cout << "\nWorker thread " <<
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

    void PbfReader::_processWays(
        const OSMPBF::PrimitiveGroup&                       primitiveGroup,
        const ::std::vector<::OsmFileParser::Utf16String>&  stringTable )
    {
        const int numberOfWays = primitiveGroup.ways_size();

        /*
        std::cout << "\t\t\tNumber of ways in primitive group: " <<
                  numberOfWays << std::endl;
        */

        ::OsmFileParser::OsmPrimitive::Identifier       id(0);
        ::OsmFileParser::OsmPrimitive::Version          version(0);
        ::OsmFileParser::OsmPrimitive::Timestamp        timestamp(0);
        ::OsmFileParser::OsmPrimitive::Identifier       changesetId(0);
        ::OsmFileParser::OsmPrimitive::UserId           userId(0);
        ::OsmFileParser::Utf16String                    username;
        ::OsmFileParser::OsmPrimitive::PrimitiveTags    tags;

        for ( int wayIndex = 0; wayIndex < numberOfWays; ++wayIndex )
        {
            const ::OSMPBF::Way currWay = primitiveGroup.ways(wayIndex);

            id = currWay.id();

            // Sanity check data to make sure we have full set of info we need
            if ( currWay.has_info() == false )
            {
                throw ( "Way section does not have info section which we need" );
            }

            if ( _processPrimitiveInfo(
                        stringTable,
                        currWay.info(),
                        version,
                        timestamp,
                        changesetId,
                        userId,
                        username) == false )
            {
                throw ( "Optional info section for way is missing data we need" );
            }

            if ( currWay.keys_size() > 0 )
            {
                if ( _parseTags(
                            stringTable, currWay.keys(), currWay.vals(), tags) == false )
                {
                    throw ( "Could not parse tags for way" );
                }
            }

            // Read node refs for way
            const ::OsmFileParser::OsmPrimitive::Way::WayNodeRefs wayNodeRefs =
                _parseWayNodeRefs(currWay);

            const ::OsmFileParser::OsmPrimitive::Way newWay(
                id,
                version,
                timestamp,
                changesetId,
                userId,
                username,
                tags,
                wayNodeRefs);

            m_pPrimitiveVisitor->visit(newWay);
        }

        std::cout << "\t\t\tAll " << numberOfWays <<
                  " ways in primitive group visited" << std::endl;
    }

    bool PbfReader::_processPrimitiveInfo(
        const ::std::vector<::OsmFileParser::Utf16String>&  stringTable,
        const OSMPBF::Info&                                 infoBlock,

        ::OsmFileParser::OsmPrimitive::Version&             version,
        ::OsmFileParser::OsmPrimitive::Timestamp&           timestamp,
        ::OsmFileParser::OsmPrimitive::Identifier&          changesetId,
        ::OsmFileParser::OsmPrimitive::UserId&              userId,
        ::OsmFileParser::Utf16String&                       username )
    {
        if (
            (infoBlock.has_version() == false ) ||
            (infoBlock.has_timestamp() == false) ||
            (infoBlock.has_changeset() == false) ||
            (infoBlock.has_uid() == false) ||
            (infoBlock.has_user_sid() == false) )
        {
            return false;
        }

        version         = infoBlock.version();
        timestamp       = infoBlock.timestamp();
        changesetId     = infoBlock.changeset();
        userId          = infoBlock.uid();
        username        = stringTable.at(infoBlock.user_sid());

        return true;
    }

    bool PbfReader::_parseTags(
        const ::std::vector<::OsmFileParser::Utf16String>&  stringTable,

        const ::google::protobuf::RepeatedField <
        ::google::protobuf::uint32 > & keys,

        const ::google::protobuf::RepeatedField <
        ::google::protobuf::uint32 > & values,

        ::OsmFileParser::OsmPrimitive::PrimitiveTags&   tags )
    {
        // Sanity check the parallel lists
        if ( (keys.size() != values.size()) )
        {
            return false;
        }

        tags.clear();
        tags.reserve(keys.size());

        for ( int tagIndex = 0; tagIndex < keys.size(); ++tagIndex )
        {
            tags.push_back(
                ::OsmFileParser::OsmPrimitive::Tag(
                    stringTable.at(keys.Get(tagIndex)),
                    stringTable.at(values.Get(tagIndex))
                )
            );
        }

        return true;
    }

    ::OsmFileParser::OsmPrimitive::Way::WayNodeRefs
    PbfReader::_parseWayNodeRefs(const ::OSMPBF::Way& way )
    {
        const int numWayNodeRefs = way.refs_size();
        ::OsmFileParser::OsmPrimitive::Way::WayNodeRefs wayNodeRefs(
            numWayNodeRefs );

        // Protocol uses delta encoding, have to remember previous value
        ::OsmFileParser::OsmPrimitive::Identifier nodeId(0);

        for (
            int wayNodeRefIndex = 0;
            wayNodeRefIndex < numWayNodeRefs;
            ++wayNodeRefIndex )
        {
            nodeId += way.refs(wayNodeRefIndex);

            wayNodeRefs.at( wayNodeRefIndex ) = nodeId;
        }

        return wayNodeRefs;
    }

    void PbfReader::_processRelations(
        const OSMPBF::PrimitiveGroup&                       primitiveGroup,
        const ::std::vector<::OsmFileParser::Utf16String>&  stringTable )
    {
        const int numberOfRelations = primitiveGroup.relations_size();

        std::cout << "\t\t\tNumber of relations in primitive group: " <<
                  numberOfRelations << std::endl;

        ::OsmFileParser::OsmPrimitive::Identifier       id(0);
        ::OsmFileParser::OsmPrimitive::Version          version(0);
        ::OsmFileParser::OsmPrimitive::Timestamp        timestamp(0);
        ::OsmFileParser::OsmPrimitive::Identifier       changesetId(0);
        ::OsmFileParser::OsmPrimitive::UserId           userId(0);
        ::OsmFileParser::Utf16String                    username;
        ::OsmFileParser::OsmPrimitive::PrimitiveTags    tags;

        for (
            int relationIndex = 0;
            relationIndex < numberOfRelations;
            ++relationIndex )
        {
            const ::OSMPBF::Relation currRelation = primitiveGroup.relations(
                    relationIndex);

            id = currRelation.id();

            // Sanity check data to make sure we have full set of info we need
            if ( currRelation.has_info() == false )
            {
                throw ( "Relation section does not have info section which we need" );
            }

            if ( _processPrimitiveInfo(
                        stringTable,
                        currRelation.info(),
                        version,
                        timestamp,
                        changesetId,
                        userId,
                        username) == false )
            {
                throw ( "Optional info section for relationis missing data we need" );
            }

            if ( currRelation.keys_size() > 0 )
            {
                if ( _parseTags(
                            stringTable, currRelation.keys(), currRelation.vals(), tags) == false )
                {
                    throw ( "Could not parse tags for relation" );
                }
            }

            // TODO: read relation members
            ::OsmFileParser::OsmPrimitive::Relation::RelationMembers relationMembers(
                currRelation.roles_sid_size() );

            if ( _parseRelationMembers(
                        currRelation, stringTable, relationMembers) == false )
            {
                throw "Could not parse relation members";
            }

            const ::OsmFileParser::OsmPrimitive::Relation newRelation(
                id,
                version,
                timestamp,
                changesetId,
                userId,
                username,
                tags,
                relationMembers );

            m_pPrimitiveVisitor->visit(newRelation);

            /*
            std::cout << "\t\t\t\tMembers in relation: " <<
                relationMembers.size() << std::endl;

            break;
            */
        }

        std::cout << "\t\t\tAll " << numberOfRelations <<
                  " relations in primitive group visited" << std::endl;
    }

    bool PbfReader::_parseRelationMembers(
        const OSMPBF::Relation&                                     relation,
        const ::std::vector<::OsmFileParser::Utf16String>&          stringTable,
        ::OsmFileParser::OsmPrimitive::Relation::RelationMembers&   relationMembers )
    {
        const int numRelationMembers = relation.roles_sid_size();

        // Sanity check arrays are parallel
        if (
            (relation.memids_size() != numRelationMembers) ||
            (relation.types_size() != numRelationMembers) )
        {
            return false;
        }

        ::OsmFileParser::OsmPrimitive::Identifier idValue(0);   // Uses delta encoding

        for ( int i = 0; i < numRelationMembers; ++i )
        {
            ::OsmFileParser::OsmPrimitive::Relation::RelationMemberType memberType;

            switch ( relation.types(i) )
            {
                case OSMPBF::Relation_MemberType_NODE :
                    memberType =
                        ::OsmFileParser::OsmPrimitive::Relation::RelationMemberType::NODE;
                    break;

                case OSMPBF::Relation_MemberType_WAY :
                    memberType = ::OsmFileParser::OsmPrimitive::Relation::RelationMemberType::WAY;
                    break;

                case OSMPBF::Relation_MemberType_RELATION :
                    memberType =
                        ::OsmFileParser::OsmPrimitive::Relation::RelationMemberType::RELATION;
                    break;

                default :
                    return false;
                    break;
            }

            idValue += relation.memids(i);

            ::OsmFileParser::OsmPrimitive::Relation::RelationMember currMember =
            {
                stringTable.at(relation.roles_sid(i)),  // member role
                idValue,                                // member id
                memberType                              // member type
            };

            relationMembers.at(i) = currMember;
        }

        return true;
    }
}

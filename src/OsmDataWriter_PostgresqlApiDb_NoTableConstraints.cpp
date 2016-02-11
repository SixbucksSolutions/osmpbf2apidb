#include <iostream>
#include <mutex>
#include <map>
#include <vector>
#include <string>
#include <utility>
#include <memory>
#include <sstream>
#include <fstream>
#include <ctime>
#include <cmath>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include "OsmFileParser/include/Primitive.hpp"
#include "OsmFileParser/include/PrimitiveVisitor.hpp"
#include "OsmFileParser/include/Node.hpp"
#include "OsmFileParser/include/Way.hpp"
#include "OsmFileParser/include/LonLatCoordinate.hpp"
#include "OsmFileParser/include/Utf16String.hpp"

#include "OsmDataWriter_PostgresqlApiDb_NoTableConstraints.hpp"


namespace OsmDataWriter
{
    namespace PostgresqlApiDb
    {
        NoTableConstraints::NoTableConstraints(
            const ::boost::filesystem::path&        sqlDirectory) :

            m_outputDir(sqlDirectory),
            m_workerThreadList(),
            m_fileSectionList(),
            m_workerFileStreamMapsMutex(),
            m_workerFileStreamMaps()
        {
            if ( ::boost::filesystem::is_directory(m_outputDir) ==
                    false )
            {
                throw ( ::std::string("Cannot open output SQL directory ") +
                        m_outputDir.string() );
            }

            if ( ::boost::filesystem::is_empty(m_outputDir) == false )
            {
                throw ( ::std::string("Output SQL directory ") +
                        m_outputDir.string() + " is not empty" );
            }

            _createSectionNameList();
        }

        void NoTableConstraints::visit(
            const ::OsmFileParser::OsmPrimitive::Node& node )
        {
            const unsigned int workerIndex =
                _addWorkerThreadToThreadList();

            FileStreamMap workerFileStreams =
                _getWorkerFileStreamMap(workerIndex);

            // Create table headers if needed
            _createNodeTables(workerIndex, workerFileStreams);

            // Write this primitive to disk
            _writeNodeToTables( node, workerFileStreams );
        }

        void NoTableConstraints::visit(
            const ::OsmFileParser::OsmPrimitive::Way& way )
        {
            const unsigned int workerIndex =
                _addWorkerThreadToThreadList();

            FileStreamMap workerFileStreams =
                _getWorkerFileStreamMap(workerIndex);

            // Create table headers if needed
            _createWayTables(workerIndex, workerFileStreams);

            // Write this primitive to disk
            _writeWayToTables( way, workerFileStreams );
        }

        void NoTableConstraints::visit(
            const ::OsmFileParser::OsmPrimitive::Relation& /*relation*/ )
        {
            const unsigned int workerIndex =
                _addWorkerThreadToThreadList();

            FileStreamMap workerFileStreams =
                _getWorkerFileStreamMap(workerIndex);

            // Create table headers if needed
            _createRelationTables(workerIndex, workerFileStreams);

            // Write this primitive to disk
            //_writeWayToTables( way, workerFileStreams );
        }

        unsigned int NoTableConstraints::_addWorkerThreadToThreadList()
        {
            if ( m_workerThreadList.contains() == false )
            {
                m_workerThreadList.add();

                /*
                std::cout << "Added new worker thread #" <<
                    std::dec << m_workerThreadList.getIndex() <<
                    " with thread hash 0x" << std::hex <<
                    ::std::this_thread::get_id() << std::endl;
                */

                _createFilePointerMap( m_workerThreadList.getIndex() );
            }

            return m_workerThreadList.getIndex();
        }

        void NoTableConstraints::_createFilePointerMap(
            const unsigned int  workerThreadIndex )
        {
            FileStreamMap emptyMap = FileStreamMap(
                                         new ::std::map<::std::string, ::std::shared_ptr<::std::ostream>>);

            ::std::lock_guard<::std::mutex> lockGuard(m_workerFileStreamMapsMutex);

            m_workerFileStreamMaps.insert(
                ::std::make_pair(workerThreadIndex, emptyMap) );

            /*
            ::std::cout << "Added file pointers for thread " << workerThreadIndex <<
                        ::std::endl;
            */
        }

        void NoTableConstraints::_createSectionNameList()
        {
            m_fileSectionList.push_back("sequence_updates");
            m_fileSectionList.push_back("users");
            m_fileSectionList.push_back("changesets");
            m_fileSectionList.push_back("current_nodes");
            m_fileSectionList.push_back("current_node_tags");
            m_fileSectionList.push_back("nodes");
            m_fileSectionList.push_back("node_tags");
            m_fileSectionList.push_back("current_ways");
            m_fileSectionList.push_back("current_way_nodes");
            m_fileSectionList.push_back("current_way_tags");
            m_fileSectionList.push_back("ways");
            m_fileSectionList.push_back("way_nodes");
            m_fileSectionList.push_back("way_tags");
            m_fileSectionList.push_back("current_relations");
            m_fileSectionList.push_back("current_relation_members");
            m_fileSectionList.push_back("current_relation_tags");
            m_fileSectionList.push_back("relations");
            m_fileSectionList.push_back("relation_members");
            m_fileSectionList.push_back("relation_tags");
        }

        NoTableConstraints::FileStreamMap NoTableConstraints::_getWorkerFileStreamMap(
            const unsigned int      workerThreadIndex )
        {
            ::std::lock_guard<::std::mutex> lockGuard(m_workerFileStreamMapsMutex);

            return m_workerFileStreamMaps.at(workerThreadIndex);
        }


        void NoTableConstraints::_createNodeTables(
            const unsigned int                      workerIndex,
            NoTableConstraints::FileStreamMap&      workerFiles )
        {
            if ( workerFiles->count(::std::string("current_nodes")) == 0 )
            {
                //std::cout << "Have to create tables" << std::endl;
                workerFiles->insert(
                    ::std::make_pair( std::string("current_nodes"),
                                      _createTable(
                                          workerIndex,
                                          "current_nodes",
                                          "COPY current_nodes (id, latitude, longitude, "
                                          "changeset_id, visible, \"timestamp\", tile, "
                                          "version) FROM stdin;")) );


                workerFiles->insert(
                    ::std::make_pair( ::std::string("current_node_tags"),
                                      _createTable(
                                          workerIndex,
                                          "current_node_tags",
                                          "COPY current_node_tags (node_id, k, v) "
                                          "FROM stdin;")) );


                workerFiles->insert(
                    ::std::make_pair( ::std::string("nodes" ),
                                      _createTable(
                                          workerIndex,
                                          "nodes",
                                          "COPY nodes (node_id, latitude, longitude, "
                                          "changeset_id, visible, \"timestamp\", tile, "
                                          "version, redaction_id) FROM stdin;")) );

                workerFiles->insert(
                    ::std::make_pair( ::std::string("node_tags"),
                                      _createTable(
                                          workerIndex,
                                          "node_tags",
                                          "COPY node_tags (node_id, version, k, v) "
                                          "FROM stdin;\n")) );
            }

            //std::cout << "Leaving _createNodeTables" << std::endl;
        }

        ::std::shared_ptr<::std::ostream> NoTableConstraints::_createTable(
            const unsigned int      workerIndex,
            const ::std::string&    tableName,
            const ::std::string&    tableSchema )
        {
            // Have to build filename with a stream as
            //      boost::format is not trivial to convert
            //      to a string
            ::std::stringstream filenameStream;
            filenameStream << ::boost::format("%s_%05d.sql") %
                           tableName %
                           workerIndex;

            const ::boost::filesystem::path tablePath(
                m_outputDir / filenameStream.str() );

            ::std::shared_ptr<::std::ostream> tableStream;

            try
            {
                /*
                std::cout << "Trying to open file " << tablePath.string() <<
                    std::endl;
                */

                tableStream = ::std::shared_ptr<::std::ostream>(
                                  new std::ofstream(
                                      tablePath.string(),
                                      ::std::ofstream::binary) );

                // Write UTF-8 byte-order mark
                //
                //      Byte-order mark is Totally optional as UTF-8
                //      can easily be determined by bitstream, but
                //      removes any ambiguity
                const unsigned char utf8BOM[3] = { 0xEF, 0xBB, 0xBF };
                tableStream->write( reinterpret_cast<const char*>(
                                        utf8BOM), sizeof(utf8BOM) );

                *tableStream << tableSchema << std::endl;
            }
            catch ( ... )
            {
                throw ( std::string("Error when trying to create table " ) +
                        tablePath.string() );
            }

            return tableStream;
        }

        void NoTableConstraints::_writeNodeToTables(
            const ::OsmFileParser::OsmPrimitive::Node&  node,
            NoTableConstraints::FileStreamMap&          workerFileStreams)
        {
            const ::OsmFileParser::LonLatCoordinate lonLat =
                node.getLonLat();
            ::std::int_fast32_t lon;
            ::std::int_fast32_t lat;
            lonLat.getLonLat(lon, lat);

            ::std::stringstream nodesStream;
            nodesStream <<
                        ::boost::format("%d\t%d\t%d\t%d\tt\t%s\t%d\t%d\n") %
                        node.getPrimitiveId() %
                        lat %
                        lon %
                        node.getChangesetId() %
                        _generateISO8601(node.getTimestamp()) %
                        _lonLatToTileNumber(lonLat) %
                        node.getVersion();

            _writeToFileStream( "current_nodes", nodesStream.str(),
                                workerFileStreams );

            // Set contents of stream back to empty string
            nodesStream.str( ::std::string() );

            nodesStream <<
                        ::boost::format("%d\t%d\t%d\t%d\tt\t%s\t%d\t%d\t\\N\n") %
                        node.getPrimitiveId() %
                        lat %
                        lon %
                        node.getChangesetId() %
                        _generateISO8601(node.getTimestamp()) %
                        _lonLatToTileNumber(lonLat) %
                        node.getVersion();

            _writeToFileStream( "nodes", nodesStream.str(),
                                workerFileStreams );


            _writeTagsToTable( node,
                               ::std::string("current_node_tags"),
                               ::std::string("%d\t%s\t%s\n"),
                               workerFileStreams );

            _writeTagsToTable( node,
                               node.getVersion(),
                               ::std::string("node_tags"),
                               ::std::string("%d\t%d\t%s\t%s\n"),
                               workerFileStreams );
        }

        ::std::string NoTableConstraints::_generateISO8601(
            const ::OsmFileParser::OsmPrimitive::Timestamp& timestamp ) const
        {
            char conversionBuffer[32];
            ::std::strftime( reinterpret_cast<char*>(&conversionBuffer),
                             sizeof(conversionBuffer),
                             "%F %T",
                             ::std::gmtime(&timestamp) );

            return ::std::string(conversionBuffer);
        }

        unsigned int NoTableConstraints::_lonLatToTileNumber(
            const ::OsmFileParser::LonLatCoordinate&   lonLat ) const
        {
            ::std::int_fast32_t lon;
            ::std::int_fast32_t lat;
            lonLat.getLonLat(lon, lat);
            const double lonDegree =
                ::OsmFileParser::LonLatCoordinate::convertNanodegreeToDegree(
                    lon );
            const double latDegree =
                ::OsmFileParser::LonLatCoordinate::convertNanodegreeToDegree(
                    lat );

            const int lonInt = ::std::round((lonDegree + 180.0) * 65535.0 / 360.0);
            const int latInt = ::std::round((latDegree + 90.0) * 65535.0 / 180.0);

            unsigned int tile = 0;

            for (int i = 15; i >= 0; i--)
            {
                tile = (tile << 1) | ((lonInt >> i) & 1);
                tile = (tile << 1) | ((latInt >> i) & 1);
            }

            return tile;
        }

        void NoTableConstraints::_writeTagsToTable(
            const ::OsmFileParser::OsmPrimitive::Primitive&
            primitive,
            const ::std::string&                tableName,
            const ::std::string&                formatString,
            NoTableConstraints::FileStreamMap&  workerFileStreams )
        {
            ::std::stringstream tagStream;

            const ::OsmFileParser::OsmPrimitive::PrimitiveTags tags =
                primitive.getTags();

            for ( ::OsmFileParser::OsmPrimitive::PrimitiveTags::const_iterator
                    tagsIter = tags.begin(); tagsIter != tags.end(); ++tagsIter )
            {
                tagStream.str( ::std::string() );

                tagStream << ::boost::format(formatString) %
                          primitive.getPrimitiveId() %
                          tagsIter->getKey().toUtf8() %
                          tagsIter->getValue().toUtf8();

                _writeToFileStream( tableName, tagStream.str(),
                                    workerFileStreams );
            }
        }

        void NoTableConstraints::_writeTagsToTable(
            const ::OsmFileParser::OsmPrimitive::Primitive&
            primitive,

            const ::OsmFileParser::OsmPrimitive::Version
            primitiveVersion,

            const ::std::string&                tableName,
            const ::std::string&                formatString,
            NoTableConstraints::FileStreamMap&  workerFileStreams )
        {
            ::std::stringstream tagStream;

            const ::OsmFileParser::OsmPrimitive::PrimitiveTags tags =
                primitive.getTags();

            for ( ::OsmFileParser::OsmPrimitive::PrimitiveTags::const_iterator
                    tagsIter = tags.begin(); tagsIter != tags.end(); ++tagsIter )
            {
                tagStream.str( ::std::string() );

                tagStream << ::boost::format(formatString)  %
                          primitive.getPrimitiveId()        %
                          primitiveVersion                  %
                          tagsIter->getKey().toUtf8()       %
                          tagsIter->getValue().toUtf8();


                _writeToFileStream( tableName, tagStream.str(),
                                    workerFileStreams );
            }
        }

        void NoTableConstraints::_createWayTables(
            const unsigned int                  workerIndex,
            NoTableConstraints::FileStreamMap&  workerFileStreams )
        {
            if ( workerFileStreams->count(::std::string("current_ways")) == 0 )
            {
                //std::cout << "Have to create tables" << std::endl;
                workerFileStreams->insert(
                    ::std::make_pair( std::string("current_ways"),
                                      _createTable(
                                          workerIndex,
                                          "current_ways",
                                          "COPY current_ways (id, changeset_id, "
                                          "\"timestamp\", visible, version) "
                                          "FROM stdin;\n")));

                workerFileStreams->insert(
                    ::std::make_pair( std::string("current_way_tags"),
                                      _createTable( workerIndex, "current_way_tags",
                                                    "COPY current_way_tags (way_id, k, v) FROM stdin;\n")));


                workerFileStreams->insert(
                    ::std::make_pair( std::string("current_way_nodes"),
                                      _createTable( workerIndex, "current_way_nodes",
                                                    "COPY current_way_nodes (way_id, node_id, "
                                                    "sequence_id) FROM stdin;\n")));

                workerFileStreams->insert(
                    ::std::make_pair( std::string("ways"),
                                      _createTable(
                                          workerIndex,
                                          "ways",
                                          "COPY ways (way_id, changeset_id, "
                                          "\"timestamp\", version, visible, "
                                          "redaction_id) FROM stdin;\n")));

                workerFileStreams->insert(
                    ::std::make_pair( std::string("way_tags"),
                                      _createTable( workerIndex, "way_tags",
                                                    "COPY way_tags (way_id, version, k, v) "
                                                    "FROM stdin;\n" )));


                workerFileStreams->insert(
                    ::std::make_pair( std::string("way_nodes"),
                                      _createTable( workerIndex, "way_nodes",
                                                    "COPY way_nodes (way_id, node_id, version, sequence_id) "
                                                    "FROM stdin;\n" )));
            }


        }

        void NoTableConstraints::_writeWayToTables(
            const ::OsmFileParser::OsmPrimitive::Way&   way,
            NoTableConstraints::FileStreamMap&          workerFileStreams )
        {
            ::std::stringstream waysStream;
            waysStream <<
                       ::boost::format("%d\t%d\t%s\tt\t%d\n") %
                       way.getPrimitiveId() %
                       way.getChangesetId() %
                       _generateISO8601(way.getTimestamp()) %
                       way.getVersion();

            _writeToFileStream( "current_ways", waysStream.str(),
                                workerFileStreams );

            waysStream.str( ::std::string() );

            waysStream <<
                       ::boost::format("%d\t%d\t%s\t%d\tt\\N\n")   %
                       way.getPrimitiveId()                        %
                       way.getChangesetId()                        %
                       way.getVersion()                            %
                       _generateISO8601(way.getTimestamp());

            _writeToFileStream( "ways", waysStream.str(),
                                workerFileStreams );

            _writeTagsToTable( way,
                               ::std::string("current_way_tags"),
                               ::std::string("%d\t%s\t%s\n"),
                               workerFileStreams );

            _writeTagsToTable( way,
                               way.getVersion(),
                               ::std::string("way_tags"),
                               ::std::string("%d\t%d\t%s\t%s\n"),
                               workerFileStreams );

            _writeWayNodesToTables( way, workerFileStreams );
        }

        void NoTableConstraints::_writeWayNodesToTables(
            const ::OsmFileParser::OsmPrimitive::Way&   way,
            NoTableConstraints::FileStreamMap&          workerFileStreams )
        {
            const ::OsmFileParser::OsmPrimitive::Way::WayNodeRefs
            wayNodes = way.getWayNodeRefs();

            ::std::stringstream waynodesStream;

            const ::OsmFileParser::OsmPrimitive::Identifier wayId =
                way.getPrimitiveId();

            const ::OsmFileParser::OsmPrimitive::Version wayVersion =
                way.getVersion();

            for ( unsigned int i = 0; i < wayNodes.size(); ++i )
            {
                waynodesStream.str( ::std::string() );
                waynodesStream <<
                               ::boost::format("%d\t%d\t%d\n")  %
                               wayId                            %
                               wayNodes.at(i)                   %
                               (i + 1);

                _writeToFileStream( "current_way_nodes",
                                    waynodesStream.str(), workerFileStreams );


                waynodesStream.str( ::std::string() );

                waynodesStream <<
                               ::boost::format("%d\t%d\t%d\t%d\n") %
                               wayId                               %
                               wayNodes.at(i)                      %
                               wayVersion                          %
                               (i + 1);

                _writeToFileStream( "way_nodes",
                                    waynodesStream.str(), workerFileStreams );
            }
        }

        void NoTableConstraints::_createRelationTables(
            const unsigned int                  workerIndex,
            NoTableConstraints::FileStreamMap&  workerFileStreams )
        {
            if ( workerFileStreams->count(::std::string("current_relations")) == 0 )
            {
                //std::cout << "Have to create tables" << std::endl;
                workerFileStreams->insert(
                    ::std::make_pair( std::string("current_relations"),
                                      _createTable(
                                          workerIndex, "current_relations",
                                          "COPY current_relations (id, "
                                          "changeset_id, \"timestamp\", "
                                          "visible, version) FROM stdin;\n")));

                workerFileStreams->insert(
                    ::std::make_pair( std::string("current_relation_tags"),
                                      _createTable(
                                          workerIndex, "current_relation_tags",
                                          "COPY current_relation_tags "
                                          "(relation_id, k, v) FROM stdin;\n")));


                workerFileStreams->insert(
                    ::std::make_pair( std::string("current_relation_members"),
                                      _createTable(
                                          workerIndex, "current_relation_members",
                                          "COPY current_relation_members "
                                          "(relation_id, member_type, member_id, "
                                          "member_role, sequence_id) FROM stdin;\n")));

                workerFileStreams->insert(
                    ::std::make_pair( std::string("relations"),
                                      _createTable(
                                          workerIndex, "relations",
                                          "COPY relations (relation_id, changeset_id, "
                                          "\"timestamp\", version, visible, "
                                          "redaction_id) FROM stdin;\n")));

                workerFileStreams->insert(
                    ::std::make_pair( std::string("relation_tags"),
                                      _createTable(
                                          workerIndex, "relation_tags",
                                          "COPY relation_tags (relation_id, "
                                          "version, k, v) FROM stdin;\n")));


                workerFileStreams->insert(
                    ::std::make_pair( std::string("relation_members"),
                                      _createTable(
                                          workerIndex, "relation_members",
                                          "COPY relation_members (relation_id, "
                                          "member_type, member_id, member_role, "
                                          "version, sequence_id) FROM stdin;\n" )));
            }
        }

        void NoTableConstraints::_writeRelationToTables(
            const ::OsmFileParser::OsmPrimitive::Relation&  relation,
            NoTableConstraints::FileStreamMap&              workerFileStreams )
        {
            ;
        }
    }
}

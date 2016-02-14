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
#include "WorkerThreadContext.hpp"

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
            m_workerThreadContextsMutex(),
            m_workerThreadContexts()
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

            // Write this primitive to disk
            _writeNodeToTables( node, workerIndex );
        }

        void NoTableConstraints::visit(
            const ::OsmFileParser::OsmPrimitive::Way& way )
        {
            /*
            const unsigned int workerIndex =
                _addWorkerThreadToThreadList();

            FileStreamMap workerFileStreams =
                _getWorkerFileStreamMap(workerIndex);

            // Create table headers if needed
            _createWayTables(workerIndex, workerFileStreams);

            // Write this primitive to disk
            _writeWayToTables( way, workerFileStreams );
            */
        }

        void NoTableConstraints::visit(
            const ::OsmFileParser::OsmPrimitive::Relation& /*relation*/ )
        {
            /*
            const unsigned int workerIndex =
                _addWorkerThreadToThreadList();

            FileStreamMap workerFileStreams =
                _getWorkerFileStreamMap(workerIndex);

            // Create table headers if needed
            _createRelationTables(workerIndex, workerFileStreams);

            // Write this primitive to disk
            //_writeWayToTables( way, workerFileStreams );
            */
        }

        unsigned int NoTableConstraints::_addWorkerThreadToThreadList()
        {
            unsigned int workerThreadIndex;

            if ( m_workerThreadList.contains() == false )
            {
                m_workerThreadList.add();

                /*
                std::cout << "Added new worker thread #" <<
                    std::dec << m_workerThreadList.getIndex() <<
                    " with thread hash 0x" << std::hex <<
                    ::std::this_thread::get_id() << std::endl;
                */

                ::std::lock_guard<::std::mutex> lockGuard(
                    m_workerThreadContextsMutex);

                workerThreadIndex = m_workerThreadList.getIndex();
                ::std::shared_ptr<WorkerThreadContext> workerContext(
                    new WorkerThreadContext() );

                m_workerThreadContexts.insert(
                    ::std::make_pair(workerThreadIndex,
                                     workerContext) );
            }
            else
            {
                workerThreadIndex = m_workerThreadList.getIndex();
            }

            return m_workerThreadList.getIndex();
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

        void NoTableConstraints::_createNodeTables(
            const unsigned int                      workerIndex,
            ::std::shared_ptr<WorkerThreadContext>& workerContext )
        {
            //std::cout << "Have to create tables" << std::endl;
            workerContext->newTable(
                "current_nodes",
                _createTable(
                    workerIndex,
                    "current_nodes",
                    "COPY current_nodes (id, latitude, longitude, "
                    "changeset_id, visible, \"timestamp\", tile, "
                    "version) FROM stdin;") );



            workerContext->newTable(
                "current_node_tags",
                _createTable(
                    workerIndex,
                    "current_node_tags",
                    "COPY current_node_tags (node_id, k, v) "
                    "FROM stdin;") );

            workerContext->newTable(
                "nodes",
                _createTable(
                    workerIndex,
                    "nodes",
                    "COPY nodes (node_id, latitude, longitude, "
                    "changeset_id, visible, \"timestamp\", tile, "
                    "version, redaction_id) FROM stdin;") );


            workerContext->newTable(
                "node_tags",
                _createTable(
                    workerIndex,
                    "node_tags",
                    "COPY node_tags (node_id, version, k, v) "
                    "FROM stdin;\n") );

            //std::cout << "Leaving _createNodeTables" << std::endl;

            workerContext->nodeTablesCreated(true);
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
            const unsigned int                          workerContextIndex )
        {
            ::std::shared_ptr<WorkerThreadContext> workerContext =
                _getWorkerContext(workerContextIndex);

            // Do we need to create tables?
            if ( workerContext->nodeTablesCreated() == false )
            {
                _createNodeTables(workerContextIndex, workerContext );
            }

            const ::OsmFileParser::LonLatCoordinate lonLat =
                node.getLonLat();
            ::std::int_fast32_t lon;
            ::std::int_fast32_t lat;
            lonLat.getLonLat(lon, lat);


            *(workerContext->getTable("current_nodes")) <<
                    node.getPrimitiveId()       << "\t"     <<
                    lat                         << "\t"     <<
                    lon                         << "\t"     <<
                    node.getChangesetId()       << "\tt\t"  << // has visible tag
                    _generateISO8601(
                        node.getTimestamp())    << "\t"     <<
                    _lonLatToTileNumber(lonLat) << "\t"     <<
                    node.getVersion()           <<
                    ::std::endl;

            *(workerContext->getTable("nodes"))         <<
                    node.getPrimitiveId()       << "\t"     <<
                    lat                         << "\t"     <<
                    lon                         << "\t"     <<
                    node.getChangesetId()       << "\tt\t"  << // has visible tag
                    _generateISO8601(
                        node.getTimestamp())    << "\t"     <<
                    _lonLatToTileNumber(lonLat) << "\t"     <<
                    node.getVersion()           << "\t\\N"  <<
                    ::std::endl;

			_writeTags( node, workerContext, "current_node_tags", "node_tags" );
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

        void NoTableConstraints::_writeTags(
            const ::OsmFileParser::OsmPrimitive::Primitive&     primitive,
            ::std::shared_ptr<WorkerThreadContext>&				workerContext,
			const ::std::string&								currentTagsName,
			const ::std::string&								tagsName )
        {
            const ::OsmFileParser::OsmPrimitive::PrimitiveTags tags =
                primitive.getTags();

			::std::shared_ptr<::std::ostream> currentTagsStream = 
				workerContext->getTable( currentTagsName );

			::std::shared_ptr<::std::ostream> tagsStream =
				workerContext->getTable( tagsName );

			const ::OsmFileParser::OsmPrimitive::Identifier id =
				primitive.getPrimitiveId();

			const ::OsmFileParser::OsmPrimitive::Version version =
				primitive.getVersion();

            for ( ::OsmFileParser::OsmPrimitive::PrimitiveTags::const_iterator
                    tagsIter = tags.begin(); tagsIter != tags.end(); ++tagsIter )
            {
				const ::std::string tagKey 		= tagsIter->getKey().toUtf8();
				const ::std::string tagValue	= tagsIter->getValue().toUtf8();
                *currentTagsStream <<
                        id				<< "\t" <<
                        tagKey 			<< "\t" <<
                        tagValue 		<< 
						::std::endl;

                *tagsStream <<
                        id				<< "\t" <<
						version			<< "\t" <<
						tagKey			<< "\t" <<
						tagValue		<<
						::std::endl;
            }
        }

        void NoTableConstraints::_createWayTables(
            const unsigned int                  workerIndex,
            WorkerThreadContext&    workerContext )
        {
            /*
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


            */
        }

        void NoTableConstraints::_writeWayToTables(
            const ::OsmFileParser::OsmPrimitive::Way&   way,
            WorkerThreadContext&                        workerContext )
        {
            /*
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
            */
        }

        void NoTableConstraints::_writeWayNodesToTables(
            const ::OsmFileParser::OsmPrimitive::Way&   way,
            WorkerThreadContext&                        workerContext )
        {
            /*
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
            */
        }

        void NoTableConstraints::_createRelationTables(
            const unsigned int                  workerIndex,
            WorkerThreadContext&    workerContext )
        {
            /*
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
            */
        }

        void NoTableConstraints::_writeRelationToTables(
            const ::OsmFileParser::OsmPrimitive::Relation&  relation,
            WorkerThreadContext&                            workerContext )
        {
            /*
            // Write relations
            ::std::stringstream relationsStream;
            relationsStream <<
                            ::boost::format("%d\t%d\t%s\tt\t%d\n") %
                            relation.getPrimitiveId() %
                            relation.getChangesetId() %
                            _generateISO8601(relation.getTimestamp()) %
                            relation.getVersion();

            _writeToFileStream( "current_relations", relationsStream.str(),
                                workerFileStreams );

            relationsStream.str( ::std::string() );

            relationsStream <<
                            ::boost::format("%d\t%d\t%d\t%s\tt\\N\n")   %
                            relation.getPrimitiveId()                        %
                            relation.getChangesetId()                        %
                            relation.getVersion()                            %
                            _generateISO8601(relation.getTimestamp());

            _writeTagsToTable( relation,
                               ::std::string("current_relation_tags"),
                               ::std::string("%d\t%s\t%s\n"),
                               workerFileStreams );

            _writeTagsToTable( relation,
                               relation.getVersion(),
                               ::std::string("relation_tags"),
                               ::std::string("%d\t%d\t%s\t%s\n"),
                               workerFileStreams );

            // Write relation members
            */
        }

        std::shared_ptr<WorkerThreadContext> NoTableConstraints::_getWorkerContext(
            const unsigned int  workerThreadIndex )
        {
            ::std::lock_guard<::std::mutex> lockGuard(m_workerThreadContextsMutex);

            return m_workerThreadContexts.at(workerThreadIndex);
        }
    }
}

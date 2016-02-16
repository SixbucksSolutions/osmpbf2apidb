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
#include "OsmFileParser/include/Relation.hpp"
#include "OsmFileParser/include/Changeset.hpp"
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
            m_workerThreadContexts(),
            m_changesetsMutex(),
            m_changesets()
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


        NoTableConstraints::~NoTableConstraints()
        {
            // Terminate all open file handles with proper character
            for ( ::std::map < int,
                    ::std::shared_ptr<WorkerThreadContext >>::iterator it =
                        m_workerThreadContexts.begin();
                    it != m_workerThreadContexts.end();
                    ++it )
            {
                ;
            }
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
            const unsigned int workerIndex =
                _addWorkerThreadToThreadList();

            // Write this primitive to disk
            _writeWayToTables( way, workerIndex );
        }

        void NoTableConstraints::visit(
            const ::OsmFileParser::OsmPrimitive::Relation& relation )
        {
            const unsigned int workerIndex =
                _addWorkerThreadToThreadList();

            // Write this primitive to disk
            _writeRelationToTables( relation, workerIndex );
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
                    "FROM stdin;") );

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
            ::std::shared_ptr<WorkerThreadContext>&             workerContext,
            const ::std::string&                                currentTagsName,
            const ::std::string&                                tagsName )
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
                const ::std::string tagKey      = tagsIter->getKey().toUtf8();
                const ::std::string tagValue    = tagsIter->getValue().toUtf8();
                *currentTagsStream <<
                                   id              << "\t" <<
                                   tagKey          << "\t" <<
                                   tagValue        <<
                                   ::std::endl;

                *tagsStream <<
                            id              << "\t" <<
                            version         << "\t" <<
                            tagKey          << "\t" <<
                            tagValue        <<
                            ::std::endl;
            }
        }



        void NoTableConstraints::_createWayTables(
            const unsigned int                          workerIndex,
            ::std::shared_ptr<WorkerThreadContext>&     workerContext )
        {
            workerContext->newTable(
                "current_ways",
                _createTable(
                    workerIndex,
                    "current_ways",
                    "COPY current_ways (id, changeset_id, "
                    "\"timestamp\", visible, version) FROM stdin;"));

            workerContext->newTable(
                "current_way_tags",
                _createTable( workerIndex,
                              "current_way_tags",
                              "COPY current_way_tags (way_id, k, v) FROM stdin;"));

            workerContext->newTable(
                "current_way_nodes",
                _createTable(
                    workerIndex, "current_way_nodes",
                    "COPY current_way_nodes (way_id, node_id, "
                    "sequence_id) FROM stdin;"));

            workerContext->newTable(
                "ways",
                _createTable(
                    workerIndex,
                    "ways",
                    "COPY ways (way_id, changeset_id, \"timestamp\", "
                    "version, visible, redaction_id) FROM stdin;"));

            workerContext->newTable(
                "way_tags",
                _createTable( workerIndex, "way_tags",
                              "COPY way_tags (way_id, version, k, v) "
                              "FROM stdin;"));


            workerContext->newTable(
                "way_nodes",
                _createTable( workerIndex, "way_nodes",
                              "COPY way_nodes (way_id, node_id, version, "
                              "sequence_id) FROM stdin;"));

            workerContext->wayTablesCreated(true);
        }

        void NoTableConstraints::_writeWayToTables(
            const ::OsmFileParser::OsmPrimitive::Way&   way,
            const unsigned int                          workerContextIndex )
        {
            ::std::shared_ptr<WorkerThreadContext> workerContext =
                _getWorkerContext(workerContextIndex);

            // Do we need to create tables?
            if ( workerContext->wayTablesCreated() == false )
            {
                _createWayTables(workerContextIndex, workerContext );
            }

            *(workerContext->getTable("current_ways")) <<
                    way.getPrimitiveId()        << "\t"     <<
                    way.getChangesetId()        << "\t"     <<
                    _generateISO8601(
                        way.getTimestamp())     << "\t"     <<
                    "t"                         << "\t"     << // visible
                    way.getVersion()            <<
                    ::std::endl;

            *(workerContext->getTable("ways"))  <<
                                                way.getPrimitiveId()        << "\t"     <<
                                                way.getChangesetId()        << "\t"     <<
                                                _generateISO8601(
                                                    way.getTimestamp())     << "\t"     <<
                                                way.getVersion()            << "\t"     <<
                                                "t"                         << "\t"     <<  // visible
                                                "\\N"                       <<              // redaction
                                                ::std::endl;

            _writeTags( way, workerContext, "current_way_tags", "way_tags" );

            _writeWayNodesToTables( way, workerContext );
        }

        void NoTableConstraints::_writeWayNodesToTables(
            const ::OsmFileParser::OsmPrimitive::Way&   way,
            ::std::shared_ptr<WorkerThreadContext>&     workerContext )
        {
            const ::OsmFileParser::OsmPrimitive::Way::WayNodeRefs
            wayNodes = way.getWayNodeRefs();

            const ::OsmFileParser::OsmPrimitive::Identifier wayId =
                way.getPrimitiveId();

            const ::OsmFileParser::OsmPrimitive::Version wayVersion =
                way.getVersion();

            ::std::shared_ptr<::std::ostream> currentWayNodesStream =
                workerContext->getTable("current_way_nodes");

            ::std::shared_ptr<::std::ostream> wayNodesStream =
                workerContext->getTable("way_nodes");

            for ( unsigned int i = 0; i < wayNodes.size(); ++i )
            {
                const ::OsmFileParser::OsmPrimitive::Identifier
                currWayNodeId = wayNodes.at(i);

                *currentWayNodesStream  <<
                                        wayId               << "\t" <<
                                        currWayNodeId       << "\t" <<
                                        (i + 1)             <<
                                        ::std::endl;

                *wayNodesStream         <<
                                        wayId               << "\t" <<
                                        currWayNodeId       << "\t" <<
                                        wayVersion          << "\t" <<
                                        (i + 1)             <<
                                        ::std::endl;
            }
        }

        void NoTableConstraints::_createRelationTables(
            const unsigned int                      workerIndex,
            ::std::shared_ptr<WorkerThreadContext>& workerContext )
        {
            workerContext->newTable(
                "current_relations",
                _createTable(
                    workerIndex, "current_relations",
                    "COPY current_relations (id, changeset_id, "
                    "\"timestamp\", visible, version) FROM stdin;"));

            workerContext->newTable(
                "current_relation_tags",
                _createTable(
                    workerIndex, "current_relation_tags",
                    "COPY current_relation_tags "
                    "(relation_id, k, v) FROM stdin;"));


            workerContext->newTable(
                "current_relation_members",
                _createTable(
                    workerIndex, "current_relation_members",
                    "COPY current_relation_members "
                    "(relation_id, member_type, member_id, "
                    "member_role, sequence_id) FROM stdin;"));

            workerContext->newTable(
                "relations",
                _createTable(
                    workerIndex, "relations",
                    "COPY relations (relation_id, changeset_id, "
                    "\"timestamp\", version, visible, "
                    "redaction_id) FROM stdin;"));

            workerContext->newTable(
                "relation_tags",
                _createTable(
                    workerIndex, "relation_tags",
                    "COPY relation_tags (relation_id, "
                    "version, k, v) FROM stdin;"));


            workerContext->newTable(
                "relation_members",
                _createTable(
                    workerIndex, "relation_members",
                    "COPY relation_members (relation_id, "
                    "member_type, member_id, member_role, "
                    "version, sequence_id) FROM stdin;"));

            workerContext->relationTablesCreated(true);
        }

        void NoTableConstraints::_writeRelationToTables(
            const ::OsmFileParser::OsmPrimitive::Relation&  relation,
            const unsigned int                              workerContextIndex )
        {
            ::std::shared_ptr<WorkerThreadContext> workerContext =
                _getWorkerContext(workerContextIndex);

            // Do we need to create tables?
            if ( workerContext->relationTablesCreated() == false )
            {
                _createRelationTables(workerContextIndex, workerContext );
            }

            *(workerContext->getTable("current_relations")) <<
                    relation.getPrimitiveId()       << "\t"     <<
                    relation.getChangesetId()       << "\t"     <<
                    _generateISO8601(
                        relation.getTimestamp())    << "\t"     <<
                    "t"                             << "\t"     << // visible
                    relation.getVersion()           <<
                    ::std::endl;

            *(workerContext->getTable("relations")) <<
                                                    relation.getPrimitiveId()       << "\t"     <<
                                                    relation.getChangesetId()       << "\t"     <<
                                                    _generateISO8601(
                                                            relation.getTimestamp())    << "\t"     <<
                                                    relation.getVersion()           << "\t"     <<
                                                    "t"                             << "\t"     <<  // visible
                                                    "\\N"                           <<              // redaction
                                                    ::std::endl;

            _writeTags( relation, workerContext, "current_relation_tags",
                        "relation_tags" );

            _writeRelationMembersToTables( relation, workerContext );
        }

        std::shared_ptr<WorkerThreadContext> NoTableConstraints::_getWorkerContext(
            const unsigned int  workerThreadIndex )
        {
            ::std::lock_guard<::std::mutex> lockGuard(m_workerThreadContextsMutex);

            return m_workerThreadContexts.at(workerThreadIndex);
        }

        void NoTableConstraints::_writeRelationMembersToTables(
            const ::OsmFileParser::OsmPrimitive::Relation&  relation,
            ::std::shared_ptr<WorkerThreadContext>&         workerContext )
        {
            const ::OsmFileParser::OsmPrimitive::Relation::RelationMembers
            relationMembers = relation.getRelationMembers();

            const ::OsmFileParser::OsmPrimitive::Identifier relationId =
                relation.getPrimitiveId();

            const ::OsmFileParser::OsmPrimitive::Version relationVersion =
                relation.getVersion();

            ::std::shared_ptr<::std::ostream> currentRelationMembersStream =
                workerContext->getTable("current_relation_members");

            ::std::shared_ptr<::std::ostream> relationMembersStream =
                workerContext->getTable("relation_members");

            for ( unsigned int i = 0; i < relationMembers.size(); ++i )
            {
                const ::OsmFileParser::OsmPrimitive::Relation::RelationMember
                currRelationMember = relationMembers.at(i);

                ::std::string relationMemberType;

                switch ( currRelationMember.memberType )
                {
                    case ::OsmFileParser::OsmPrimitive::Relation::RelationMemberType::NODE:
                        relationMemberType = ::std::string("Node");

                        break;

                    case ::OsmFileParser::OsmPrimitive::Relation::RelationMemberType::WAY:
                        relationMemberType = ::std::string("Way");

                        break;

                    case ::OsmFileParser::OsmPrimitive::Relation::RelationMemberType::RELATION:
                        relationMemberType = ::std::string("Relation");

                        break;

                    default:
                        throw ( "Invalid relation member type" );

                        break;
                }

                const ::OsmFileParser::OsmPrimitive::Identifier memberId =
                    currRelationMember.memberId;

                const ::std::string memberRole(currRelationMember.memberRole.toUtf8());

                *currentRelationMembersStream  <<
                                               relationId          << "\t" <<
                                               relationMemberType  << "\t" <<
                                               memberId            << "\t" <<
                                               memberRole          << "\t" <<
                                               (i + 1)             <<
                                               ::std::endl;

                *relationMembersStream  <<
                                        relationId          << "\t" <<
                                        relationMemberType  << "\t" <<
                                        memberId            << "\t" <<
                                        memberRole          << "\t" <<
                                        relationVersion     << "\t" <<
                                        (i + 1)             <<
                                        ::std::endl;
            }
        }
    }
}

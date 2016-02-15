#ifndef _OSMDATAWRITER_POSTGRESQLAPIDB_NOCONSTRAINTS
#define _OSMDATAWRITER_POSTGRESQLAPIDB_NOCONSTRAINTS

#include <cstdint>
#include <mutex>
#include <map>
#include <unordered_map>
#include <vector>
#include <string>
#include <iosfwd>
#include <memory>
#include <boost/filesystem.hpp>
#include "OsmFileParser/include/Primitive.hpp"
#include "OsmFileParser/include/Node.hpp"
#include "OsmFileParser/include/PrimitiveVisitor.hpp"
#include "OsmFileParser/include/Way.hpp"
#include "WorkerThreadList.hpp"
#include "WorkerThreadContext.hpp"

namespace OsmDataWriter
{
    namespace PostgresqlApiDb
    {
        class NoTableConstraints : public ::OsmFileParser::PrimitiveVisitor
        {
            public:
                NoTableConstraints(
                    const ::boost::filesystem::path&    sqlDirectory
                );

                virtual ~NoTableConstraints() { }

                virtual bool shouldVisitNodes() const
                {
                    return true;
                }

                virtual bool shouldVisitWays() const
                {
                    return true;
                }

                virtual bool shouldVisitRelations() const
                {
                    return true;
                }

                virtual bool shouldVisitChangesets() const
                {
                    return true;
                }

                virtual void visit(
                    const ::OsmFileParser::OsmPrimitive::Node& node
                );

                virtual void visit(
                    const ::OsmFileParser::OsmPrimitive::Way&   way
                );

                virtual void visit(
                    const ::OsmFileParser::OsmPrimitive::Relation&
                    relation
                );

            protected:
                ::boost::filesystem::path       m_outputDir;
                WorkerThreadList                m_workerThreadList;
                ::std::vector<::std::string>    m_fileSectionList;
                ::std::mutex                    m_workerThreadContextsMutex;

                ::std::map < int,
                ::std::shared_ptr<WorkerThreadContext >> m_workerThreadContexts;

                void _createSectionNameList();

                unsigned int _addWorkerThreadToThreadList();

                void _createFilePointerMap(
                    const unsigned int  workerThreadIndex
                );

                ::std::shared_ptr<WorkerThreadContext> _getWorkerContext(
                    const unsigned int workerIndex
                );

                void _updateWorkerContext(
                    const unsigned int      workerIndex,
                    WorkerThreadContext&    workerContext
                );

                ::std::shared_ptr<::std::ostream> _createTable(
                    const unsigned int      workerIndex,
                    const ::std::string&    tableName,
                    const ::std::string&    tableSchema
                );

                void _writeToFileStream(
                    const ::std::string&    fileStreamName,
                    const ::std::string&    writeData,
                    WorkerThreadContext&    workerThreadContext
                )
                {
                    //*(workerFileStreams->at(fileStreamName)) << writeData;
                }

                void _createNodeTables(
                    const unsigned int                      workerIndex,
                    ::std::shared_ptr<WorkerThreadContext>& workerThreadContext
                );

                void _writeNodeToTables(
                    const ::OsmFileParser::OsmPrimitive::Node&  node,
                    const unsigned int                          workerContextIndex
                );

                unsigned int _lonLatToTileNumber(
                    const ::OsmFileParser::LonLatCoordinate& lonLat
                ) const;

                ::std::string _generateISO8601(
                    const ::OsmFileParser::OsmPrimitive::Timestamp&     timestamp
                ) const;

                void _writeCurrentTags(
                    const ::OsmFileParser::OsmPrimitive::Primitive&     primitive,
                    ::std::shared_ptr<::std::ostream>                   stream
                );

                void _writeTags(
                    const ::OsmFileParser::OsmPrimitive::Primitive&     primitive,
                    ::std::shared_ptr<WorkerThreadContext>&             workerContext,
                    const ::std::string&                                currentTagsTableName,
                    const ::std::string&                                tagsTableName
                );

                void _createWayTables(
                    const unsigned int                          workerIndex,
                    ::std::shared_ptr<WorkerThreadContext>&     workerThreadContext
                );

                void _writeWayToTables(
                    const ::OsmFileParser::OsmPrimitive::Way&   way,
                    const unsigned int                          workerThreadIndex
                );

                void _writeWayNodesToTables(
                    const ::OsmFileParser::OsmPrimitive::Way&   way,
                    ::std::shared_ptr<WorkerThreadContext>&     workerThreadContext
                );

                void _createRelationTables(
                    const unsigned int                          workerIndex,
                    ::std::shared_ptr<WorkerThreadContext>&     workerContext
                );

                void _writeRelationToTables(
                    const ::OsmFileParser::OsmPrimitive::Relation&  relation,
                    const unsigned int                              workerThreadIndex
                );
        };
    }
}

#endif // _OSMDATAWRITER_POSTGRESQLAPIDB_NOCONSTRAINTS


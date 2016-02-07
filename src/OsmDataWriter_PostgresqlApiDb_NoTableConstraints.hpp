#ifndef _OSMDATAWRITER_POSTGRESQLAPIDB_NOCONSTRAINTS
#define _OSMDATAWRITER_POSTGRESQLAPIDB_NOCONSTRAINTS

#include <cstdint>
#include <mutex>
#include <map>
#include <vector>
#include <string>
#include <iostream>
#include <memory>
#include <boost/filesystem.hpp>
#include "OsmFileParser/include/Primitive.hpp"
#include "OsmFileParser/include/Node.hpp"
#include "OsmFileParser/include/PrimitiveVisitor.hpp"
#include "OsmFileParser/include/Way.hpp"
#include "WorkerThreadList.hpp"

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
                ::std::mutex                    m_filePointersMutex;

                typedef ::std::map < unsigned int,
                        ::std::map<::std::string, ::std::shared_ptr<::std::ostream> >>
                        SqlFilePointers;

                SqlFilePointers                 m_filePointers;

                void _createSectionNameList();

                void _addWorkerThreadToThreadList();

                void _createFilePointerMap(
                    const unsigned int  workerThreadIndex
                );
        };

    }

}

#endif // _OSMDATAWRITER_POSTGRESQLAPIDB_NOCONSTRAINTS


#ifndef _OSMDATAWRITER_POSTGRESQLAPIDB_NOCONSTRAINTS
#define _OSMDATAWRITER_POSTGRESQLAPIDB_NOCONSTRAINTS

#include <cstdint>
#include <mutex>
#include "OsmFileParser/include/Primitive.hpp"
#include "OsmFileParser/include/Node.hpp"
#include "OsmFileParser/include/PrimitiveVisitor.hpp"
#include "OsmFileParser/include/Way.hpp"

namespace OsmDataWriter
{
    namespace PostgresqlApiDb
    {

        class NoTableConstraints : public ::OsmFileParser::PrimitiveVisitor
        {
            public:
                NoTableConstraints();

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
                    return false;
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

                ::std::uint_fast64_t    getVisitedNodes() const
                {
                    return m_nodesVisited;
                }

                ::std::uint_fast64_t    getVisitedWays() const
                {
                    return m_waysVisited;
                }

                ::std::uint_fast64_t    getVisitedRelations() const
                {
                    return m_relationsVisited;
                }

            protected:
                ::std::mutex            m_visitNodeMutex;
                ::std::mutex            m_visitWayMutex;
                ::std::mutex            m_visitRelationMutex;
                ::std::uint_fast64_t    m_nodesVisited;
                ::std::uint_fast64_t    m_waysVisited;
                ::std::uint_fast64_t    m_relationsVisited;
        };

    }

}

#endif // _OSMDATAWRITER_POSTGRESQLAPIDB_NOCONSTRAINTS


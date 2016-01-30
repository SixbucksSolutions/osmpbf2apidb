#ifndef _OSMDATAWRITER_POSTGRESQLAPIDB_NOCONSTRAINTS
#define _OSMDATAWRITER_POSTGRESQLAPIDB_NOCONSTRAINTS

#include "OsmFileParser/include/Primitive.hpp"
#include "OsmFileParser/include/Node.hpp"
#include "OsmFileParser/include/PrimitiveVisitor.hpp"

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
                    return false;
                }

                virtual bool shouldVisitRelations() const
                {
                    return false;
                }

                virtual bool shouldVisitChangesets() const
                {
                    return false;
                }

                virtual void visit( const ::OsmFileParser::OsmPrimitive::Node& node );


        };

    }

}

#endif // _OSMDATAWRITER_POSTGRESQLAPIDB_NOCONSTRAINTS

#ifndef _OSMDATAWRITER_POSTGRESQLAPIDB_NOCONSTRAINTS
#define _OSMDATAWRITER_POSTGRESQLAPIDB_NOCONSTRAINTS

#include "OsmFileParser/include/Node.hpp"

namespace OsmDataWriter
{
    namespace PostgresqlApiDb
    {

        class NoTableConstraints
        {
            public:
                NoTableConstraints();

                virtual ~NoTableConstraints() { }

                void nodeCallback(
                    const ::OsmFileParser::OsmPrimitive::Node& node,
                    const unsigned int workerId
                );
        };

    }

}

#endif // _OSMDATAWRITER_POSTGRESQLAPIDB_NOCONSTRAINTS

#ifndef _OSMDATAWRITER_POSTGRESQLAPIDB_NOCONSTRAINTS
#define _OSMDATAWRITER_POSTGRESQLAPIDB_NOCONSTRAINTS

#include "OsmFileParser/primitives/Node.hpp"

namespace OsmDataWriter
{

    namespace PostgresqlApiDb
    {

        class NoTableConstraints
        {
            public:
                NoTableConstraints();

                ~NoTableConstraints();

                static void pbfParserCallback(
                    const ::OsmFileParser::OsmPrimitive::Node& node
                );
        };

    }

}

#endif // _OSMDATAWRITER_POSTGRESQLAPIDB_NOCONSTRAINTS

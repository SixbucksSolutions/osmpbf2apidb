#include "OsmFileParser/include/Node.hpp"
#include "OsmDataWriter_PostgresqlApiDb_NoTableConstraints.hpp"

namespace OsmDataWriter
{
    namespace PostgresqlApiDb
    {
        NoTableConstraints::NoTableConstraints()
        {
            ;
        }

        void NoTableConstraints::nodeCallback(
            const ::OsmFileParser::OsmPrimitive::Node& node,
            const unsigned int workerId )
        {
            ;
        }



    }
}

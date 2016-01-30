#include "primitives/Node.hpp"
#include "OsmDataWriter_PostgresqlApiDb_NoTableConstraints.hpp"

namespace OsmDataWriter
{
    namespace PostgresqlApiDb
    {
        NoTableConstraints::NoTableConstraints()
        {
            ;
        }

        NoTableConstraints::~NoTableConstraints()
        {
            ;
        }

        void NoTableConstraints::pbfParserCallback(
            const ::OsmFileParser::OsmPrimitive::Node&  node )
        {
            ;
        }
    }
}

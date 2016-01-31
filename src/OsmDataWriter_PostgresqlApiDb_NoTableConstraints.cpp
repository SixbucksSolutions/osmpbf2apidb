#include <iostream>
#include <mutex>
#include "OsmDataWriter_PostgresqlApiDb_NoTableConstraints.hpp"
#include "OsmFileParser/include/Primitive.hpp"
#include "OsmFileParser/include/PrimitiveVisitor.hpp"
#include "OsmFileParser/include/Node.hpp"

namespace OsmDataWriter
{
    namespace PostgresqlApiDb
    {
        NoTableConstraints::NoTableConstraints():
            m_visitDataMutex(),
            m_nodesVisited(0)
        {
            ;
        }

        void NoTableConstraints::visit(
            const ::OsmFileParser::OsmPrimitive::Node& node )
        {
            bool shouldPrint(false);

            // Dedicated scope to limit critical sections
            {
                ::std::lock_guard<::std::mutex> lock( m_visitDataMutex );
                ++m_nodesVisited;
            }

            /*
            if ( node.getTags().size() > 0 )
            {
                shouldPrint = true;
            }
            */


            if ( shouldPrint == true )
            {
                std::cout << std::endl << node.toString() << std::endl;
            }
        }
    }
}

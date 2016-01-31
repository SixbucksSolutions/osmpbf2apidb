#include <iostream>
#include <mutex>
#include "OsmDataWriter_PostgresqlApiDb_NoTableConstraints.hpp"
#include "OsmFileParser/include/Primitive.hpp"
#include "OsmFileParser/include/PrimitiveVisitor.hpp"
#include "OsmFileParser/include/Node.hpp"
#include "OsmFileParser/include/Way.hpp"

namespace OsmDataWriter
{
    namespace PostgresqlApiDb
    {
        NoTableConstraints::NoTableConstraints():
            m_visitNodeMutex(),
            m_visitWayMutex(),
            m_nodesVisited(0),
            m_waysVisited(0)
        {
            ;
        }

        void NoTableConstraints::visit(
            const ::OsmFileParser::OsmPrimitive::Node& node )
        {
            bool shouldPrint(false);

            // Dedicated scope to limit critical sections
            {
                ::std::lock_guard<::std::mutex> lock( m_visitNodeMutex);
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

        void NoTableConstraints::visit(
            const ::OsmFileParser::OsmPrimitive::Way& way )
        {
            bool shouldPrint(false);

            // Dedicated scope to limit critical sections
            {
                ::std::lock_guard<::std::mutex> lock( m_visitWayMutex );
                ++m_waysVisited;
            }

            /*
            if ( (m_waysVisited < 4) )
            {
                shouldPrint = true;
            }

            if ( way.getTags().size() > 0 )
            {
                shouldPrint = true;
            }
            */


            if ( shouldPrint == true )
            {
                std::cout << std::endl << way.toString() << std::endl;
            }
        }

    }
}

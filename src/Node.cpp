#include <cstdint>
#include "Node.hpp"
#include "OsmEntityPrimitive.hpp"
#include "LonLatCoordinate.hpp"

namespace osmpbf2apidb
{
    Node::Node(
        const std::int64_t      id,
        const LonLatCoordinate& lonLatLocation ) :

        m_osmId(id),
        m_coordinates(lonLatLocation)
    {
        ;
    }

    Node::~Node()
    {
        ;
    }

    void Node::writeDataToDbTableFiles()
    {
        ;
    }

}

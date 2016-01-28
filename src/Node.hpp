#ifndef _NODE_H
#define _NODE_H

#include <cstdint>
#include "OsmEntityPrimitive.hpp"
#include "LonLatCoordinate.hpp"

namespace osmpbf2apidb
{
    class Node : public OsmEntityPrimitive
    {
        public:
            Node(
                const std::int64_t      id,
                const LonLatCoordinate& latLonLocation
            );

            virtual ~Node();

            virtual void writeDataToDbTableFiles();

        private:
            std::int64_t        m_osmId;
            LonLatCoordinate    m_coordinates;
    };
}

#endif // _OSMENTITYPRIMITIVE_NODE_H

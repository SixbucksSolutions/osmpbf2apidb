#include "Primitive.hpp"
#include "Node.hpp"

namespace OsmFileParser
{
    namespace OsmPrimitive
    {
        Node::Node(
            const ::OsmFileParser::OsmPrimitive::Identifier     nodeId,
            const ::OsmFileParser::OsmPrimitive::Version        versionNumber,
            const ::OsmFileParser::OsmPrimitive::Timestamp      timestamp,
            const ::OsmFileParser::OsmPrimitive::Identifier     changesetId,
            const ::OsmFileParser::OsmPrimitive::UserId         userId,
            const ::OsmFileParser::Utf16String&                 username,
            const ::OsmFileParser::LonLatCoordinate&            lonLat ) :

            Primitive(nodeId, versionNumber, timestamp, changesetId, userId, username),
            m_lonLat(lonLat)
        {
            ;
        }
    }
}


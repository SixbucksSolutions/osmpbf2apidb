#include <cstdint>
#include <string>
#include <iostream>
#include <boost/lexical_cast.hpp>
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
            const ::OsmFileParser::OsmPrimitive::PrimitiveTags& tags,
            const ::OsmFileParser::LonLatCoordinate&            lonLat ) :

            Primitive(nodeId, versionNumber, timestamp, changesetId, userId, username,
                      tags),
            m_lonLat(lonLat)
        {
            ;
        }


        ::std::string Node::toString() const
        {
            std::int_fast32_t lon;
            std::int_fast32_t lat;
            m_lonLat.getLonLat(lon, lat);
            ::std::string retString("\tNode\n" +
                                    ::OsmFileParser::OsmPrimitive::Primitive::toString() +
                                    "\t\tLatitude  : " +
                                    ::boost::lexical_cast<std::string>
                                    (LonLatCoordinate::convertNanodegreeToDegree(lat)) +
                                    "\n" +
                                    "\t\tLongitude : " +
                                    boost::lexical_cast<std::string>
                                    (LonLatCoordinate::convertNanodegreeToDegree(lon)) +
                                    "\n");

            return retString;
        }
    }
}


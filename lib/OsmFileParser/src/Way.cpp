#include <cstdint>
#include <string>
#include <iostream>
#include <boost/lexical_cast.hpp>
#include "Primitive.hpp"
#include "Way.hpp"

namespace OsmFileParser
{
    namespace OsmPrimitive
    {
        Way::Way(
            const ::OsmFileParser::OsmPrimitive::Identifier     nodeId,
            const ::OsmFileParser::OsmPrimitive::Version        versionNumber,
            const ::OsmFileParser::OsmPrimitive::Timestamp      timestamp,
            const ::OsmFileParser::OsmPrimitive::Identifier     changesetId,
            const ::OsmFileParser::OsmPrimitive::UserId         userId,
            const ::OsmFileParser::Utf16String&                 username,
            const ::OsmFileParser::OsmPrimitive::PrimitiveTags& tags ):

            Primitive(nodeId, versionNumber, timestamp, changesetId, userId, username,
                      tags)
            //m_lonLat(lonLat)
        {
            ;
        }


        ::std::string Way::toString() const
        {
            return Primitive::toString();
        }
    }
}


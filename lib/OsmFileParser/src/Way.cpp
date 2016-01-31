#include <cstdint>
#include <string>
#include <vector>
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
            const ::OsmFileParser::OsmPrimitive::PrimitiveTags& tags,
            const Way::WayNodeRefs&                             wayNodeRefs ):

            Primitive(nodeId, versionNumber, timestamp, changesetId, userId, username,
                      tags),

            m_wayNodeRefs(wayNodeRefs)
        {
            ;
        }

        ::std::string Way::toString() const
        {
            WayNodeRefs nodeRefs = getWayNodeRefs();

            ::std::string wayNodeRefsString;

            for (
                WayNodeRefs::iterator wayNodeRefsIter = nodeRefs.begin();
                wayNodeRefsIter != nodeRefs.end();
                ++wayNodeRefsIter )
            {
                wayNodeRefsString += "\t\t\t\t\t" +
                                     boost::lexical_cast<std::string>(*wayNodeRefsIter) + "\n";
            }


            return "\t\t\tWay:\n" + Primitive::toString() +
                   "\t\t\t\tWay Nodes :\n" +
                   wayNodeRefsString;

        }
    }
}


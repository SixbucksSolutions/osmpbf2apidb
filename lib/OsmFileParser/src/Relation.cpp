#include <cstdint>
#include <string>
#include <vector>
#include <iostream>
#include "Primitive.hpp"
#include "Relation.hpp"

namespace OsmFileParser
{
    namespace OsmPrimitive
    {
        Relation::Relation(
            const ::OsmFileParser::OsmPrimitive::Identifier     nodeId,
            const ::OsmFileParser::OsmPrimitive::Version        versionNumber,
            const ::OsmFileParser::OsmPrimitive::Timestamp      timestamp,
            const ::OsmFileParser::OsmPrimitive::Identifier     changesetId,
            const ::OsmFileParser::OsmPrimitive::UserId         userId,
            const ::OsmFileParser::Utf16String&                 username,
            const ::OsmFileParser::OsmPrimitive::PrimitiveTags& tags ) :

            Primitive(nodeId, versionNumber, timestamp, changesetId, userId, username,
                      tags)
        {
            ;
        }

        ::std::string Relation::toString() const
        {
            return "\t\t\tRelation:\n" + Primitive::toString() +
                   "\t\t\t\tRelation members:\n";
        }
    }
}


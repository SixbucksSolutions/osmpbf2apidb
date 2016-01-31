#include <cstdint>
#include <string>
#include <vector>
#include <iostream>
#include <boost/lexical_cast.hpp>
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
            const ::OsmFileParser::OsmPrimitive::PrimitiveTags& tags,
            const Relation::RelationMembers&                    members ) :

            Primitive(nodeId, versionNumber, timestamp, changesetId, userId, username,
                      tags),

            m_relationMembers(members)
        {
            ;
        }

        ::std::string Relation::toString() const
        {
            ::std::string relationMembersString;

            for (
                RelationMembers::const_iterator relationMemberIter =
                    m_relationMembers.cbegin();
                relationMemberIter != m_relationMembers.cend();
                ++relationMemberIter )
            {
                relationMembersString += std::string("\n") +
                    "\t\t\t\t\tRole: " + relationMemberIter->memberRole.toUtf8() + "\n" +
                    "\t\t\t\t\t  ID: " +
                    boost::lexical_cast<std::string>(
                        relationMemberIter->memberId) + "\n" +
					"\t\t\t\t\tType: ";

				switch ( relationMemberIter->memberType )
				{
					case ::OsmFileParser::OsmPrimitive::Relation::RelationMemberType::NODE:
						relationMembersString += "Node\n";
						break;
					case ::OsmFileParser::OsmPrimitive::Relation::RelationMemberType::WAY:
						relationMembersString += "Way\n";
						break;
					case ::OsmFileParser::OsmPrimitive::Relation::RelationMemberType::RELATION:
						relationMembersString += "Relation\n";
						break;
					default:
						throw( "Invalid member type" );
						break;
				}



            }


            return "\t\t\tRelation:\n" + Primitive::toString() +
                   "\t\t\t\tRelation members:\n" + relationMembersString;
        }
    }
}


#ifndef _RELATION_HPP
#define _RELATION_HPP

#include <string>
#include <vector>
#include "Primitive.hpp"
#include "Utf16String.hpp"

namespace OsmFileParser
{
    namespace OsmPrimitive
    {
        class Relation : public ::OsmFileParser::OsmPrimitive::Primitive
        {
            public:

                enum RelationMemberType
                {
                    NODE        = 0,
                    WAY         = 1,
                    RELATION    = 2
                };

                struct RelationMember
                {
                    ::OsmFileParser::Utf16String                memberRole;
                    ::OsmFileParser::OsmPrimitive::Identifier   memberId;
                    RelationMemberType                          memberType;
                };

                typedef ::std::vector<RelationMember>   RelationMembers;

                Relation(
                    const ::OsmFileParser::OsmPrimitive::Identifier     wayId,
                    const ::OsmFileParser::OsmPrimitive::Version        versionNumber,
                    const ::OsmFileParser::OsmPrimitive::Timestamp      timestamp,
                    const ::OsmFileParser::OsmPrimitive::Identifier     changesetId,
                    const ::OsmFileParser::OsmPrimitive::UserId         userId,
                    const ::OsmFileParser::Utf16String&                 username,
                    const ::OsmFileParser::OsmPrimitive::PrimitiveTags& tags,
                    const RelationMembers&                              members );

                virtual ~Relation() { }

                RelationMembers getRelationMembers() const
                {
                    return m_relationMembers;
                }

                virtual ::std::string toString() const;

            protected:

                RelationMembers m_relationMembers;

        };
    }
}

#endif // _RELATION_HPP


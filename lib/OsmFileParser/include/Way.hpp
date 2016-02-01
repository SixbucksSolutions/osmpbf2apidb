#ifndef _WAY_HPP
#define _WAY_HPP

#include <string>
#include <vector>
#include "Primitive.hpp"
#include "Utf16String.hpp"

namespace OsmFileParser
{
    namespace OsmPrimitive
    {
        class Way : public ::OsmFileParser::OsmPrimitive::Primitive
        {
            public:

                typedef ::std::vector<::OsmFileParser::OsmPrimitive::Identifier> WayNodeRefs;

                Way(
                    const ::OsmFileParser::OsmPrimitive::Identifier     wayId,
                    const ::OsmFileParser::OsmPrimitive::Version        versionNumber,
                    const ::OsmFileParser::OsmPrimitive::Timestamp      timestamp,
                    const ::OsmFileParser::OsmPrimitive::Identifier     changesetId,
                    const ::OsmFileParser::OsmPrimitive::UserId         userId,
                    const ::OsmFileParser::Utf16String&                 username,
                    const ::OsmFileParser::OsmPrimitive::PrimitiveTags& tags,
                    const ::std::vector <
                    ::OsmFileParser::OsmPrimitive::Identifier > &     wayNodeRefs );

                virtual ~Way() { }

                virtual WayNodeRefs getWayNodeRefs() const
                {
                    return m_wayNodeRefs;
                }

                virtual ::std::string toString() const;

            protected:
                WayNodeRefs m_wayNodeRefs;
        };
    }
}

#endif // _WAY_HPP

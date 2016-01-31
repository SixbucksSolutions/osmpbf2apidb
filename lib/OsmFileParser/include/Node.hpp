#ifndef _NODE_HPP
#define _NODE_HPP

#include <string>
#include "Primitive.hpp"
#include "LonLatCoordinate.hpp"
#include "Utf16String.hpp"

namespace OsmFileParser
{
    namespace OsmPrimitive
    {
        class Node : public ::OsmFileParser::OsmPrimitive::Primitive
        {
            public:

                Node(
                    const ::OsmFileParser::OsmPrimitive::Identifier     nodeId,
                    const ::OsmFileParser::OsmPrimitive::Version        versionNumber,
                    const ::OsmFileParser::OsmPrimitive::Timestamp      timestamp,
                    const ::OsmFileParser::OsmPrimitive::Identifier     changesetId,
                    const ::OsmFileParser::OsmPrimitive::UserId         userId,
                    const ::OsmFileParser::Utf16String&                 username,
                    const ::OsmFileParser::OsmPrimitive::PrimitiveTags& tags,
                    const ::OsmFileParser::LonLatCoordinate&            lonLat );

                virtual ~Node() { }

                virtual ::OsmFileParser::LonLatCoordinate getLonLat() const
                {
                    return m_lonLat;
                }

                virtual ::std::string toString() const;

            protected:
                ::OsmFileParser::LonLatCoordinate   m_lonLat;

        };
    }
}

#endif // _NODE_HPP


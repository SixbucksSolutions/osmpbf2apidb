#ifndef _NODE
#define _NODE

#include <string>
#include "Primitive.hpp"
#include "LonLatCoordinate.hpp"

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
                    const ::OsmFileParser::LonLatCoordinate&            lonLat );

                virtual ~Node() { }

                //virtual ::std::string toString() const;

            protected:
                ::OsmFileParser::LonLatCoordinate   m_lonLat;

        };
    }
}

#endif // _NODE


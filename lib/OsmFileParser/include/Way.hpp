#ifndef _WAY_HPP
#define _WAY_HPP

#include <string>
#include "Primitive.hpp"
#include "Utf16String.hpp"

namespace OsmFileParser
{
    namespace OsmPrimitive
    {
        class Way : public ::OsmFileParser::OsmPrimitive::Primitive
        {
            public:

                Way(
                    const ::OsmFileParser::OsmPrimitive::Identifier     wayId,
                    const ::OsmFileParser::OsmPrimitive::Version        versionNumber,
                    const ::OsmFileParser::OsmPrimitive::Timestamp      timestamp,
                    const ::OsmFileParser::OsmPrimitive::Identifier     changesetId,
                    const ::OsmFileParser::OsmPrimitive::UserId         userId,
                    const ::OsmFileParser::Utf16String&                 username,
                    const ::OsmFileParser::OsmPrimitive::PrimitiveTags& tags );

                virtual ~Way() { }

                virtual ::std::string toString() const;

            protected:
                //::OsmFileParser::LonLatCoordinate   m_lonLat;

        };
    }
}

#endif // _WAY_HPP


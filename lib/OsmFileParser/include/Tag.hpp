#ifndef _TAG_HPP
#define _TAG_HPP

#include "Utf16String.hpp"

namespace OsmFileParser
{
    namespace OsmPrimitive
    {
        class Tag
        {
            public:
                Tag(
                    const ::OsmFileParser::Utf16String&   key,
                    const ::OsmFileParser::Utf16String&   value
                );

                virtual ~Tag() { }

                ::OsmFileParser::Utf16String    getKey() const
                {
                    return m_key;
                }

                ::OsmFileParser::Utf16String    getValue() const
                {
                    return m_value;
                }

            protected:
                ::OsmFileParser::Utf16String    m_key;
                ::OsmFileParser::Utf16String    m_value;

        };
    }
}

#endif // _TAG_HPP

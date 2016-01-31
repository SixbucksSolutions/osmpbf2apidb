#include "Utf16String.hpp"
#include "Tag.hpp"

namespace OsmFileParser
{
    namespace OsmPrimitive
    {
        Tag::Tag(
            const ::OsmFileParser::Utf16String&   key,
            const ::OsmFileParser::Utf16String&   value ):

            m_key(key),
            m_value(value)
        {
            ;
        }
    }
}

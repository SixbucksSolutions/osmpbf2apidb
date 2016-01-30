#include "Primitive.hpp"

namespace OsmFileParser
{
    namespace OsmPrimitive
    {
        Primitive::Primitive(
            const Identifier    primitiveId,
            const Version       versionNumber,
            const Timestamp     timestamp,
            const Identifier    changesetId,
            const UserId        userId,
            const Utf16String&  username ):

            m_primitiveId(primitiveId),
            m_versionNumber(versionNumber),
            m_timestamp(timestamp),
            m_changesetId(changesetId),
            m_userId(userId),
            m_username(username)
        {
            ;
        }


    }
}

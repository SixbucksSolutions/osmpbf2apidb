#include <cstdint>
#include "Primitive.hpp"
#include "Changeset.hpp"

namespace OsmFileParser
{
    namespace OsmPrimitive
    {
        Changeset::Changeset(
            const ::OsmFileParser::OsmPrimitive::Identifier changesetId,
            const ::OsmFileParser::OsmPrimitive::Identifier userId ) :

            m_changesetId(changesetId),
            m_userId(userId),
            m_openedAt(UINT64_MAX),
            m_closedAt(0),
            m_changesetChanges(0)
        {
            ;
        }

        void Changeset::updateAccess(
            const ::OsmFileParser::OsmPrimitive::Timestamp accessTime )
        {
            if ( accessTime < m_openedAt )
            {
                m_openedAt = accessTime;
            }

            if ( accessTime > m_closedAt )
            {
                m_closedAt = accessTime;
            }
        }
    }
}

#include <cstdint>
#include "Primitive.hpp"
#include "Changeset.hpp"

namespace OsmFileParser
{
    namespace OsmPrimitive
    {
        Changeset::Changeset(
            const ::OsmFileParser::OsmPrimitive::Identifier changesetId ) :

            m_changesetId(changesetId),
            m_userId(0),
            m_username(),
            m_openedAt(INT64_MAX),
            m_closedAt(0),
            m_changesetChanges(0)
        {
            ;
        }

        void Changeset::updateAccess(
            const ::OsmFileParser::OsmPrimitive::Timestamp accessTime )
        {
            // Ignore invalid times
            if ( accessTime == 0 )
            {
                return;
            }

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

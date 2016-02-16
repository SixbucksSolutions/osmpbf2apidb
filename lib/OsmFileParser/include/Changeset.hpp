#ifndef _CHANGESET_HPP
#define _CHANGESET_HPP

#include <cstdint>
#include "Primitive.hpp"

namespace OsmDataWriter
{
    namespace PostgresqlApiDb
    {
        class Changeset
        {
            public:
                Changeset(
                    const ::OsmFileParser::OsmPrimitive::Identifier
                    changesetId,

                    const ::OsmFileParser::OsmPrimitive::Identifier
                    userId
                );

                virtual ~Changeset() { }


                void updateAccess(
                    const ::OsmFileParser::OsmPrimitive::Timestamp
                    accessTimestamp
                );

                void incrementChanges()
                {
                    m_changesetChanges++;
                }

                ::OsmFileParser::OsmPrimitive::Identifier
                getChangesetId() const
                {
                    return m_changesetId;
                }

                ::OsmFileParser::OsmPrimitive::Identifier
                getUserId() const
                {
                    return m_userId;
                }

                ::OsmFileParser::OsmPrimitive::Timestamp
                getOpenedAt() const
                {
                    return m_openedAt;
                }

                ::OsmFileParser::OsmPrimitive::Timestamp
                getClosedAt() const
                {
                    return m_closedAt;
                }

                ::std::uint64_t getChanges() const
                {
                    return m_changesetChanges;
                }


            protected:
                ::OsmFileParser::OsmPrimitive::Identifier   m_changesetId;
                ::OsmFileParser::OsmPrimitive::Identifier   m_userId;
                ::OsmFileParser::OsmPrimitive::Timestamp    m_openedAt;
                ::OsmFileParser::OsmPrimitive::Timestamp    m_closedAt;
                ::std::uint64_t                 m_changesetChanges;

        };

    }
}
#endif // _CHANGESET_HPP

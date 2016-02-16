#ifndef _CHANGESET_HPP
#define _CHANGESET_HPP

#include <cstdint>
#include "Utf16String.hpp"
#include "Primitive.hpp"

namespace OsmFileParser
{
    namespace OsmPrimitive
    {
        class Changeset
        {
            public:
                Changeset(
                    const ::OsmFileParser::OsmPrimitive::Identifier
                    changesetId
                );

                virtual ~Changeset() { }


                void updateAccess(
                    const ::OsmFileParser::OsmPrimitive::Timestamp
                    accessTimestamp
                );

                void setUserId(
                    const ::OsmFileParser::OsmPrimitive::Identifier
                    userId
                )
                {
                    m_userId = userId;
                }

                void setUsername(
                    const ::OsmFileParser::Utf16String& username
                )
                {
                    m_username.setFromUtf8Bytes(username.toUtf8());
                }


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

                ::OsmFileParser::Utf16String
                getUsername() const
                {
                    return m_username;
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
                ::OsmFileParser::Utf16String                m_username;
                ::OsmFileParser::OsmPrimitive::Timestamp    m_openedAt;
                ::OsmFileParser::OsmPrimitive::Timestamp    m_closedAt;
                ::std::uint64_t                 m_changesetChanges;

        };

    }
}
#endif // _CHANGESET_HPP

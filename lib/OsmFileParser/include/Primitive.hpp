#ifndef _PRIMITIVE
#define _PRIMITIVE

#include <cstdint>
#include <vector>
#include "Utf16String.hpp"
#include "Tag.hpp"

namespace OsmFileParser
{
    namespace OsmPrimitive
    {
        /**
         * OSM identifiers have been 64-bit signed integers since 2012-ish
         *
         * See: http://wiki.openstreetmap.org/wiki/64-bit_Identifiers
         */
        typedef ::std::int64_t Identifier;

        typedef ::std::int32_t Version;

        typedef ::std::int64_t Timestamp;

        typedef ::std::int32_t UserId;

        typedef ::std::vector<::OsmFileParser::OsmPrimitive::Tag>   PrimitiveTags;

        /**
         * Base class for OSM primitive entities
         */
        class Primitive
        {
            public:
                /**
                 * Virtual destructor, must be implemented in child classes
                 */
                virtual ~Primitive() { }

                /**
                 * Get the unique identifier value for the given primititive
                 *
                 * @return Unique identifier for this OSM primitive
                 *
                 * @note Only guaranteed to be unique per primitive type
                 */
                virtual ::OsmFileParser::OsmPrimitive::Identifier getPrimitiveId() const
                {
                    return m_primitiveId;
                }

                /**
                 * Get the version of the primitive
                 *
                 * @return Version number of this primitive
                 */
                virtual ::OsmFileParser::OsmPrimitive::Version getVersion() const
                {
                    return m_versionNumber;
                }

                /**
                 * Get the timestamp for this primitive
                 *
                 * @return Timestamp for creation or last update
                 */
                virtual ::OsmFileParser::OsmPrimitive::Timestamp getTimestamp() const
                {
                    return m_timestamp;
                }

                /**
                 * Get the changeset ID for this primitive
                 *
                 * @return Changeset ID of the given primitive
                 */
                virtual ::OsmFileParser::OsmPrimitive::Identifier getChangesetId() const
                {
                    return m_changesetId;
                }

                /**
                 * Get the User ID for this primitive
                 *
                 * @return User ID for this primitive
                 */
                virtual ::OsmFileParser::OsmPrimitive::UserId getUserId() const
                {
                    return m_userId;
                }

                /**
                 * Get the username for who performed last edit
                 *
                 * @return Username of last editor
                 */
                virtual ::OsmFileParser::Utf16String getUsername() const
                {
                    return m_username;
                }

                /**
                 * Retrieve the tags for this primitive
                 *
                 * @return Tags stored in this primitive
                 */
                virtual ::OsmFileParser::OsmPrimitive::PrimitiveTags    getTags() const
                {
                    return m_tags;
                }

                virtual ::std::string toString() const;

            protected:

                /// Primitive ID
                ::OsmFileParser::OsmPrimitive::Identifier       m_primitiveId;

                /// Version number
                ::OsmFileParser::OsmPrimitive::Version          m_versionNumber;

                /// Timestamp
                ::OsmFileParser::OsmPrimitive::Timestamp        m_timestamp;

                /// Changeset number
                ::OsmFileParser::OsmPrimitive::Identifier       m_changesetId;

                /// User UID
                ::OsmFileParser::OsmPrimitive::UserId           m_userId;

                /// User name
                ::OsmFileParser::Utf16String                    m_username;

                /// Key/value tags
                ::OsmFileParser::OsmPrimitive::PrimitiveTags    m_tags;

                // Constructor to initialize values that child classes can leverage
                Primitive(
                    const Identifier        primitiveId,
                    const Version           versionNumber,
                    const Timestamp         timestamp,
                    const Identifier        changesetId,
                    const UserId            userId,
                    const Utf16String&      username,
                    const PrimitiveTags&    tags
                );


        };
    }
}

#endif // _PRIMITIVE


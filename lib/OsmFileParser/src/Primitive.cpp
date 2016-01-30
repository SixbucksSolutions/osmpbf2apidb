#include <string>
#include <iostream>
#include <boost/lexical_cast.hpp>
#include <boost/date_time.hpp>
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

        ::std::string Primitive::toString() const
        {
            return ::std::string(
                       "\t\tID        : " +
                       ::boost::lexical_cast<::std::string>(getPrimitiveId()) +
                       "\n" +

                       "\t\tVersion   : " +
                       ::boost::lexical_cast<::std::string>(getVersion()) +
                       "\n" +

                       "\t\tTimestamp : " +
                       "\n" +

                       "\t\tChangeset : " +
                       "\n" +

                       "\t\tUser ID   : " +
                       "\n" +

                       "\t\tUsername  : "


                       "\n" );
        }
    }
}

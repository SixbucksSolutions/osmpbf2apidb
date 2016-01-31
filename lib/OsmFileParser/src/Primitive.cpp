#include <string>
#include <sstream>
#include <iostream>
#include <boost/lexical_cast.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/format.hpp>
#include "Primitive.hpp"

namespace OsmFileParser
{
    namespace OsmPrimitive
    {
        Primitive::Primitive(
            const Identifier        primitiveId,
            const Version           versionNumber,
            const Timestamp         timestamp,
            const Identifier        changesetId,
            const UserId            userId,
            const Utf16String&      username,
            const PrimitiveTags&    tags ):

            m_primitiveId(primitiveId),
            m_versionNumber(versionNumber),
            m_timestamp(timestamp),
            m_changesetId(changesetId),
            m_userId(userId),
            m_username(username),
            m_tags(tags)
        {
            ;
        }

        ::std::string Primitive::toString() const
        {
            const ::boost::posix_time::ptime timestamp(
                ::boost::posix_time::from_time_t(getTimestamp()) );

            ::std::stringstream dateStringStream;
            dateStringStream << boost::format(" (%1%-%2%-%3% %4%:%5%:%6%)") %
                             timestamp.date().year() %
                             timestamp.date().month() %
                             timestamp.date().day() %
                             timestamp.time_of_day().hours() %
                             timestamp.time_of_day().minutes() %
                             timestamp.time_of_day().seconds();


            ::std::string retString(
                "\t\tID        : " +
                ::boost::lexical_cast<::std::string>(getPrimitiveId()) +
                "\n" +

                "\t\tVersion   : " +
                ::boost::lexical_cast<::std::string>(getVersion()) +
                "\n" +

                "\t\tTimestamp : " +
                ::boost::lexical_cast<::std::string>(getTimestamp()) +
                dateStringStream.str() +
                "\n" +

                "\t\tChangeset : " +
                ::boost::lexical_cast<std::string>(getChangesetId()) +
                "\n" +

                "\t\tUser ID   : " +
                ::boost::lexical_cast<std::string>(getUserId()) +
                "\n" +

                "\t\tUsername  : " +
                getUsername().toUtf8() +
                "\n" +

                "\t\tTags      : ");

            // Append tags
            if ( getTags().size() == 0 )
            {
                retString += "(none)\n";
            }
            else
            {
                const PrimitiveTags tags = getTags();

                for (
                    PrimitiveTags::const_iterator tagIter = tags.cbegin();
                    tagIter != tags.cend();
                    ++tagIter )
                {
                    retString += "\n\t\t\tKey   : \"" + tagIter->getKey().toUtf8() +
                                 "\"\n\t\t\tValue : \"" +
                                 tagIter->getValue().toUtf8() + "\"\n";
                }

                retString += "\n";
            }

            return retString;
        }
    }
}

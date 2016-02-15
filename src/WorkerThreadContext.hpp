#ifndef _OSMDATAWRITER_POSTGRESQLAPIDB_WORKERTHREADCONTEXT_HPP
#define _OSMDATAWRITER_POSTGRESQLAPIDB_WORKERTHREADCONTEXT_HPP

#include <memory>
#include <unordered_map>
#include <map>
#include <string>
#include <iosfwd>

namespace OsmDataWriter
{
    namespace PostgresqlApiDb
    {
        class WorkerThreadContext
        {

            public:

                WorkerThreadContext();

                virtual ~WorkerThreadContext() { }

                bool nodeTablesCreated() const
                {
                    return m_nodeTablesCreated;
                }

                void nodeTablesCreated(
                    const bool  created
                )
                {
                    m_nodeTablesCreated = created;
                }

                bool wayTablesCreated() const
                {
                    return m_wayTablesCreated;
                }

                void wayTablesCreated(
                    const bool  created
                )
                {
                    m_wayTablesCreated = created;
                }

                bool relationTablesCreated() const
                {
                    return m_relationTablesCreated;
                }

                void relationTablesCreated(
                    const bool  created
                )
                {
                    m_relationTablesCreated = created;
                }

                void newTable(
                    const ::std::string&                tableName,
                    ::std::shared_ptr<::std::ostream>   fileStream
                );

                ::std::shared_ptr<::std::ostream> getTable(
                    const ::std::string&    tableName
                ) const
                {
                    return m_fileStreams.at(tableName);
                }


            protected:

                typedef ::std::unordered_map <::std::string,
                        ::std::shared_ptr<::std::ostream >> FileStreamMap;

                bool            m_nodeTablesCreated;
                bool            m_wayTablesCreated;
                bool            m_relationTablesCreated;

                FileStreamMap   m_fileStreams;
        };
    }
}





#endif // _OSMDATAWRITER_POSTGRESQLAPIDB_WORKERTHREADCONTEXT_HPP

#include <string>
#include <memory>
#include <iostream>
#include <utility>

#include "WorkerThreadContext.hpp"

namespace OsmDataWriter
{
    namespace PostgresqlApiDb
    {
        WorkerThreadContext::WorkerThreadContext():

            m_nodeTablesCreated(false),
            m_wayTablesCreated(false),
            m_relationTablesCreated(false),
            m_fileStreams(new ::std::unordered_map <::std::string,
                          ::std::shared_ptr<::std::ostream >> )
        {
            ;
        }

        void WorkerThreadContext::newTable(
            const ::std::string&                tableName,
            ::std::shared_ptr<::std::ostream>   fileStream )
        {
            m_fileStreams->insert( ::std::make_pair(tableName, fileStream) );
        }
    }
}

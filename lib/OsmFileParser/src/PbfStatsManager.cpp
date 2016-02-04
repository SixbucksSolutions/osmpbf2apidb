#include <cstdint>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/lexical_cast.hpp>
#include "PbfStatsManager.hpp"

namespace OsmFileParser
{
    PbfStatsManager::PbfStatsManager() :
        m_compressedBytesProcessed(0),
        m_totalCompressedBytes(0),
        m_totalProcessingTime(::boost::posix_time::seconds(0)),
        m_nodeStats(),
        m_wayStats(),
        m_relationStats(),
        m_statsMutex()
    {
        m_nodeStats.entitiesVisited = 0;
        m_nodeStats.entityProcessingTime =
            ::boost::posix_time::seconds(0);

        m_wayStats.entitiesVisited = 0;
        m_wayStats.entityProcessingTime =
            ::boost::posix_time::seconds(0);

        m_relationStats.entitiesVisited = 0;
        m_relationStats.entityProcessingTime =
            ::boost::posix_time::seconds(0);
    }

    void PbfStatsManager::startStatsProcessing()
    {
        ;
    }

    void PbfStatsManager::datablockProcessingCompleted(
        const ::std::uint_fast32_t                          compressedBytesProcessed,
        const ::boost::posix_time::time_duration&           processingTime,
        const ::OsmFileParser::PbfStatsManager::EntityType  entityType,
        const ::std::uint_fast16_t                          entitiesVisited )
    {
        // Scope lock -- gets released even if exception is thrown
        ::std::lock_guard<::std::mutex> lockGuard(m_statsMutex);

        m_compressedBytesProcessed  += compressedBytesProcessed;
        m_totalProcessingTime       += processingTime;

        switch ( entityType )
        {
            case EntityType::NODE:
                m_nodeStats.entitiesVisited             += entitiesVisited;
                m_nodeStats.entityProcessingTime        += processingTime;

                break;

            case EntityType::WAY:
                m_wayStats.entitiesVisited              += entitiesVisited;
                m_wayStats.entityProcessingTime         += processingTime;

                break;

            case EntityType::RELATION:
                m_relationStats.entitiesVisited         += entitiesVisited;
                m_relationStats.entityProcessingTime    += processingTime;

                break;

                /*
                case EntityType::CHANGESET:

                    break;
                */

            default:
                throw ( "Invalid entity type passed when updating datablock stats" );

                break;
        }
    }

    void PbfStatsManager::stopStatsProcessing()
    {
        std::cout <<
                  "Stats:" << std::endl <<

                  "\tNodes:" << std::endl <<
                  "\t\tProcessed: " <<
                  ::boost::lexical_cast<std::string>(m_nodeStats.entitiesVisited) <<
                  std::endl <<
                  "\t\tTime (s): " <<
                  ::boost::lexical_cast<std::string>(
                      m_nodeStats.entityProcessingTime.total_nanoseconds() / 1000000000.0) <<
                  std::endl <<
                  "\t\tProcessed/second: " <<
                  ::boost::lexical_cast<std::string>(
                      m_nodeStats.entitiesVisited /
                      (m_nodeStats.entityProcessingTime.total_nanoseconds() / 1000000000.0) ) <<
                  std::endl;

        std::cout <<
                  "\tWays:" << std::endl <<
                  "\t\tProcessed: " <<
                  ::boost::lexical_cast<std::string>(m_wayStats.entitiesVisited) <<
                  std::endl <<
                  "\t\tTime (s): " <<
                  ::boost::lexical_cast<std::string>(
                      m_wayStats.entityProcessingTime.total_nanoseconds() / 1000000000.0) <<
                  std::endl <<
                  "\t\tProcessed/second: " <<
                  ::boost::lexical_cast<std::string>(
                      m_wayStats.entitiesVisited /
                      (m_wayStats.entityProcessingTime.total_nanoseconds() / 1000000000.0) ) <<
                  std::endl;

        std::cout <<
                  "\tRelations:" << std::endl <<
                  "\t\tProcessed: " <<
                  ::boost::lexical_cast<std::string>(m_relationStats.entitiesVisited) <<
                  std::endl <<
                  "\t\tTime (s): " <<
                  ::boost::lexical_cast<std::string>(
                      m_relationStats.entityProcessingTime.total_nanoseconds() / 1000000000.0) <<
                  std::endl <<
                  "\t\tProcessed/second: " <<
                  ::boost::lexical_cast<std::string>(
                      m_relationStats.entitiesVisited /
                      (m_relationStats.entityProcessingTime.total_nanoseconds() / 1000000000.0) ) <<
                  std::endl;


    }
}

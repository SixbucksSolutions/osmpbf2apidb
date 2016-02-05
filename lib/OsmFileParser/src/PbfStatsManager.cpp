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
        m_wayStats.entitiesVisited = 0;
        m_relationStats.entitiesVisited = 0;
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

        ::boost::posix_time::ptime now =
            ::boost::posix_time::microsec_clock::universal_time();

        if ( entityType == EntityType::NODE )
        {
            // Is this first notification we've seen for this type?
            if ( m_nodeStats.entitiesVisited == 0 )
            {
                m_nodeStats.timeFirstProcessed = now - processingTime;
            }

            m_nodeStats.entitiesVisited     +=  entitiesVisited;
            m_nodeStats.timeLastProcessed   =   now;
        }
        else if ( entityType == EntityType::WAY )
        {
            // Is this first notification we've seen for this type?
            if ( m_wayStats.entitiesVisited == 0 )
            {
                m_wayStats.timeFirstProcessed = now - processingTime;
            }

            m_wayStats.entitiesVisited     +=  entitiesVisited;
            m_wayStats.timeLastProcessed   =   now;

        }
        else if ( entityType == EntityType::RELATION )
        {
            // Is this first notification we've seen for this type?
            if ( m_relationStats.entitiesVisited == 0 )
            {
                m_relationStats.timeFirstProcessed = now - processingTime;
            }

            m_relationStats.entitiesVisited     +=  entitiesVisited;
            m_relationStats.timeLastProcessed   =   now;
        }
        /*
        else if ( entityType == EntityType::CHANGESET )
        {
            ;
        }
        */
        else
        {
            throw ( "Invalid entity type passed when updating datablock stats" );
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
                  ((m_nodeStats.timeLastProcessed - m_nodeStats.timeFirstProcessed).
                   total_microseconds() / 1000000.0) <<
                  std::endl <<
                  "\t\tProcessed/second: " <<
                  ::boost::lexical_cast<std::string>(
                      m_nodeStats.entitiesVisited /
                      ((m_nodeStats.timeLastProcessed - m_nodeStats.timeFirstProcessed).
                       total_microseconds() / 1000000.0)) <<
                  std::endl << std::endl;

        std::cout <<
                  "\tWays:" << std::endl <<
                  "\t\tProcessed: " <<
                  ::boost::lexical_cast<std::string>(m_wayStats.entitiesVisited) <<
                  std::endl <<
                  "\t\tTime (s): " <<
                  ((m_wayStats.timeLastProcessed - m_wayStats.timeFirstProcessed).
                   total_microseconds() / 1000000.0) <<
                  std::endl <<
                  "\t\tProcessed/second: " <<
                  ::boost::lexical_cast<std::string>(
                      m_wayStats.entitiesVisited /
                      ((m_wayStats.timeLastProcessed - m_wayStats.timeFirstProcessed).
                       total_microseconds() / 1000000.0)) <<
                  std::endl << std::endl;

        std::cout <<
                  "\tRelations:" << std::endl <<
                  "\t\tProcessed: " <<
                  ::boost::lexical_cast<std::string>(m_relationStats.entitiesVisited) <<
                  std::endl <<
                  "\t\tTime (s): " <<
                  ((m_relationStats.timeLastProcessed - m_relationStats.timeFirstProcessed).
                   total_microseconds() / 1000000.0) <<
                  std::endl <<
                  "\t\tProcessed/second: " <<
                  ::boost::lexical_cast<std::string>(
                      m_relationStats.entitiesVisited /
                      ((m_relationStats.timeLastProcessed - m_relationStats.timeFirstProcessed).
                       total_microseconds() / 1000000.0)) <<
                  std::endl << std::endl;

        ::boost::posix_time::ptime firstProcessing =
            ::std::min(m_nodeStats.timeFirstProcessed, m_wayStats.timeFirstProcessed);
        firstProcessing = ::std::min(firstProcessing,
                                     m_relationStats.timeFirstProcessed);

        ::boost::posix_time::ptime lastProcessing =
            ::std::max(m_nodeStats.timeLastProcessed, m_wayStats.timeLastProcessed);
        lastProcessing = ::std::max(lastProcessing,
                                    m_relationStats.timeLastProcessed);

        std::uint_fast64_t totalEntities =
            m_nodeStats.entitiesVisited +
            m_wayStats.entitiesVisited +
            m_relationStats.entitiesVisited;

        std::cout <<
                  "\tEntities:" << std::endl <<
                  "\t\tProcessed: " <<
                  totalEntities <<
                  std::endl <<
                  "\t\tTime (s): " <<
                  ((lastProcessing - firstProcessing).
                   total_microseconds() / 1000000.0) <<
                  std::endl <<
                  "\t\tProcessed/second: " <<
                  ::boost::lexical_cast<std::string>(
                      totalEntities /
                      ((lastProcessing - firstProcessing).
                       total_microseconds() / 1000000.0)) <<
                  std::endl << std::endl;


    }
}

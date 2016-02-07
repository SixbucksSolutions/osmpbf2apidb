#include <cstdint>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <iostream>
#include <iomanip>
#include <ctime>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>
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
        m_statsMutex(),
        m_statsThread(),
        m_statsThreadTermMutex(),
        m_statsThreadCondition(),
        m_statsThreadTerminate(false)
    {
        m_nodeStats.entitiesVisited = 0;
        m_wayStats.entitiesVisited = 0;
        m_relationStats.entitiesVisited = 0;
    }

    void PbfStatsManager::startStatsProcessing()
    {
        // Launch stats monitoring thread
        m_statsThread = std::thread(
                            ::std::bind(&OsmFileParser::PbfStatsManager::_statsDisplay, this) );
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
        // Notify stats thread to terminate -- use dedicated scope for
        //      lock_guard
        {
            ::std::lock_guard<::std::mutex> lock(m_statsThreadTermMutex);
            m_statsThreadTerminate = true;
        }
        m_statsThreadCondition.notify_one();

        /*
        std::cout << "Main thread has requested stats thread terminate itself" <<
                  std::endl;
        */

        // Wait for stats thread to receive notification and terminate, so it
        //      rejoins cleanly
        m_statsThread.join();
        /*
        std::cout << "Stats thread has rejoined main thread cleanly" <<
                  std::endl;
        */

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

    void PbfStatsManager::_statsDisplay()
    {
        // Mark the time we start processing stats, as
        //      everything we do after this is relative
        //      to that time
        const ::std::chrono::time_point <
        ::std::chrono::steady_clock > statsEpoch(
            ::std::chrono::steady_clock::now() );

        // Need to calculate next wakeup time; due to spurious wakeups,
        //     never know why we woke up
        ::std::chrono::time_point <
        ::std::chrono::steady_clock > nextWakeup(
            statsEpoch + ::std::chrono::seconds(1) );

        // Wait for condition variable indicating main thread
        //      wants us to terminate. If a second goes by
        //      without being signalled, print stats update
        ::std::unique_lock<::std::mutex> lock(m_statsThreadTermMutex);

        while ( m_statsThreadTerminate == false )
        {
            m_statsThreadCondition.wait_until(lock, nextWakeup);

            const ::std::chrono::steady_clock::time_point currTime =
                ::std::chrono::steady_clock::now();

            // Check for spurious wakeup of condition_variable
            if ( (m_statsThreadTerminate == false) &&
                    (currTime < nextWakeup) )
            {
                /*
                std::cout << "Spurious wakeup, going back to sleep" <<
                    std::endl;
                */
                continue;
            }

            // Pull local copy of stats that we can perform computations on to keep
            //      critical section as small as possible
            ::std::uint_fast64_t    processedBytes;
            ::std::uint_fast64_t    totalBytes;
            EntityProcessingStats   nodeStats;
            EntityProcessingStats   wayStats;
            EntityProcessingStats   relationStats;
            {
                ::std::lock_guard<::std::mutex> lockGuard(m_statsMutex);
                processedBytes  = m_compressedBytesProcessed;
                totalBytes      = m_totalCompressedBytes;
                nodeStats       = m_nodeStats;
                wayStats        = m_wayStats;
                relationStats   = m_relationStats;
            }

            // Time display
            char timeFormat[32];
            const ::std::time_t now_c = ::std::chrono::system_clock::to_time_t(
                                            ::std::chrono::system_clock::now());
            ::std::strftime( timeFormat, sizeof(timeFormat),
                             "%F %T", ::std::localtime(&now_c));
            ::std::cout << timeFormat << "  ";

            // % complete (based on processed compressed bytes / total compressed bytes)
            const uint_fast64_t percentComplete =
                ( (processedBytes * 100) / totalBytes );

            ::std::cout << ::boost::format("%3d") % percentComplete << "%";


            // Element processing rates
            const ::std::uint_fast64_t totalElements =
                nodeStats.entitiesVisited +
                wayStats.entitiesVisited +
                relationStats.entitiesVisited;

            const ::std::chrono::seconds totalSeconds =
                ::std::chrono::duration_cast<::std::chrono::seconds>(currTime - statsEpoch);
            const ::std::uint_fast64_t elementRate = totalElements / totalSeconds.count();

            const ::std::uint_fast64_t nodeRate =
                nodeStats.entitiesVisited /
                (((nodeStats.timeLastProcessed - nodeStats.timeFirstProcessed).
                  total_microseconds()) / 1000000.0);

            const ::std::uint_fast64_t wayRate =
                wayStats.entitiesVisited /
                (((wayStats.timeLastProcessed - wayStats.timeFirstProcessed).
                  total_microseconds()) / 1000000.0);

            const ::std::uint_fast64_t relationRate =
                relationStats.entitiesVisited /
                (((relationStats.timeLastProcessed - relationStats.timeFirstProcessed).
                  total_microseconds()) / 1000000.0);

            const ::std::string unitChars[4] = { "", "K", "M", "B" };

            if ( totalElements > 0 )
            {
                int unitIndex = 0;
                ::std::uint_fast64_t    visitCount = totalElements;

                while ( visitCount >= 1000 )
                {
                    visitCount /= 1000;
                    unitIndex++;
                }

                ::std::cout <<
                            ::boost::format("  E: %3d%s (%5d K/s)") %
                            visitCount %
                            unitChars[unitIndex] %
                            (elementRate / 1000);
            }
            else
            {
                std::cout <<
                          "E:    - (    - K/s)";
            }


            if ( nodeStats.entitiesVisited > 0 )
            {
                int unitIndex = 0;
                ::std::uint_fast64_t    visitCount = nodeStats.entitiesVisited;

                while ( visitCount >= 1000 )
                {
                    visitCount /= 1000;
                    unitIndex++;
                }

                ::std::cout <<
                            ::boost::format(" | N: %3d%s (%5d K/s)") %
                            visitCount %
                            unitChars[unitIndex] %
                            (nodeRate / 1000);
            }
            else
            {
                std::cout <<
                          " | N:    - (    - K/s)";
            }

            if ( wayStats.entitiesVisited > 0 )
            {
                int unitIndex = 0;
                ::std::uint_fast64_t    visitCount = wayStats.entitiesVisited;

                while ( visitCount >= 1000 )
                {
                    visitCount /= 1000;
                    unitIndex++;
                }

                ::std::cout <<
                            ::boost::format(" | W: %3d%s (%5d K/s)") %
                            visitCount %
                            unitChars[unitIndex] %
                            (wayRate / 1000);
            }
            else
            {
                std::cout <<
                          " | W:    - (    - K/s)";
            }

            if ( relationStats.entitiesVisited > 0 )
            {
                int unitIndex = 0;
                ::std::uint_fast64_t    visitCount = relationStats.entitiesVisited;

                while ( visitCount >= 1000 )
                {
                    visitCount /= 1000;
                    unitIndex++;
                }

                ::std::cout <<
                            ::boost::format(" | R: %3d%s (%5d K/s)") %
                            visitCount %
                            unitChars[unitIndex] %
                            (relationRate / 1000);
            }
            else
            {
                std::cout <<
                          " | R:    - (    - K/s)";
            }

            ::std::cout << ::std::endl;

            // Reset wakeup time and go back to sleep
            nextWakeup += ::std::chrono::seconds(1);
        }

        // When unique_lock goes out of scope it will be released

        //std::cout << "Stats thread terminating normally" << std::endl;
    }
}

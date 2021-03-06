#ifndef _PBFSTATSMANAGER_HPP
#define _PBFSTATSMANAGER_HPP

#include <cstdint>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <chrono>
#include <boost/date_time/posix_time/posix_time_types.hpp>

namespace OsmFileParser
{
    class PbfStatsManager
    {
        public:

            enum EntityType
            {
                NODE        = 0,
                WAY         = 1,
                RELATION    = 2
                              //CHANGESET = 3     // Not implemented yet
            };

            PbfStatsManager();

            virtual ~PbfStatsManager() { }

            void totalCompressedBytes(
                const ::std::uint64_t   totalCompressedBytes )
            {
                m_totalCompressedBytes = totalCompressedBytes;
            }

            ::std::uint64_t totalCompressedBytes() const
            {
                return m_totalCompressedBytes;
            }

            void startStatsProcessing();

            /**
             *
             * @note PBF spec limits number of elements per
             *      datablock to 8,000, hence why using a
             *      16-bit datatype for item count is safe
             */
            void datablockProcessingCompleted(
                const ::std::uint_fast32_t                          compressedBytesProcessed,
                const ::boost::posix_time::time_duration&           processingTime,
                const ::OsmFileParser::PbfStatsManager::EntityType  entityType,
                const ::std::uint_fast16_t                          entitiesVisited
            );

            void stopStatsProcessing();

        protected:

            struct EntityProcessingStats
            {
                ::std::uint_fast64_t                entitiesVisited;
                ::boost::posix_time::ptime          timeFirstProcessed;
                ::boost::posix_time::ptime          timeLastProcessed;
                ::boost::posix_time::time_duration  totalProcessingTime;
            };

            /// Compressed bytes processed so far
            ::std::uint_fast64_t                m_compressedBytesProcessed;

            /// Total compressed bytes in the PBF file
            ::std::uint_fast64_t                m_totalCompressedBytes;

            /// Total processing time
            ::boost::posix_time::time_duration  m_totalProcessingTime;

            // Detailed stats for each entity type
            EntityProcessingStats               m_nodeStats;
            EntityProcessingStats               m_wayStats;
            EntityProcessingStats               m_relationStats;

            /*
             // Commented out, not tracking changesets yet
            EntityProcessingStats               m_changesetStats;
            */

            /// Mutex to protect stats as threads start reporting them
            ::std::mutex                        m_statsMutex;

            /// Stats processing thread data
            ::std::thread                       m_statsThread;
            ::std::mutex                        m_statsThreadTermMutex;
            ::std::condition_variable           m_statsThreadCondition;
            bool                                m_statsThreadTerminate;
            ::std::chrono::time_point <
            ::std::chrono::system_clock >    m_statsWakeup;

            /// Stats worker thread function
            void _statsDisplay();

    };
}
#endif // _PBFSTATSMANAGER_HPP

#ifndef _LONLATCOORDINATE_HPP
#define _LONLATCOORDINATE_HPP

#include <cstdint>

namespace osmpbf2apidb
{
    class LonLatCoordinate
    {
        public:
            LonLatCoordinate();

            /**
             * Create coordinate based on nanodegree accuracy
             *
             * @param [in] lon Nanodegrees of longitude (-1800000000 to +1800000000, full integers only)
             * @param [in] lat Nanodegrees of latitude  (- 900000000 to + 900000000, full integers only)
             */
            LonLatCoordinate(
                const std::int_fast32_t&    lon,
                const std::int_fast32_t&    lat
            );

            ~LonLatCoordinate();

            /**
             * Get coordinates in nanodegree accuracy
             */
            void getLonLat(
                std::int_fast32_t&  lon,
                std::int_fast32_t&  lat
            ) const;

            /**
             * Update coordinate using nanodegree delta values from current
             */
            void deltaUpdate(
                const std::int_fast32_t&    lonDelta,
                const std::int_fast32_t&    latDelta
            );

            static std::int_fast32_t   convertDegreeToNanodegree(
                const double&   degree
            );

            static double         convertNanodegreeToDegree(
                const std::int_fast32_t&    nanoDegree
            );


        private:
            std::int_fast32_t   m_lon;
            std::int_fast32_t   m_lat;

    };
}

#endif // _LONLATCOORDINATE_HPP

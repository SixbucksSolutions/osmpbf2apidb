#include <cstdint>
#include "LonLatCoordinate.hpp"

namespace osmpbf2apidb
{
    LonLatCoordinate::LonLatCoordinate():
        m_lon(0),
        m_lat(0)
    {
        ;
    }

    LonLatCoordinate::LonLatCoordinate(
        const std::int_fast32_t&    lon,
        const std::int_fast32_t&    lat ):

        m_lon(lon),
        m_lat(lat)
    {
        ;
    }

    LonLatCoordinate::~LonLatCoordinate()
    {
        ;
    }

    void LonLatCoordinate::getLonLat(
        std::int_fast32_t&  lon,
        std::int_fast32_t&  lat ) const
    {
        lon = m_lon;
        lat = m_lat;
    }

    void LonLatCoordinate::deltaUpdate(
        const std::int_fast32_t&    lonDelta,
        const std::int_fast32_t&    latDelta )
    {
        m_lon += lonDelta;
        m_lat += latDelta;
    }

    std::int_fast32_t   convertDegreeToNanodegree(
        const double&       degree )
    {
        const double scaleUp(degree * 1000000000);
        const int_fast32_t convert = scaleUp;

        return convert;
    }



}

#ifndef _LONLATCOORDINATE_HPP
#define _LONLATCOORDINATE_HPP

#include <cstdint>

namespace osmpbf2apidb
{
	class LonLatCoordinate
	{
		public:
			/**
			 * Create coordinate based on degree accuracy
			 *
			 * @param [in] lon Full degrees of longitude (-180.0 to +180.0)
			 * @param [in] lat Full degrees of latitude  (- 90.0 to + 90.0)
			 */
			LonLatCoordinate( 
					const double& lon, 
					const double& lat 
					);

			/**
			 * Create coordinate based on nanodegree accuracy
			 *
			 * @param [in] lon Nanodegrees of longitude (-1800000000 to +1800000000, full integers only)
			 * @param [in] lat Nanodegrees of latitude  (- 900000000 to + 900000000, full integers only)
			 */
			LonLatCoordinate(
					const std::int_fast32_t&	lon,
					const std::int_fast32_t&	lat
					);

			/**
			 * Get coordinates in degree accuracy
			 */
			void getLonLat( 
					double& lon,
					double& lat 
					) const;

			/**
			 * Get coordinates in nanodegree accuracy
			 */
			void getLonLat(
					std::int_fast32_t&	lon,
					std::int_fast32_t&	lat
					) const;

		private:
			std::int_fast32_t	m_lon;
			std::int_fast32_t	m_lat;


	};
}

#endif // _LONLATCOORDINATE_HPP

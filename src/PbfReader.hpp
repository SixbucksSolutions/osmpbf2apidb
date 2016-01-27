#include <iosfwd>		// Forward declarations of IO stream functions
#include <fstream>
#include <string>
#include <cstdint>
#include <boost/shared_ptr.hpp>

namespace osmpbf2pgsql 
{
	class PbfReader
	{
		public:
			/**
			 * Create a new PBF Reader
			 *
			 * @param pbfFilename [in] Name of the PBF file to open
			 */
			PbfReader(
				const std::string& pbfFilename 	
			);

			/**
			 * Return size of the currently-open PBF file
			 *
			 * @return Size of PBF file in bytes
			 */
			std::uint64_t getFileSizeInBytes() const;

			/**
			 * Spit out info about header
			 */
			void readHeader();

			~PbfReader();

		private:
			std::ifstream 		m_pbfInput;
			std::uint64_t 		m_pbfFileSizeInBytes;	
	};
}

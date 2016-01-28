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
			 * Determine starting location for all datablocks
			 */
			void findDatablocks();

			~PbfReader();

		private:
			std::ifstream 		m_pbfInput;
			std::uint64_t 		m_pbfFileSizeInBytes;	

			/**
			 * Read a uint32_t from current file pointer location
			 *
			 * @return NETWORK-byte order uint32_t
			 */
			std::uint32_t _readUint32();

			/**
			 * Make sure there's room in the file for the read we're about to do
			 *
			 * Throws exception if not enough bytes remain
			 */
			void _verifySpaceInFile(
				const std::uint64_t 	bytesToRead
			);
	};
}

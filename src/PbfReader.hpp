#include <string>
#include <cstdint>

namespace osmpbf2apidb
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

			std::uint64_t                                                     	m_pbfFileSizeInBytes;
			char*																						m_pMemoryMappedBuffer;

			std::uint64_t		_calculateFileOffset(
				char const * const	pFilePtr 
			) const;
	};
}

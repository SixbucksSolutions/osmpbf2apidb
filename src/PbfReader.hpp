#include <string>
#include <cstdint>
#include <boost/shared_ptr.hpp>
#include <google/protobuf/io/zero_copy_stream.h>

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

			std::uint64_t                                                     	m_pbfFileSizeInBytes;
			void*																						m_pMemoryMappedBuffer;
			::boost::shared_ptr< ::google::protobuf::io::ZeroCopyInputStream >	m_pbfInputStream;
	};
}

#ifndef _DATABLOCKWORKLIST_HPP
#define _DATABLOCKWORKLIST_HPP

#include <cstdint>
#include <vector>

namespace osmpbf2apidb
{
	class DatablockWorklist
	{
		public:

			struct CompressedDatablock {
				char* 		   pByteStart;		///< Pointer to the first byte of the compressed block
				char* 		   pByteEnd;		///< Pointer to the last byte of the compressed block
				std::int32_t   sizeInBytes;	///< Total bytes of compressed data
			};

			DatablockWorklist();

			/**
			 * Add an entry about a compressed block of data that needs processing
			 *
			 * @param [in] newDatablock	Information about datablock
			 */
			void addDatablock( 
				const CompressedDatablock& newDatablock 
			);

			~DatablockWorklist();

		private:
			std::vector<CompressedDatablock> 	m_datablockList;
	};
}

#endif	// _DATABLOCKWORKLIST_HPP

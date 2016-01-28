#ifndef _DATABLOCKWORKLIST_HPP
#define _DATABLOCKWORKLIST_HPP

#include <cstdint>
#include <list>

namespace osmpbf2apidb
{
    class DatablockWorklist
    {
        public:

            struct CompressedDatablock
            {
                /// Offset in memory-mapped buffer where block begins
                uint64_t     	offsetStart;

                /// Ending offset (last valid byte)
                uint64_t		offsetEnd;

                /// Total bytes of compressed data
                std::int32_t   	sizeInBytes;
            };

            DatablockWorklist();

            /**
             * Is the worklist empty?
             *
             * @return True if empty, else false
             */
            bool empty() const;

            /**
             * Get next worklist item to process
             *
             * @return Next item on list
             *
             * @note This reduces the size of the list by one (the returned element
             *      has been removed)
             */
            CompressedDatablock getNextDatablock();

            /**
             * Add an entry about a compressed block of data that needs processing
             *
             * @param [in] newDatablock Information about datablock
             */
            void addDatablock(
                const CompressedDatablock& newDatablock
            );

            ~DatablockWorklist();

        private:
            std::list<CompressedDatablock>    m_datablockList;
    };
}

#endif  // _DATABLOCKWORKLIST_HPP

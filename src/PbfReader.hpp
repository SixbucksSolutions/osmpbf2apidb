#ifndef _PBFREADER_HPP
#define _PBFREADER_HPP

#include <string>
#include <cstdint>
#include <list>
#include <boost/shared_array.hpp>
#include <osmpbf/osmpbf.h>
#include "DatablockWorklist.hpp"

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
             * Walk over the PBF datablocks and add information about them to
             *      the worklists that will be handed to worker threads
             *
             * @param [out]     pWorklists      The worklists that need to be populated
             * @param [in]      numWorklists    Number of entries in the worklists array
             */
            void generateDatablockWorklists(
                boost::shared_array<DatablockWorklist> pWorklists,
                const unsigned int  numWorklists );

            /**
             * Given a starting offset and number of bytes, decompress and decode the data in that block, handing
             *  the caller back a set of OSM entities (i.e., Node, Way, Relations) that they can do something with
             *
             * @param [in]  compressedData  Information about block to process
             */
            std::list<int> getOsmEntitiesFromCompressedDatablock(
                const DatablockWorklist::CompressedDatablock&   compressedData );

            ~PbfReader();

        private:

            std::uint64_t   m_pbfFileSizeInBytes;
            char*           m_pMemoryMappedBuffer;

            std::uint64_t       _calculateFileOffset(
                char const* const  pFilePtr
            ) const;

            void            _inflateCompressedPayload(
                const OSMPBF::Blob& currDataPayload,
                unsigned char*      pInflateBuffer
            );
    };
}

#endif // _PBFREADER_HPP

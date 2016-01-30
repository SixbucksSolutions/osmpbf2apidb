#ifndef _PBFREADER_HPP
#define _PBFREADER_HPP

#include <string>
#include <cstdint>
#include <vector>
#include <osmpbf/osmpbf.h>
#include "Utf16String.hpp"
#include "Node.hpp"
#include "../src/DatablockWorklist.hpp"

namespace OsmFileParser
{
    class PbfReader
    {
        public:
            /**
             * Create a new PBF Reader
             */
            PbfReader();

            ~PbfReader();

            /**
             * Return size of the currently-open PBF file
             *
             * @return Size of PBF file in bytes
             */
            std::uint64_t getFileSizeInBytes() const;

            /**
             * Parse the specified PBF, invoking callback functions (if provided) as entities are
             *   encountered
             *
             * @note This function will only use one worker thread to parse the file
             */
            void parse(
                const std::string&  pbfFilename,
                std::function < void(
                    const OsmFileParser::OsmPrimitive::Node&,
                    const unsigned int
                ) > nodeCallback
            );

            /**
             * Parse the specified PBF.
             *
             * The function will spawn the specified number of threads to divide up the
             *  data processing
             *
             * Any callback functions provided will be invoked as matching primitives
             *  are encountered
             */
            void parse(
                const std::string&  pbfFilename,

                std::function < void(
                    const OsmFileParser::OsmPrimitive::Node&,
                    const unsigned int
                ) > nodeCallback,

                const unsigned int numberOfWorkerThreads
            );

            /**
             * Given a starting offset and number of bytes, decompress and decode the data in that block, handing
             *  the caller back a set of OSM entities (i.e., Node, Way, Relations) that they can do something with
             *
             * @param [in]  compressedData  Information about block to process
             */
            /*
            std::list<int> getOsmEntitiesFromCompressedDatablock(
                const DatablockWorklist::CompressedDatablock&   compressedData );
            */

        private:

            std::uint64_t                                       m_pbfFileSizeInBytes;
            char*                                               m_pMemoryMappedBuffer;

            void _memoryMapPbfFile(
                const ::std::string&    pbfFilename
            );

            std::uint64_t       _calculateFileOffset(
                char const* const  pFilePtr
            ) const;

            /**
             * Walk over the PBF datablocks and add information about them to
             *      the worklists that will be handed to worker threads
             *
             * @param [in]      numWorklists    Number of entries in the worklists array
             *
             * @return Specified number of worklists
             */
            const std::vector<DatablockWorklist> _generateDatablockWorklists(
                const unsigned int  numWorklists
            );

            void            _inflateCompressedPayload(
                const OSMPBF::Blob& currDataPayload,
                unsigned char*      pInflateBuffer
            );

            void            _processOsmPrimitiveBlock(
                const OSMPBF::PrimitiveBlock&   primitiveBlock
            );

            std::vector<Utf16String> _generateStringList(
                const OSMPBF::PrimitiveBlock&   primitiveBlock
            );

            void _processDenseNodes(
                const OSMPBF::DenseNodes&       denseNodes,
                const OSMPBF::PrimitiveBlock&   primitiveBlock
            );
    };
}

#endif // _PBFREADER_HPP

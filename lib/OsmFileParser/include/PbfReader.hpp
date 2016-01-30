#ifndef _PBFREADER_HPP
#define _PBFREADER_HPP

#include <string>
#include <cstdint>
#include <vector>
#include <osmpbf/osmpbf.h>
#include "Utf16String.hpp"
#include "Node.hpp"
#include "DatablockWorklist.hpp"
#include "PrimitiveVisitor.hpp"

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
             * Parse a PBF file, invoking visitor visit() methods as appropriate
             *
             * @note A single worker thread will be used
             */
            void parse(
                const std::string&                  pbfFilename,
                ::OsmFileParser::PrimitiveVisitor*  pPrimitiveVisitor
            );

            /**
             * Parse the specified PBF.
             *
             * @param [in] pPrimitiveVisitor The visitor object to call upon primitives
             *
             * The function will spawn the specified number of threads to divide up the
             *  data processing
             */
            void parse(
                const std::string&                  pbfFilename,
                ::OsmFileParser::PrimitiveVisitor*  pPrimitiveVisitor,
                const unsigned int                  numberOfWorkerThreads
            );

        private:

            std::uint64_t                               m_pbfFileSizeInBytes;
            char*                                       m_pMemoryMappedBuffer;
            std::vector<::OsmFileParser::Utf16String>   m_stringTable;
            ::OsmFileParser::PrimitiveVisitor*          m_pPrimitiveVisitor;
            bool                                        m_visitNodes;
            bool                                        m_visitWays;
            bool                                        m_visitRelations;
            bool                                        m_visitChangesets;

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
            std::vector<DatablockWorklist> _generateDatablockWorklists(
                const unsigned int  numWorklists
            );

            /**
             * Given a starting offset and number of bytes, decompress and decode the data in that block, handing
             *  the caller back a set of OSM entities (i.e., Node, Way, Relations) that they can do something with
             *
             * @param [in]  compressedData  Information about block to process
             */
            void _parseCompressedDatablock(
                const DatablockWorklist::CompressedDatablock&   compressedData );

            void            _inflateCompressedPayload(
                const OSMPBF::Blob& currDataPayload,
                unsigned char*      pInflateBuffer
            );

            void            _processOsmPrimitiveBlock(
                const OSMPBF::PrimitiveBlock&   primitiveBlock
            );

            void _generateStringTable(
                const OSMPBF::PrimitiveBlock&   primitiveBlock
            );

            void _processDenseNodes(
                const OSMPBF::DenseNodes&       denseNodes,
                const OSMPBF::PrimitiveBlock&   primitiveBlock
            );

            void _processWorklist(
                const unsigned int  workerId,
                DatablockWorklist&  worklist
            );

    };
}

#endif // _PBFREADER_HPP

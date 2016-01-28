#include <string>
#include <iostream>
#include <thread>
#include <functional>
#include <boost/shared_array.hpp>
#include <boost/lexical_cast.hpp>
#include "osmpbf2apidb.hpp"
#include "PbfReader.hpp"
#include "DatablockWorklist.hpp"

int main(
    int         argc,
    char**  argv )
{
    if ( argc != 3 )
    {
        std::cerr << argv[0] << " [PBF file] [number of worker threads]" << std::endl
                  << std::endl;
        exit( -1 );
    }

    const std::string pbfFilename(argv[1]);
    const unsigned int numberWorkerThreads( boost::lexical_cast<unsigned int>
                                            (argv[2]) );

    try
    {
        osmpbf2apidb::PbfReader pbfReader(pbfFilename);

        // Create array for worklists -- automatically cleaned up no matter if we leave the try block normally
        //       or throw an exception
        boost::shared_array<osmpbf2apidb::DatablockWorklist> pWorklists =
            boost::shared_array<osmpbf2apidb::DatablockWorklist>(
                new osmpbf2apidb::DatablockWorklist[numberWorkerThreads]);

        pbfReader.generateDatablockWorklists(pWorklists, numberWorkerThreads);

        // Launch number of worker threads indicated on command line,
        //      giving each worker thread one worklist
        std::thread* pWorkerThreads = new std::thread[numberWorkerThreads];

        for ( unsigned int i = 0; i < numberWorkerThreads; ++i )
        {
            pWorkerThreads[i] = std::thread( processWorklist, i,
                                             std::ref(pWorklists[i]) );
        }

        // Wait for all threads to come home
        for ( unsigned int i = 0; i < numberWorkerThreads; ++i )
        {
            pWorkerThreads[i].join();
            std::cout << "Worker thread " <<
                      boost::lexical_cast<std::string>(i) <<
                      " has rejoined successfully!" << std::endl;
        }

        std::cout << std::endl << "All worker threads have rejoined successfully!" <<
                  std::endl;

        // Delete array of threads
        delete [] pWorkerThreads;
        pWorkerThreads = nullptr;


    }
    catch ( std::string const& e )
    {
        std::cerr << "Error processing PBF data: " << e << std::endl;
        return -1;
    }

    return 0;
}

void processWorklist(
    const unsigned int                  workerId,
    osmpbf2apidb::DatablockWorklist&    worklist )
{
    std::cout << "Worker thread " <<
              boost::lexical_cast<std::string>(workerId) << " started!" <<
              std::endl;

    while ( worklist.empty() == false )
    {

        // Get next chunk of work
        osmpbf2apidb::DatablockWorklist::CompressedDatablock currBlock =
            worklist.getNextDatablock();

        std::cout << "Worker thread " <<
                  boost::lexical_cast<std::string>(workerId) <<
                  " working datablock starting at offset 0x" << std::hex <<
                  currBlock.offsetStart << std::endl;

    }


    std::cout << "Worker thread " <<
              boost::lexical_cast<std::string>(workerId) <<
              " terminating normally!" << std::endl;
}

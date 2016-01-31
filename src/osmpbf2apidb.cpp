#include <iostream>
#include <string>
#include <functional>
#include <boost/lexical_cast.hpp>
#include "OsmFileParser/include/PbfReader.hpp"
#include "OsmDataWriter_PostgresqlApiDb_NoTableConstraints.hpp"

int main(
    int         argc,
    char**  argv )
{
    if ( argc != 4 )
    {
        ::std::cerr << argv[0] <<
                    " [PBF file] [number of worker threads] [output directory for sql files]"
                    << ::std::endl << ::std::endl;
        return -1;
    }

    const ::std::string pbfFilename(argv[1]);
    const unsigned int numberWorkerThreads(
        ::boost::lexical_cast<unsigned int>(argv[2]) );
    const ::std::string sqlOutputDir(argv[3]);

    try
    {
        ::OsmFileParser::PbfReader pbfReader;
        ::OsmDataWriter::PostgresqlApiDb::NoTableConstraints sqlFileWriter;
        pbfReader.parse(pbfFilename, &sqlFileWriter);

        // We know once we're done with parse, all worker threads have safely terminated so no chance of
        //  race condition on following data
        std::cout << ::std::endl <<
                  "Nodes visited     : " << ::std::dec <<
                  sqlFileWriter.getVisitedNodes() << std::endl <<
                  "Ways visited      : " << sqlFileWriter.getVisitedWays() <<
                  std::endl <<
                  "Relations visited : " <<
                  sqlFileWriter.getVisitedRelations() << std::endl;
    }
    catch ( char const* const  e )
    {
        std::cerr << "Caught exception when parsing data: " << e << std::endl;
        return -1;
    }

}

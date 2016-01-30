#include <iostream>
#include <string>
#include <boost/lexical_cast.hpp>
#include "OsmFileParser/PbfReader.hpp"

int main(
    int         argc,
    char**  argv )
{
    if ( argc != 3 )
    {
        std::cerr << argv[0] <<
                  " [PBF file] [number of worker threads] [output directory for sql files]"
                  << std::endl << std::endl;
        return -1;
    }

    const std::string pbfFilename(argv[1]);
    const unsigned int numberWorkerThreads(
        boost::lexical_cast<unsigned int>(argv[2]) );

    try
    {
        ::OsmFileParser::PbfReader pbfReader(pbfFilename);

    }
    catch ( char const* const  e )
    {

    }

}

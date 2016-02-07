#include <iostream>
#include <string>
#include <functional>
#include <boost/lexical_cast.hpp>
#include <boost/filesystem.hpp>
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
    const ::boost::filesystem::path sqlOutputDir(argv[3]);

    try
    {
        ::OsmFileParser::PbfReader pbfReader;
        ::OsmDataWriter::PostgresqlApiDb::NoTableConstraints sqlFileWriter(
            sqlOutputDir);
        pbfReader.parse(pbfFilename, &sqlFileWriter, numberWorkerThreads);
    }
    catch ( ::std::string const&   e )
    {
        std::cerr << "Caught exception when parsing data: " << e << std::endl;
        return -1;
    }
    catch ( char const* const e )
    {
        std::cerr << "Caught exception when parsing data: " << e << ::std::endl;
        return -1;
    }

    return 0;
}

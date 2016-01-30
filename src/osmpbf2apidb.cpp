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

        ::std::function <
        void(const ::OsmFileParser::OsmPrimitive::Node&,
             const unsigned int) > nodeCallback =
                 std::bind(
                     &OsmDataWriter::PostgresqlApiDb::NoTableConstraints::nodeCallback,
                     sqlFileWriter, std::placeholders::_1,
                     std::placeholders::_2);

        pbfReader.parse(pbfFilename, nodeCallback);
    }
    catch ( char const* const  e )
    {

    }

}

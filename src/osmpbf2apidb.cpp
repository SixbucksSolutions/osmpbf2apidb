#include <string>
#include <iostream>		
#include <boost/shared_array.hpp>
#include <boost/lexical_cast.hpp>
#include "osmpbf2apidb.hpp"
#include "PbfReader.hpp"
#include "DatablockWorklist.hpp"

int main(
	int 		argc,
	char**	argv )
{
	if ( argc != 3 )
	{
		std::cerr << argv[0] << " [PBF file] [number of worker threads]" << std::endl << std::endl;
		exit( -1 );
	}

	const std::string pbfFilename(argv[1]);
	const unsigned int numberWorkerThreads( boost::lexical_cast<unsigned int>(argv[2]) );

	try
	{
		osmpbf2apidb::PbfReader pbfReader(pbfFilename);

		// Create array for worklists -- automatically cleaned up no matter if we leave the try block normally
      //       or throw an exception
	 	boost::shared_array<osmpbf2apidb::DatablockWorklist> pWorklists = 
         boost::shared_array<osmpbf2apidb::DatablockWorklist>(
         new osmpbf2apidb::DatablockWorklist[numberWorkerThreads]);

		pbfReader.generateDatablockWorklists(pWorklists, numberWorkerThreads);
	}
	catch ( std::string const & e )
	{
		std::cerr << "Error processing PBF data: " << e << std::endl;
		exit(-1);
	}


}

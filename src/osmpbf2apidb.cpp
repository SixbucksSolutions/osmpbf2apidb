#include <string>
#include <iostream>		
#include <boost/shared_ptr.hpp>
#include "osmpbf2apidb.hpp"
#include "PbfReader.hpp"

int main(
	int 		argc,
	char**	argv )
{
	if ( argc != 2 )
	{
		std::cerr << argv[0] << " [PBF file]" << std::endl << std::endl;
		exit( -1 );
	}

	const std::string pbfFilename(argv[1]);

	boost::shared_ptr<osmpbf2apidb::PbfReader> pbfReader;
	try
	{
		pbfReader = boost::shared_ptr<osmpbf2apidb::PbfReader>(new osmpbf2apidb::PbfReader(pbfFilename));
	}
	catch ( std::string const & e )
	{
		std::cerr << "Error creating PBF reader: " << e << std::endl;
		exit(-1);
	}

	//std::cout << "Reader for " << pbfFilename << " successfully created!" << std::endl;

	pbfReader->findDatablocks();
}

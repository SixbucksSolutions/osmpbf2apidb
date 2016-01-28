#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <cstdint>
#include <string>
#include <netinet/in.h>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/lexical_cast.hpp>
#include <google/protobuf/io/zero_copy_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl_lite.h>
#include "PbfReader.hpp"
#include "fileformat.pb.h"
#include "osmformat.pb.h"


namespace osmpbf2pgsql
{
	PbfReader::PbfReader(const std::string& pbfFilename ):
		m_pbfFileSizeInBytes(0),
		m_pMemoryMappedBuffer(NULL)
	{
		// Open file
		int fd = -1;
		if ( (fd = open( pbfFilename.c_str(), O_RDONLY) ) == -1 )
		{
			throw( "Could not open" + pbfFilename );
		}

		// Run stat to get filesize
		struct stat sb;
		if ( fstat(fd, &sb) == -1 )
		{
			throw("Could not run fstat on " + pbfFilename );
		}
		m_pbfFileSizeInBytes = sb.st_size;

		std::cout << "File is " << m_pbfFileSizeInBytes << " bytes long" << std::endl;

		// Memmap file into our program's address space
		if ( (m_pMemoryMappedBuffer = mmap(0, m_pbfFileSizeInBytes, PROT_READ, MAP_SHARED, fd, 0)) == MAP_FAILED )
		{
			throw( "Could not mmap " + pbfFilename );
		}

		// Can close file descriptor, not needed anymore that file is in our address space
		if ( close(fd) == -1 ) 
		{
			throw( "Could not close file descriptor after mapping file" );
		}

		std::cout << "Bytes in file: " << getFileSizeInBytes() << std::endl;
	}

	void PbfReader::findDatablocks()
	{
		char* 		pCurrentBufferOffset = reinterpret_cast<char*>(m_pMemoryMappedBuffer);
		uint32_t* 	pBlobHeaderLength = reinterpret_cast<uint32_t*>(pCurrentBufferOffset);

 		// Find out how many bytes in the blob header
		std::cout << "BlobHeader length: " << ntohl(*pBlobHeaderLength) << std::endl;

		// Move pointer to next piece of data
		pCurrentBufferOffset += sizeof(uint32_t);

		// Read blobheader
		OSMPBF::BlobHeader blobHeader;
		if ( blobHeader.ParseFromArray(pCurrentBufferOffset, ntohl(*pBlobHeaderLength)) == false )
		{
			throw( "Unable to parse blob header" );
		}

		std::cout << "Read blob header successfully!" << std::endl;


	}

	PbfReader::~PbfReader()
	{
		munmap( m_pMemoryMappedBuffer, m_pbfFileSizeInBytes );
		m_pMemoryMappedBuffer = NULL;
		m_pbfFileSizeInBytes = 0;
	}

	std::uint64_t PbfReader::getFileSizeInBytes() const
	{
		return m_pbfFileSizeInBytes;
	}
}

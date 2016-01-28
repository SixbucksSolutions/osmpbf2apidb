#include <cstdint>
#include <iostream>
#include <fstream>
#include <string>
#include <netinet/in.h>
#include <boost/shared_ptr.hpp>
#include <boost/lexical_cast.hpp>
#include <osmpbf/osmpbf.h>
#include "PbfReader.hpp"


namespace osmpbf2pgsql
{
	PbfReader::PbfReader(const std::string& pbfFilename ):
		m_pbfInput(pbfFilename, std::ios::binary),
		m_pbfFileSizeInBytes(0)
	{
		if ( m_pbfInput.good() == false )
		{
			throw( "Could not open PBF file " + pbfFilename );
		}

		m_pbfInput.seekg(0, m_pbfInput.end);
		m_pbfFileSizeInBytes = m_pbfInput.tellg();
		m_pbfInput.seekg(0, m_pbfInput.beg);

		std::cout << "Bytes in file: " << getFileSizeInBytes() << std::endl;
	}

	void PbfReader::findDatablocks()
	{
		m_pbfInput.seekg(0, m_pbfInput.beg);

		const uint32_t currPos = m_pbfInput.tellg();
		while ( (m_pbfInput.eof() == false) && (currPos < m_pbfFileSizeInBytes) )
		{
			// Read four bytes of datablock type
			const uint32_t datablockType = _readUint32();
			std::cout << "Fileblock type: " << ntohl(datablockType) << std::endl;
			
			// Read blobheader
			OSMPBF::BlobHeader myHeader;

			break;
		}
	}

	PbfReader::~PbfReader()
	{
		m_pbfInput.close();
		m_pbfFileSizeInBytes = 0;
	}

	std::uint64_t PbfReader::getFileSizeInBytes() const
	{
		return m_pbfFileSizeInBytes;
	}

	std::uint32_t PbfReader::_readUint32()
	{
		uint32_t dataRead;
		const int bytesToRead = sizeof(uint32_t);
		_verifySpaceInFile(bytesToRead);
	   m_pbfInput.read((char*)&dataRead, bytesToRead);

		if ( !m_pbfInput  )
		{
			throw( "Unable to read requested " + boost::lexical_cast<std::string>(bytesToRead) + " from input file" );
		}

		return dataRead;
	}

	void PbfReader::_verifySpaceInFile(
		const std::uint64_t 	bytesToRead )
	{
		if ( (getFileSizeInBytes() - m_pbfInput.tellg() + 1) < bytesToRead )
		{
			throw( "Not enough space to read " + boost::lexical_cast<std::string>(bytesToRead) + " bytes from input file" );
		}
	}
}

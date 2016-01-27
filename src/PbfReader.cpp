#include <cstdint>
#include <iostream>
#include <fstream>
#include <string>
#include <boost/shared_ptr.hpp>
#include <osmpbf/osmpbf.h>
#include "PbfReader.hpp"


namespace osmpbf2pgsql
{
	PbfReader::PbfReader(const std::string& pbfFilename ):
		m_pbfInput(pbfFilename),
		m_pbfFileSizeInBytes(0)
	{
		if ( m_pbfInput.good() == false )
		{
			throw( "Could not open PBF file " + pbfFilename );
		}

		m_pbfInput.seekg(0, m_pbfInput.end);
		m_pbfFileSizeInBytes = m_pbfInput.tellg();
		m_pbfInput.seekg(0, m_pbfInput.beg);

	}

	void PbfReader::readHeader()
	{
		std::cout << "Bytes in file: " << getFileSizeInBytes() << std::endl;
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

}

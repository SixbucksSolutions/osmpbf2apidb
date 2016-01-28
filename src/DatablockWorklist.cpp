#include <vector>
#include "DatablockWorklist.hpp"

namespace osmpbf2apidb
{
    DatablockWorklist::DatablockWorklist():
        m_datablockList()
    {
        ;
    }

    bool DatablockWorklist::empty() const
    {
        return ( m_datablockList.empty() );
    }

    void DatablockWorklist::addDatablock(
        const CompressedDatablock&  newDatablock )
    {
        m_datablockList.push_back(newDatablock);
    }

    DatablockWorklist::CompressedDatablock DatablockWorklist::getNextDatablock()
    {
        // Throw a fit if they tried to pull data from an empty list
        if ( empty() == true )
        {
            throw ( "Tried to retrieve data from an empty worklist");
        }

        CompressedDatablock dataToReturn = m_datablockList.front();

        m_datablockList.pop_front();

        return dataToReturn;
    }


    DatablockWorklist::~DatablockWorklist()
    {
        ;
    }
}

#include "DatablockWorklist.hpp"

namespace osmpbf2apidb
{
    DatablockWorklist::DatablockWorklist():
        m_datablockList()
    {
        ;
    }

    void DatablockWorklist::addDatablock(
        const CompressedDatablock&  newDatablock )
    {
        m_datablockList.push_back(newDatablock);
    }

    DatablockWorklist::~DatablockWorklist()
    {
        ;
    }
}

#ifndef _OSMPBF2APIDB_HPP
#define _OSMPBF2APIDB_HPP

#include "DatablockWorklist.hpp"
#include "PbfReader.hpp"

int main(
    int     argc,
    char**  argv );

void processWorklist(
    const unsigned int                  workerId,
    osmpbf2apidb::DatablockWorklist&    worklist,
    osmpbf2apidb::PbfReader&            pbfReader );


#endif // _OSMPBF2APIDB_HPP

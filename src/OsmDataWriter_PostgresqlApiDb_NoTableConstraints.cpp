#include <iostream>
#include <mutex>
#include "OsmDataWriter_PostgresqlApiDb_NoTableConstraints.hpp"
#include "OsmFileParser/include/Primitive.hpp"
#include "OsmFileParser/include/PrimitiveVisitor.hpp"
#include "OsmFileParser/include/Node.hpp"
#include "OsmFileParser/include/Way.hpp"

namespace OsmDataWriter
{
    namespace PostgresqlApiDb
    {
        NoTableConstraints::NoTableConstraints():
            m_workerThreadList()
        {
            ;
        }

        void NoTableConstraints::visit(
            const ::OsmFileParser::OsmPrimitive::Node& node )
        {
            _addWorkerThreadToThreadList();
        }

        void NoTableConstraints::visit(
            const ::OsmFileParser::OsmPrimitive::Way& way )
        {
            _addWorkerThreadToThreadList();
        }

        void NoTableConstraints::visit(
            const ::OsmFileParser::OsmPrimitive::Relation& relation )
        {
            _addWorkerThreadToThreadList();
        }

        void NoTableConstraints::_addWorkerThreadToThreadList()
        {
            if ( m_workerThreadList.contains() == false )
			{
				m_workerThreadList.add();

				/*
				std::cout << "Added new worker thread #" <<
					std::dec << m_workerThreadList.getIndex() << 
					" with thread hash 0x" << std::hex <<
					::std::this_thread::get_id() << std::endl;
				*/

				// TODO: pick up here, create entry in list of file pointer
				//		structs for this worker thread
			}
        }
    }
}

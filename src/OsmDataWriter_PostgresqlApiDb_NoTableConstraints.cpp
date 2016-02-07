#include <iostream>
#include <mutex>
#include <map>
#include <vector>
#include <string>
#include <iostream>
#include <boost/filesystem.hpp>
#include "OsmDataWriter_PostgresqlApiDb_NoTableConstraints.hpp"
#include "OsmFileParser/include/Primitive.hpp"
#include "OsmFileParser/include/PrimitiveVisitor.hpp"
#include "OsmFileParser/include/Node.hpp"
#include "OsmFileParser/include/Way.hpp"

namespace OsmDataWriter
{
    namespace PostgresqlApiDb
    {
        NoTableConstraints::NoTableConstraints(
            const ::std::string&        sqlDirectory ) :

            m_outputDir(::boost::filesystem::path(sqlDirectory)),
            m_workerThreadList(),
            m_fileSectionList(),
            m_filePointersMutex(),
            m_filePointers()
        {
            if ( ::boost::filesystem::is_directory(m_outputDir) ==
                    false )
            {
                throw ( "Cannot open output SQL directory: " );
            }

            _createSectionNameList();
        }

        NoTableConstraints::NoTableConstraints(
            const ::boost::filesystem::path&        sqlDirectory) :

            m_outputDir(sqlDirectory),
            m_workerThreadList(),
            m_fileSectionList(),
            m_filePointersMutex(),
            m_filePointers()
        {
            if ( ::boost::filesystem::is_directory(m_outputDir) ==
                    false )
            {
                throw ( "Cannot open output SQL directory: " );
            }

            _createSectionNameList();
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

                _createFilePointers( m_workerThreadList.getIndex() );
            }
        }

        void NoTableConstraints::_createFilePointers(
            const unsigned int  workerThreadIndex )
        {
            // Create all file pointers

            // Acquire file pointer list mutex

            // Add file pointers to file pointer list

        }

        void NoTableConstraints::_createSectionNameList()
        {
            m_fileSectionList.push_back("sequence_updates");
            m_fileSectionList.push_back("users");
            m_fileSectionList.push_back("changesets");
            m_fileSectionList.push_back("current_nodes");
            m_fileSectionList.push_back("current_node_tags");
            m_fileSectionList.push_back("nodes");
            m_fileSectionList.push_back("node_tags");
            m_fileSectionList.push_back("current_ways");
            m_fileSectionList.push_back("current_way_nodes");
            m_fileSectionList.push_back("current_way_tags");
            m_fileSectionList.push_back("ways");
            m_fileSectionList.push_back("way_nodes");
            m_fileSectionList.push_back("way_tags");
            m_fileSectionList.push_back("current_relations");
            m_fileSectionList.push_back("current_relation_members");
            m_fileSectionList.push_back("current_relation_tags");
            m_fileSectionList.push_back("relations");
            m_fileSectionList.push_back("relation_members");
            m_fileSectionList.push_back("relation_tags");
        }
    }
}

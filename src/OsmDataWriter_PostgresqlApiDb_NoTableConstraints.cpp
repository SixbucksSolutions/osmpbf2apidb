#include <iostream>
#include <mutex>
#include <map>
#include <vector>
#include <string>
#include <utility>
#include <memory>
#include <sstream>
#include <fstream>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
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
                throw ( ::std::string("Cannot open output SQL directory ") +
                        m_outputDir.string() );
            }

            if ( ::boost::filesystem::is_empty(m_outputDir) == false )
            {
                throw ( ::std::string("Output SQL directory ") +
                        m_outputDir.string() + " is not empty" );
            }

            _createSectionNameList();
        }

        void NoTableConstraints::visit(
            const ::OsmFileParser::OsmPrimitive::Node& node )
        {
            const unsigned int workerIndex =
                _addWorkerThreadToThreadList();

            ::std::shared_ptr <::std::map <::std::string,
            ::std::shared_ptr<::std::ostream >>> workerFiles =
                _getWorkerFiles(workerIndex);

            // Create table headers if needed
            _createNodeTables(workerIndex, workerFiles);
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

        unsigned int NoTableConstraints::_addWorkerThreadToThreadList()
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

                _createFilePointerMap( m_workerThreadList.getIndex() );
            }

            return m_workerThreadList.getIndex();
        }

        void NoTableConstraints::_createFilePointerMap(
            const unsigned int  workerThreadIndex )
        {
            ::std::lock_guard<::std::mutex> lockGuard(m_filePointersMutex);

            ::std::shared_ptr <::std::map <::std::string,
            ::std::shared_ptr<::std::ostream >>>
            emptyMap = ::std::shared_ptr <::std::map <::std::string,
            ::std::shared_ptr<::std::ostream >>> (new
                                                  ::std::map <::std::string, ::std::shared_ptr<::std::ostream>>);

            m_filePointers.insert(
                ::std::make_pair(workerThreadIndex, emptyMap) );

            /*
            ::std::cout << "Added file pointers for thread " << workerThreadIndex <<
                        ::std::endl;
            */
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

        ::std::shared_ptr <::std::map <::std::string,
        ::std::shared_ptr<::std::ostream >>>
        NoTableConstraints::_getWorkerFiles(
            const unsigned int      workerThreadIndex )
        {
            ::std::lock_guard<::std::mutex> lockGuard(m_filePointersMutex);

            return m_filePointers.at(workerThreadIndex);
        }


        void NoTableConstraints::_createNodeTables(
            const unsigned int                      workerIndex,
            ::std::shared_ptr <::std::map <::std::string,
            ::std::shared_ptr<::std::ostream >>>&    workerFiles )
        {
            if ( workerFiles->count(::std::string("current_nodes")) == 0 )
            {
                std::cout << "Have to create tables" << std::endl;
                workerFiles->insert(
                    ::std::make_pair( std::string("current_nodes"),
                                      _createTable(
                                          workerIndex,
                                          "current_nodes",
                                          "COPY current_nodes (id, latitude, longitude, "
                                          "changeset_id, visible, \"timestamp\", tile, "
                                          "version) FROM stdin;")) );


                workerFiles->insert(
                    ::std::make_pair( ::std::string("current_node_tags"),
                                      _createTable(
                                          workerIndex,
                                          "current_node_tags",
                                          "COPY current_node_tags (node_id, k, v) "
                                          "FROM stdin;")) );


                workerFiles->insert(
                    ::std::make_pair( ::std::string("nodes" ),
                                      _createTable(
                                          workerIndex,
                                          "nodes",
                                          "COPY nodes (node_id, latitude, longitude, "
                                          "changeset_id, visible, \"timestamp\", tile, "
                                          "version, redaction_id) FROM stdin;")) );

                workerFiles->insert(
                    ::std::make_pair( ::std::string("node_tags"),
                                      _createTable(
                                          workerIndex,
                                          "node_tags",
                                          "COPY node_tags (node_id, version, k, v) "
                                          "FROM stdin;\n")) );
            }

            //std::cout << "Leaving _createNodeTables" << std::endl;
        }

        ::std::shared_ptr<::std::ostream>
        NoTableConstraints::_createTable(

            const unsigned int      workerIndex,
            const ::std::string&    tableName,
            const ::std::string&    tableSchema )
        {
            // Have to build filename with a stream as
            //      boost::format is not trivial to convert
            //      to a string
            ::std::stringstream filenameStream;
            filenameStream << ::boost::format("%s_%05d.sql") %
                           tableName %
                           workerIndex;

            const ::boost::filesystem::path tablePath(
                m_outputDir / filenameStream.str() );

            ::std::shared_ptr<::std::ostream> tableStream;

            try
            {
                /*
                std::cout << "Trying to open file " << tablePath.string() <<
                    std::endl;
                */

                tableStream = ::std::shared_ptr<::std::ostream>(
                                  new std::ofstream(tablePath.string(),
                                                    ::std::ofstream::binary) );
                //std::cout << "File opened" << std::endl;

                // Write UTF-8 byte-order mark
                //
                //      Byte-order mark is Totally optional as UTF-8
                //      can easily be determined by bitstream, but
                //      removes any ambiguity
                const unsigned char utf8BOM[3] = { 0xEF, 0xBB, 0xBF };
                tableStream->write( reinterpret_cast<const char*>(
                                        utf8BOM), sizeof(utf8BOM) );

                //std::cout << "Writing schema" << std::endl;
                *tableStream << tableSchema << std::endl;
                //std::cout << "Schema written" << std::endl;
            }
            catch ( ... )
            {
                throw ( std::string("Error when trying to create table " ) +
                        tablePath.string() );
            }

            return tableStream;
        }

    }
}

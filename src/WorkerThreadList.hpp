#ifndef _WORKERTHREADLIST_HPP
#define _WORKERTHREADLIST_HPP

#include <thread>
#include <mutex>
#include <unordered_map>

class WorkerThreadList
{
    public:
        WorkerThreadList();

        virtual ~WorkerThreadList() { }

        bool contains()
        {
            return contains(::std::this_thread::get_id());
        }

        bool contains(
            const ::std::thread::id&    thread
        );

        void add()
        {
            add(::std::this_thread::get_id());
        }

        void add(
            const ::std::thread::id&    thread
        );

        unsigned int getIndex()
        {
            return getIndex(
                       ::std::this_thread::get_id() );
        }

        unsigned int getIndex(
            const ::std::thread::id&    thread
        );

    protected:
        ::std::mutex                                            m_workerThreadsMutex;
        ::std::unordered_map<::std::thread::id, unsigned int>   m_workerThreads;


};

#endif // _WORKERTHREADLIST_HPP

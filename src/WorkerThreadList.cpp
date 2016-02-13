#include <thread>
#include <mutex>
#include <utility>
#include "WorkerThreadList.hpp"

WorkerThreadList::WorkerThreadList() :
    m_workerThreadsMutex(),
    m_workerThreads()
{
    ;
}

bool WorkerThreadList::contains(
    const ::std::thread::id&    thread )
{
    //::std::lock_guard<::std::mutex> threadMutex(m_workerThreadsMutex);

    return ( m_workerThreads.count(thread) == 1 );
}

void WorkerThreadList::add(
    const ::std::thread::id&    thread )
{
    ::std::lock_guard<::std::mutex> threadMutex(m_workerThreadsMutex);
    m_workerThreads.insert(
        ::std::make_pair(thread, m_workerThreads.size() + 1) );
}

unsigned int WorkerThreadList::getIndex(
    const ::std::thread::id&    thread )
{
    //::std::lock_guard<::std::mutex> threadMutex(m_workerThreadsMutex);

    return ( m_workerThreads.at(thread) );
}

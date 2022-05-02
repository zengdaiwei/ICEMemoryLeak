#include "IceXXXJobDirector.h"

IceXXXJobDirector::IceXXXJobDirector(int id, int jobItemLen)
    : m_id(id)
    , m_jobItemLen(jobItemLen)
    , m_jobItemContent(new int[jobItemLen])
    , m_running(true)
{
    for (size_t i = 0; i < jobItemLen; i++)
    {
        m_jobItemContent[i] = i;
    }
}

IceXXXJobDirector::~IceXXXJobDirector()
{
    shutdown();
    if (m_jobItemContent)
    {
        delete[] m_jobItemContent;
        m_jobItemContent = nullptr;
    }
}

bool IceXXXJobDirector::compete(::std::shared_ptr<XXXJobTrans::XXXJobWorkerPrx> worker, const::Ice::Current& current)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_fixedWorkerPrx) return false;
    m_fixedWorkerPrx = worker->ice_fixed(current.con);
    m_connection = current.con;
    m_dummyJobItemsDealerThread.reset(new std::thread(&IceXXXJobDirector::dummyJobDealProc, this));
    return true;
}

void IceXXXJobDirector::onXXXResult(::Ice::Int result, const::Ice::Current& current)
{
    //std::cout << "onXXXResult for " << m_id << ": " << result << std::endl;
}

void IceXXXJobDirector::onYYYResult(::Ice::Int result, const::Ice::Current& current)
{
    std::cout << "onYYYResult for " << m_id << ": " << result << std::endl;
}

void IceXXXJobDirector::shutdown()
{
	m_running = false;
    if (m_dummyJobItemsDealerThread)
    {
        m_dummyJobItemsDealerThread->join();
        m_dummyJobItemsDealerThread = nullptr;
    }
}

void IceXXXJobDirector::dummyJobDealProc()
{
    while (m_running)
    {
		try
		{
            const auto item = std::pair<const ::Ice::Byte*, const ::Ice::Byte*>((const Ice::Byte*)(m_jobItemContent), (const Ice::Byte*)(m_jobItemContent + m_jobItemLen));
            //m_fixedWorkerPrx->processXXXItemAsync(item);
            m_fixedWorkerPrx->processXXXItem(item);
		}
		catch (const Ice::Exception& e)
		{
			//e.ice_print(std::cout);
			//printf("\n");
            std::cout << "director " << m_id << " received exception" << std::endl;
            m_running = false;
		}
    }
}

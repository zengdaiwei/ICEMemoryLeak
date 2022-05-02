#pragma once
#include <XXXJobTransaction.h>
#include <mutex>
#include <memory>
#include <thread>
class IceXXXJobDirector: public XXXJobTrans::XXXJobDirector
{
public:
	IceXXXJobDirector(int id, int jobItemLen);
	~IceXXXJobDirector();
	// ͨ�� XXXJobDirector �̳�
	// ͨ�� XXXJobDirector �̳�
	virtual bool compete(::std::shared_ptr<XXXJobTrans::XXXJobWorkerPrx> worker, const::Ice::Current& current = ::Ice::emptyCurrent) override;
	virtual void onXXXResult(::Ice::Int result, const::Ice::Current& current = ::Ice::emptyCurrent) override;
	virtual void onYYYResult(::Ice::Int result, const::Ice::Current& current = ::Ice::emptyCurrent) override;
	void shutdown();
	bool isRunning() const
	{
		return m_running && m_fixedWorkerPrx != nullptr;
	}
private:
	const int m_id;
	const int m_jobItemLen;
	int* m_jobItemContent;
	bool m_running;
	std::mutex m_mutex;
	std::shared_ptr<XXXJobTrans::XXXJobWorkerPrx> m_fixedWorkerPrx;
    std::shared_ptr<::Ice::Connection> m_connection;

	//ģ�⹤�����ɷ����߳�
	std::unique_ptr<std::thread> m_dummyJobItemsDealerThread;

private:
	//ģ�⹤�����ɷ�
	void dummyJobDealProc();
};


#include "IceXXXJobDirector.h"
#include <DataStorm/DataStorm.h>
#include <unordered_map>

int main(int argc, char* argv[])
{
    try
    {
        /* release:
        * 1.exe - 1.exe 无计算压力： 发送 1.5GB/s
        * 1.exe - 1.exe 有模拟12倍累加计算压力： 发送 1GB/s
        * 1.exe - 1.exe 多线程 200路 Worker 有模拟12倍累加计算压力： 发送1GB/s
        * 1.exe - 4.exe(workers) 多线程 200路 worker 有模拟12倍累加计算压力： 发送1.4GB/s*/

        //initialize DataStorm
        //多线程， 明显提高多任务的吞吐量
        //而且Worker可以直接在调用接口中进行算法处理， 不用再考虑单独的异步线程调度问题；
        const char *dargv[] = { "", 
                                "--Ice.MessageSizeMax=409600", 
								"--Ice.ThreadPool.Client.Size=2",
								"--Ice.ThreadPool.Client.SizeMax=10",
								"--Ice.ThreadPool.Server.Size=2",
								"--Ice.ThreadPool.Server.SizeMax=10",
                                nullptr};
        int dargc = sizeof(dargv) / sizeof(dargv[0]) - 1;
        DataStorm::Node node(dargc, dargv);
        DataStorm::Topic<std::string, std::shared_ptr<XXXJobTrans::XXXJobDirectorPrx>> topic(node, "XXXJobDirector");
        auto writer = DataStorm::makeSingleKeyWriter(topic, "dist", "", {0, Ice::nullopt, DataStorm::ClearHistoryPolicy::OnAll});

        int directorId = 0;
        std::list<std::shared_ptr<IceXXXJobDirector>> directors;
        std::list<std::shared_ptr<::Ice::ObjectAdapter>> adapters;
        std::list<std::shared_ptr<XXXJobTrans::XXXJobDirectorPrx>> directorProxies;
        while (true)
        {
            {
                const auto adapter = node.getCommunicator()->createObjectAdapterWithEndpoints("", "default -h *");
                const auto director = std::make_shared<IceXXXJobDirector>(directorId++, 1024 * 1024);
                const auto directorProxy = Ice::uncheckedCast<XXXJobTrans::XXXJobDirectorPrx>(adapter->addWithUUID(director));
                adapter->activate();
                writer.add(directorProxy);

                directors.push_back(director);
                adapters.push_back(adapter);
                directorProxies.push_back(directorProxy);
                std::cout << directors.size() << " job directors created" << std::endl;
            }

            (void)getchar();

            auto directorItr = directors.cbegin();
            auto adapterItr = adapters.cbegin();
            auto directorProxyItr = directorProxies.cbegin();
            while (directorItr != directors.cend())
            {
                if (!(*directorItr)->isRunning())
                {
                    std::cout << "remove " << (*directorProxyItr)->ice_toString() << std::endl;
                    (*adapterItr)->remove((*directorProxyItr)->ice_getIdentity());
                    (*adapterItr)->deactivate();
                    directorItr = directors.erase(directorItr);
                    adapterItr = adapters.erase(adapterItr);
                    directorProxyItr = directorProxies.erase(directorProxyItr);
                    continue;
                }
                ++directorItr;
                ++adapterItr;
                ++directorProxyItr;
            }

            (void)getchar();
        }
        
        node.waitForShutdown();
    }
    catch(const std::exception& ex)
    {
        std::cerr << ex.what() << std::endl;
        return 1;
    }
    return 0;
}

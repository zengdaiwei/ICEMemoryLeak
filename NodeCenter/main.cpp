#include "IceXXXJobDirector.h"
#include <DataStorm/DataStorm.h>
#include <unordered_map>

int main(int argc, char* argv[])
{
    try
    {
        /* release:
        * 1.exe - 1.exe �޼���ѹ���� ���� 1.5GB/s
        * 1.exe - 1.exe ��ģ��12���ۼӼ���ѹ���� ���� 1GB/s
        * 1.exe - 1.exe ���߳� 200· Worker ��ģ��12���ۼӼ���ѹ���� ����1GB/s
        * 1.exe - 4.exe(workers) ���߳� 200· worker ��ģ��12���ۼӼ���ѹ���� ����1.4GB/s*/

        //initialize DataStorm
        //���̣߳� ������߶������������
        //����Worker����ֱ���ڵ��ýӿ��н����㷨���� �����ٿ��ǵ������첽�̵߳������⣻
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

#pragma once

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "../IFactory.hpp"
#include "../IAsio.hpp"
#include "../IManager.hpp"
#include "../IService.hpp"
#include "../IWorker.hpp"

namespace RatingService
{

namespace Tests
{

using ::testing::_;
using ::testing::StrictMock;
using ::testing::Ref;

struct AsioServiceMock : IAsioService
{
    MOCK_METHOD0(Run, void());

    MOCK_METHOD1(Stop, void(bool aForce));
};

struct AsioAcceptorMock : IAsioAcceptor
{
    MOCK_METHOD2(Accept, void(IAsioSocket* aSocket, TAcceptCallback aCallback));
};

struct AsioSocketMock : IAsioSocket
{
     MOCK_METHOD3(Receive, void(char* aBuffer, size_t aMaxLength, TReadCallback aCallback));
};

struct ManagerMock : IManager
{
    MOCK_METHOD0(Run, void());

    void ProcessMessageFromNet(std::unique_ptr<char[]> aMessage) override
    {
        ProcessMessageFromNetProxy(aMessage.get());
    }

    MOCK_METHOD1(ProcessMessageFromNetProxy, void(char* aMessage));
};

struct ServiceMock : IService
{
    MOCK_METHOD0(Run, void());

    MOCK_METHOD1(Stop, void(bool aForce));

    MOCK_METHOD1(OnAccept, void(const boost::system::error_code& aErrorCode));

    MOCK_METHOD2(OnReceive, void(const boost::system::error_code& aErrorCode, const size_t& aLength));
};

struct WorkerMock : IWorker
{
    MOCK_METHOD0(Run, void());

    MOCK_METHOD1(PostProxy, void(TWorkerTask<std::unique_ptr<char[]>>*));

    MOCK_METHOD1(ProcessProxy, void(char*));

    void Post(TWorkerTask<std::unique_ptr<char[]>> aTask)
    {
        PostProxy(&aTask);
    }

    void Process(std::unique_ptr<char[]> aTask)
    {
        ProcessProxy(aTask.get());
    }

};

struct MockFactory : IFactory
{
    template <typename T,
              template <typename, typename> typename TPtr = std::unique_ptr,
              template <typename> typename TDelete = std::default_delete>
    TPtr<T, TDelete<T>> MakeMock()
    {
        return TPtr<T, TDelete<T>>(new T);
    }

public:

    MOCK_METHOD1(MakeManagerProxy, IManager*(IFactory* aFactory));

    MOCK_METHOD1(MakeSharedServiceProxy, IService*(IManager *aManager));

    MOCK_METHOD0(MakeAsioServiceProxy, IAsioService*());

    MOCK_METHOD1(MakeAsioSocketProxy, IAsioSocket*(IAsioService* aAsioService));

    MOCK_METHOD2(MakeAsioAcceptorProxy, IAsioAcceptor*(IAsioService* aAsioService, short aPort));

public:

    std::unique_ptr<IManager> MakeManager(IFactory* aFactory) override
    {
        return std::unique_ptr<IManager>(MakeManagerProxy(aFactory));
    }

    std::shared_ptr<IService> MakeSharedService(IManager *aManager) override
    {
        return std::shared_ptr<IService>(MakeSharedServiceProxy(aManager));
    }

    std::unique_ptr<IAsioService> MakeAsioService() override
    {
        return std::unique_ptr<IAsioService>(MakeAsioServiceProxy());
    }

    std::unique_ptr<IAsioSocket> MakeAsioSocket(IAsioService* aAsioService) override
    {
        return std::unique_ptr<IAsioSocket>(MakeAsioSocketProxy(aAsioService));
    }

    std::unique_ptr<IAsioAcceptor> MakeAsioAcceptor(IAsioService* aAsioService, short aPort) override
    {
        return std::unique_ptr<IAsioAcceptor>(MakeAsioAcceptorProxy(aAsioService, aPort));
    }
};

}

}

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
using ::testing::DefaultValue;
using ::testing::Return;
using ::testing::Pointee;
using ::testing::Truly;
using ::testing::InSequence;

struct AsioServiceMock : IAsioService
{
    MOCK_METHOD0(Run, void());

    MOCK_METHOD1(Post, void(TSharedRawMessageTask));

    MOCK_METHOD1(Stop, void(bool aForce));
};

struct AsioAcceptorMock : IAsioAcceptor
{
    MOCK_METHOD2(Accept, void(IAsioSocket* aSocket, TAcceptCallback aCallback));
};

struct AsioSocketMock : IAsioSocket
{
     MOCK_METHOD3(Receive, void(uint8_t* aBuffer, size_t aMaxLength, TReadCallback aCallback));
};

struct ManagerMock : IManager
{
    MOCK_METHOD0(Run, void());

    void ProcessMessageFromNet(std::unique_ptr<uint8_t[]> aMessage, size_t aLength) override
    {
        ProcessMessageFromNetProxy(aMessage.get(), aLength);
    }

    MOCK_METHOD2(ProcessMessageFromNetProxy, void(uint8_t* aMessage, size_t aLength));
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

    MOCK_METHOD1(PostProxy, void(TSharedRawMessageTask*));

    MOCK_METHOD2(ProcessProxy, void(uint8_t* aTask, size_t aLength));

    void Post(TSharedRawMessageTask aTask) override
    {
        PostProxy(&aTask);
    }

    void Process(std::shared_ptr<uint8_t> aTask, size_t aLength) override
    {
        ProcessProxy(aTask.get(), aLength);
    }

};

struct MockFactory : IFactory
{
    ManagerMock* Manager {};
    ServiceMock* Service {};
    std::vector<WorkerMock*> Workers;

    AsioServiceMock* AsioService {};
    AsioSocketMock* AsioSocket {};
    AsioAcceptorMock* AsioAcceptor {};

    short Port;
    size_t ThreadsCount;

public:

    MockFactory() = default;

    MockFactory(short aPort, size_t aThreadsCount)
        : Port(aPort)
        , ThreadsCount(aThreadsCount)
    {
    }

    template <typename T, template <typename...> typename TPtr = std::unique_ptr>
    TPtr<T> MakeMock()
    {
        return TPtr<T>(new T);
    }

public:

    std::unique_ptr<IManager> MakeManager(IFactory* /*aFactory*/) override
    {
        auto manager = MakeMock<StrictMock<ManagerMock>>();
        Manager = manager.get();
        return std::move(manager);
    }

    std::shared_ptr<IService> MakeSharedService(IManager* /*aManager*/) override
    {
        auto service = MakeMock<StrictMock<ServiceMock>, std::shared_ptr>();
        Service = service.get();
        return std::move(service);
    }

    std::vector<std::unique_ptr<IWorker>> MakeWorkers(IFactory* /*aFactory*/, IManager* /*aManager*/) override
    {
        Workers.clear();
        std::vector<std::unique_ptr<IWorker>> result;

        for (size_t i = 0; i < ThreadsCount; ++i)
        {
            auto temp = MakeMock<StrictMock<WorkerMock>>();
            Workers.push_back(temp.get());
            result.push_back(std::move(temp));
        }

        return std::move(result);
    }

    std::unique_ptr<IAsioService> MakeAsioService() override
    {
        auto temp = MakeMock<StrictMock<AsioServiceMock>>();
        AsioService = temp.get();
        return std::move(temp);
    }

    std::unique_ptr<IAsioSocket> MakeAsioSocket(IAsioService* /*aAsioService*/) override
    {
        auto temp = MakeMock<StrictMock<AsioSocketMock>>();
        AsioSocket = temp.get();
        return std::move(temp);
    }

    std::unique_ptr<IAsioAcceptor> MakeAsioAcceptor(IAsioService* /*aAsioService*/, short /*aPort*/) override
    {
        auto temp = MakeMock<StrictMock<AsioAcceptorMock>>();
        AsioAcceptor = temp.get();
        return std::move(temp);
    }
};

}

}

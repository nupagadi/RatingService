#pragma once

#include "IFactory.hpp"
#include "Service.hpp"
#include "Asio.hpp"
#include "Manager.hpp"
#include "Worker.hpp"

namespace RatingService
{

struct Factory : IFactory
{
    Factory(short aPort, size_t aThreadsCount)
        : mAcceptorPort(aPort)
        , mThreadsCount(aThreadsCount)
    {
    }

    std::unique_ptr<IManager> MakeManager(IFactory* aFactory) override
    {
        return std::make_unique<Manager>(aFactory);
    }

    std::shared_ptr<IService> MakeSharedService(IManager* aManager) override
    {
        auto asioService = MakeAsioService();
        auto asioSocket = MakeAsioSocket(asioService.get());
        auto asioAcceptor = MakeAsioAcceptor(asioService.get(), mAcceptorPort);

        return std::make_shared<Service>(
            this, std::move(asioService), std::move(asioAcceptor), std::move(asioSocket), aManager);
    }

    std::vector<std::unique_ptr<IWorker>> MakeWorkers(IFactory* aFactory, IManager *aManager) override
    {
        assert(aFactory);
        assert(aManager);
        std::vector<std::unique_ptr<IWorker>> result;
        for (size_t i = 0; i < mThreadsCount; ++i)
        {
            result.push_back(std::make_unique<Worker>(aFactory, aManager));
        }
        return result;
    }

    std::unique_ptr<IAsioService> MakeAsioService() override
    {
        return std::make_unique<AsioService>();
    }

    std::unique_ptr<IAsioSocket> MakeAsioSocket(IAsioService* aAsioService) override
    {
        return std::make_unique<AsioSocket>(dynamic_cast<AsioService*>(aAsioService));
    }

    std::unique_ptr<IAsioAcceptor> MakeAsioAcceptor(IAsioService* aAsioService, short aPort) override
    {
        return std::make_unique<AsioAcceptor>(dynamic_cast<AsioService*>(aAsioService), aPort);
    }

private:

    short mAcceptorPort;
    size_t mThreadsCount;
};

}

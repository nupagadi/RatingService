#pragma once

#include "IFactory.hpp"
#include "IAsio.hpp"
#include "IManager.hpp"
#include "IService.hpp"
#include "IWorker.hpp"

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
        return RatingService::MakeManager(aFactory);
    }

    std::shared_ptr<IService> MakeSharedService(IManager* aManager) override
    {
        return RatingService::MakeSharedService(this, aManager, mAcceptorPort);
    }

    // TODO: Why Factory here?
    std::vector<std::unique_ptr<IWorker>> MakeWorkers(IFactory* aFactory, IManager *aManager) override
    {
        return RatingService::MakeWorkers(aFactory, aManager, mThreadsCount);
    }

    std::unique_ptr<IAsioService> MakeAsioService() override
    {
        return RatingService::MakeAsioService();
    }

    std::unique_ptr<IAsioSocket> MakeAsioSocket(IAsioService* aAsioService) override
    {
        return RatingService::MakeAsioSocket(aAsioService);
    }

    std::unique_ptr<IAsioAcceptor> MakeAsioAcceptor(IAsioService* aAsioService, short aPort) override
    {
        return RatingService::MakeAsioAcceptor(aAsioService, aPort);
    }

private:

    short mAcceptorPort;
    size_t mThreadsCount;
};

}

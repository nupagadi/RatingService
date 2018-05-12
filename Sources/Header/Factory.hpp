#pragma once

#include "IFactory.hpp"

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

    std::vector<std::unique_ptr<IWorker>> MakeWorkers(IFactory* aFactory, IManager* aManager, IData* aData) override
    {
        return RatingService::MakeWorkers(aFactory, aManager, aData, mThreadsCount);
    }

    std::unique_ptr<IAsioService> MakeAsioService() override
    {
        return RatingService::MakeAsioService();
    }

    std::unique_ptr<IAsioSocket> MakeAsioSocket(IAsioService* aAsioService) override
    {
        return RatingService::MakeAsioSocket(aAsioService);
    }

    std::unique_ptr<IAsioTimer> MakeAsioTimer(IAsioService* aAsioService) override
    {
        return RatingService::MakeAsioTimer(aAsioService);
    }

    std::unique_ptr<IAsioAcceptor> MakeAsioAcceptor(IAsioService* aAsioService, short aPort) override
    {
        return RatingService::MakeAsioAcceptor(aAsioService, aPort);
    }

    std::unique_ptr<IData> MakeData() override
    {
        return RatingService::MakeData(mThreadsCount);
    }

private:

    short mAcceptorPort;
    size_t mThreadsCount;
};

}

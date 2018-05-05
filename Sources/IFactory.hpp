#pragma once

#include <vector>

#include "IManager.hpp"
#include "IService.hpp"
#include "IWorker.hpp"
#include "IAsio.hpp"

namespace RatingService
{

struct IFactory
{
    virtual ~IFactory() = default;

    virtual std::unique_ptr<IManager> MakeManager(IFactory* aFactory) = 0;

    virtual std::shared_ptr<IService> MakeSharedService(IManager *aManager) = 0;

    virtual std::vector<std::unique_ptr<IWorker>> MakeWorkers(IFactory* aFactory, IManager *aManager) = 0;

    virtual std::unique_ptr<IAsioService> MakeAsioService() = 0;

    virtual std::unique_ptr<IAsioSocket> MakeAsioSocket(IAsioService* aAsioService) = 0;

    virtual std::unique_ptr<IAsioAcceptor> MakeAsioAcceptor(IAsioService* aAsioService, short aPort) = 0;
};

std::unique_ptr<IFactory> MakeFactory(short aPort, size_t aThreadsCount);

}

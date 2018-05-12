#pragma once

#include <vector>

#include "IManager.hpp"
#include "IService.hpp"
#include "IWorker.hpp"
#include "IAsio.hpp"
#include "IData.hpp"

namespace RatingService
{

// TODO: All const?
struct IFactory
{
    virtual ~IFactory() = default;

    virtual std::unique_ptr<IManager> MakeManager(IFactory* aFactory) = 0;

    virtual std::shared_ptr<IService> MakeSharedService(IManager* aManager) = 0;

    virtual std::vector<std::unique_ptr<IWorker>> MakeWorkers(IFactory* aFactory, IManager* aManager, IData* aData) = 0;

    virtual std::unique_ptr<IAsioService> MakeAsioService() = 0;

    virtual std::unique_ptr<IAsioSocket> MakeAsioSocket(IAsioService* aAsioService) = 0;

    virtual std::unique_ptr<IAsioAcceptor> MakeAsioAcceptor(IAsioService* aAsioService, short aPort) = 0;

    virtual std::unique_ptr<IAsioTimer> MakeAsioTimer(IAsioService* aAsioService) = 0;

    virtual std::unique_ptr<IData> MakeData() = 0;
};

std::unique_ptr<IFactory> MakeFactory(short aPort, size_t aThreadsCount);

}

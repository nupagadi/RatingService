#pragma once

#include <memory>

#include "Handlers.hpp"

namespace RatingService
{

struct IWorker
{
    virtual ~IWorker() = default;

    virtual void Run() = 0;

    virtual void Post(TSharedRawMessageTask) = 0;

    virtual void Post(TWaitTask) = 0;

    virtual void Post(TDropDataTask) = 0;

    virtual void Post(TConnectedTask) = 0;

    virtual void Process(TSharedRawMessage aTask, size_t aLength) = 0;

    virtual void Process(std::shared_future<void> aFuture) = 0;

    virtual void Process(std::chrono::seconds aNewMonday) = 0;

    virtual void Process(TConnected aType, TClientId aClientId) = 0;
};

struct IData;
struct IFactory;
struct IManager;

std::unique_ptr<IWorker> MakeWorker(
    IFactory* aFactory, IManager* aManager, IData* aData, size_t aId, size_t aThreadsCount);

std::vector<std::unique_ptr<IWorker>> MakeWorkers(
    IFactory* aFactory, IManager* aManager, IData* aData, size_t aThreadsCount);

}

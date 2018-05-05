#pragma once

#include <memory>

#include "Handlers.hpp"

namespace RatingService
{

struct IWorker
{
    virtual ~IWorker() = default;

    virtual void Run() = 0;

    virtual void Post(TSharedRawMessage) = 0;

    virtual void Process(std::shared_ptr<uint8_t> aTask, size_t aLength) = 0;
};

struct IFactory;
struct IManager;

std::vector<std::unique_ptr<IWorker>> MakeWorkers(IFactory* aFactory, IManager *aManager, size_t aThreadsCount);

}

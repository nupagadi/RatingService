#pragma once

#include <memory>

#include "Handlers.hpp"

namespace RatingService
{

struct IWorker;

struct IManager
{
    virtual ~IManager() = default;

    // TODO: Inherit from IWorker?
    virtual void Run() = 0;

    virtual void ProcessMessageFromNet(std::unique_ptr<uint8_t[]> aMessage, size_t aLength) = 0;

    virtual void ProcessNotify(size_t aTimerId, size_t aNow) = 0;

    virtual void Lock(size_t aId) = 0;

    virtual void Unlock(size_t aId) = 0;

    virtual IWorker* GetWorker(size_t aWorkerId) = 0;

    virtual void Post(TManagerSharedRawMessageTask) = 0;

    virtual void Process(TSharedRawMessage aTask, size_t aLength) = 0;
};

struct IFactory;

std::unique_ptr<IManager> MakeManager(IFactory* aFactory);

}

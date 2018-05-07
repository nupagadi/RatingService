#pragma once

#include <memory>

namespace RatingService
{

struct IManager
{
    virtual ~IManager() = default;

    // TODO: Inherit from IWorker?
    virtual void Run() = 0;

    virtual void ProcessMessageFromNet(std::unique_ptr<uint8_t[]> aMessage, size_t aLength) = 0;

    virtual void ProcessNotify(size_t aTimerId) = 0;
};

struct IFactory;

std::unique_ptr<IManager> MakeManager(IFactory* aFactory);

}

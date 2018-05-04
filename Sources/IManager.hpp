#pragma once

#include <memory>

namespace RatingService
{

struct IManager
{
    virtual ~IManager() = default;

    // TODO: Inherit from IWorker?
    virtual void Run() = 0;

    virtual void ProcessMessageFromNet(const std::unique_ptr<char[]>& aMessage) = 0;
};

}

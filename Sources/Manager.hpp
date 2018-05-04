#pragma once

#include "IManager.hpp"
#include "IService.hpp"

namespace RatingService
{

struct Manager : IManager
{
    Manager(
        std::unique_ptr<IService>&& aService,
        size_t aThreadsCount)
        : mService(std::move(aService))
        , mThreadsCount(aThreadsCount)
    {
    }

    void Run() override
    {

    }

    void ProcessMessageFromNet(std::unique_ptr<char[]> /*aMessage*/) override
    {

    }

private:

    std::unique_ptr<IService> mService;

    size_t mThreadsCount;
};

}

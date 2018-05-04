#pragma once

#include "IFactory.hpp"
#include "IManager.hpp"
#include "IService.hpp"

namespace RatingService
{

struct Manager : IManager
{
    Manager(IFactory* aFactory)
        : mService(aFactory->MakeSharedService(this))
    {
    }

    void Run() override
    {
        mService->Run();
    }

    void ProcessMessageFromNet(std::unique_ptr<char[]> /*aMessage*/) override
    {

    }

private:

    std::shared_ptr<IService> mService;
};

}

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
        , mWorkers(aFactory->MakeWorkers(this))
    {
    }

    void Run() override
    {
        mService->Run();
        for (auto& e : mWorkers)
        {
            e->Run();
        }
    }

    void ProcessMessageFromNet(std::unique_ptr<uint8_t[]> aMessage, size_t aLength) override
    {
        auto& w = mWorkers[WorkerId(aMessage.get())];
        if (aMessage[aLength - 2] == '\r' && aMessage[aLength - 1] == '\n')
        {
            aLength -= 2;
        }
        w->Post(IWorker::TRawMessage{w.get(), std::move(aMessage), aLength});
    }

private:

    size_t WorkerId(const uint8_t* aMessage)
    {
        return *reinterpret_cast<const uint32_t*>(aMessage) % mWorkers.size();
    }

private:

    std::shared_ptr<IService> mService;
    const std::vector<std::unique_ptr<IWorker>> mWorkers;
};

}

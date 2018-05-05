#pragma once

#include <thread>

#include "IFactory.hpp"
#include "IManager.hpp"
#include "IService.hpp"

namespace RatingService
{

struct Manager : IManager
{
    Manager(IFactory* aFactory)
        : mService(aFactory->MakeSharedService(this))
        , mWorkers(aFactory->MakeWorkers(aFactory, this))
    {
    }

    void Run() override
    {
        mService->Run();
        std::vector<std::thread> threads;
        for (auto& e : mWorkers)
        {
            threads.emplace_back([&e]{ e->Run(); });
        }

        for (auto& e : threads)
        {
            e.join();
        }
    }

    void ProcessMessageFromNet(std::unique_ptr<uint8_t[]> aMessage, size_t aLength) override
    {
        auto& w = mWorkers[WorkerId(aMessage.get())];
        if (aMessage[aLength - 2] == '\r' && aMessage[aLength - 1] == '\n')
        {
            aLength -= 2;
        }
        // TODO: Pass shared_ptr from Worker.
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

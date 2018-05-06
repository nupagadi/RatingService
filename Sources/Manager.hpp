#pragma once

#include <thread>

#include "IFactory.hpp"
#include "IData.hpp"
#include "IManager.hpp"
#include "IService.hpp"

namespace RatingService
{

struct Manager : IManager
{
    Manager(IFactory* aFactory)
        : mService(aFactory->MakeSharedService(this))
        , mData(aFactory->MakeData())
        , mWorkers(aFactory->MakeWorkers(aFactory, this, mData))
    {
    }

    void Run() override
    {
        std::vector<std::thread> threads;
        for (auto& e : mWorkers)
        {
            threads.emplace_back([&e]{ e->Run(); });
        }

        mService->Run();

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
        w->Post(TSharedRawMessageTask{
            w.get(),
            std::shared_ptr<uint8_t>(aMessage.release(), std::default_delete<uint8_t[]>()),
            aLength});
    }

private:

    size_t WorkerId(const uint8_t* aMessage)
    {
        return *reinterpret_cast<const uint32_t*>(aMessage) % mWorkers.size();
    }

private:

    std::shared_ptr<IService> mService;
    std::unique_ptr<IData> mData;
    const std::vector<std::unique_ptr<IWorker>> mWorkers;
};

std::unique_ptr<IManager> MakeManager(IFactory* aFactory)
{
    return std::make_unique<Manager>(aFactory);
}

}

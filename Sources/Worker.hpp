#pragma once
#include <thread>

#include "IFactory.hpp"
#include "IWorker.hpp"
#include "IManager.hpp"

namespace RatingService
{

struct Worker : IWorker
{
    // TODO: IData.
    Worker(IFactory* aFactory, IManager* aManager/*, IData* aData*/)
        : mFactory((assert(aFactory), aFactory))
        , mManager(aManager)
        , mAsioService(aFactory->MakeAsioService())
    {
        assert(mManager);
    }

    void Run() override
    {
        mAsioService->Run();
    }

    void Post(TSharedRawMessage aMessage) override
    {
        mAsioService->Post(std::move(aMessage));
    }

    void Process(std::shared_ptr<uint8_t> aTask, size_t aLength) override
    {
        (void)aTask;
        (void)aLength;
    }

private:

    IFactory* mFactory;
    IManager* mManager;
    std::unique_ptr<IAsioService> mAsioService;
};

std::vector<std::unique_ptr<IWorker>> MakeWorkers(IFactory* aFactory, IManager *aManager, size_t aThreadsCount)
{
    assert(aFactory);
    assert(aManager);
    std::vector<std::unique_ptr<IWorker>> result;
    for (size_t i = 0; i < aThreadsCount; ++i)
    {
        result.push_back(std::make_unique<Worker>(aFactory, aManager));
    }
    return result;
}

}

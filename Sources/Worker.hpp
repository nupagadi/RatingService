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
    Worker(IFactory* aFactory, IManager* aManager, IData* aData)
        : mFactory((assert(aFactory), aFactory))
        , mManager(aManager)
        , mData(aData)
        , mAsioService(aFactory->MakeAsioService())
    {
        assert(mManager);
        assert(mData);
    }

    void Run() override
    {
        mAsioService->Run();
    }

    void Post(TSharedRawMessageTask aMessage) override
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
    IData* mData;
    std::unique_ptr<IAsioService> mAsioService;
};

std::unique_ptr<IWorker> MakeWorker(IFactory* aFactory, IManager *aManager, IData* aData)
{
    assert(aFactory);
    assert(aManager);
    return std::make_unique<Worker>(aFactory, aManager, aData);
}

std::vector<std::unique_ptr<IWorker>> MakeWorkers(
    IFactory* aFactory, IManager *aManager, IData* aData, size_t aThreadsCount)
{
    assert(aFactory);
    assert(aManager);
    std::vector<std::unique_ptr<IWorker>> result;
    for (size_t i = 0; i < aThreadsCount; ++i)
    {
        result.push_back(MakeWorker(aFactory, aManager, aData));
    }
    return result;
}

}

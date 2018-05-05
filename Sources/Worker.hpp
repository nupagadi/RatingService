#pragma once

#include "IFactory.hpp"
#include "IWorker.hpp"
#include "IManager.hpp"

namespace RatingService
{

struct Worker : IWorker
{
    // TODO: IData.
    Worker(IFactory* aFactory, IManager* aManager/*, IData* aData*/)
        : mFactory(aFactory)
        , mManager(aManager)
    {
        assert(mFactory);
        assert(mManager);
    }

    void Run() override
    {
        // TODO:
//        mAsioService = aFactory->MakeAsioService();

    }

    void Post(TRawMessage) override
    {

    }

    void Process(std::unique_ptr<uint8_t[]> aTask, size_t aLength) override
    {
        (void)aTask;
        (void)aLength;
    }

private:

    IFactory* mFactory;
    IManager* mManager;
    std::unique_ptr<IAsioService> mAsioService;
};

}

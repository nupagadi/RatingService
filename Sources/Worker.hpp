#pragma once

#include <cmath>
#include <iostream>
#include <unordered_set>

#include "IFactory.hpp"
#include "RawMessageTools.hpp"

namespace RatingService
{

struct Worker : IWorker
{
    const size_t Id;

    Worker(IFactory* aFactory, IManager* aManager, IData* aData, size_t aId)
        : Id(aId)
        , mFactory((assert(aFactory), aFactory))
        , mManager(aManager)
        , mData(aData)
        , mAsioService(aFactory->MakeAsioService())
    {
        assert(mManager);
        assert(mData);
        mConnected.reserve(100'000);
    }

    void Run() override
    {
        mManager->Lock(Id);

        mAsioService->Run();

        mManager->Unlock(Id);
    }

    void Post(TSharedRawMessageTask aMessage) override
    {
        mAsioService->Post(std::move(aMessage));
    }

    void Post(TWaitTask aMessage) override
    {
        mAsioService->Post(std::move(aMessage));
    }

    void Post(TDropDataTask aMessage) override
    {
        mAsioService->Post(std::move(aMessage));
    }

    void Process(TSharedRawMessage aTask, size_t aLength) override
    {
        auto clientId = RawMessageTools::GetClientId(aTask.get());

        using MessageType = RawMessageTools::MessageType;
        switch (RawMessageTools::GetMessageType(aTask.get()))
        {
        case MessageType::Registered:
            std::cout << "Registered" << std::endl;
            if (aLength < RawMessageTools::MinRegisteredSize || RawMessageTools::MaxRegisteredSize < aLength)
            {
                std::cout << "Invalid size:" << aLength << std::endl;
                break;
            }
            mData->Register(clientId, std::move(aTask), aLength);
            break;

        case MessageType::Renamed:
            std::cout << "Renamed" << std::endl;
            if (aLength < RawMessageTools::MinRegisteredSize || RawMessageTools::MaxRegisteredSize < aLength)
            {
                std::cout << "Invalid size:" << aLength << std::endl;
                break;
            }
            mData->Register(clientId, std::move(aTask), aLength);
            break;

        case MessageType::DealWon:
        {
            std::cout << "DealWon: " << std::endl;
            if (aLength != RawMessageTools::DealWonSize)
            {
                std::cout << "Invalid size:" << aLength << std::endl;
                break;
            }
            auto amount = RawMessageTools::GetAmountOneShot(aTask.get());
            std::cout << "Amount: " << amount << std::endl;
            mData->AddDeal(clientId, amount);
        }
            break;

        case MessageType::Connected:
            std::cout << "Connected" << std::endl;
            if (aLength != RawMessageTools::ConnectedSize)
            {
                std::cout << "Invalid size:" << aLength << std::endl;
                break;
            }
            mConnected.emplace(RawMessageTools::GetClientId(aTask.get()));
            break;

        case MessageType::Disconnected:
            std::cout << "Disconnected" << std::endl;
            if (aLength != RawMessageTools::ConnectedSize)
            {
                std::cout << "Invalid size:" << aLength << std::endl;
                break;
            }
            mConnected.erase(RawMessageTools::GetClientId(aTask.get()));
            break;

        default:
            std::cerr << "Unknown message type." << std::endl;
            break;
        }
    }

    void Process(std::shared_future<void> aFuture) override
    {
        (void)aFuture;
    }

    void Process(TaskType aTask) override
    {
        (void)aTask;
    }

private:

    IFactory* mFactory;
    IManager* mManager;
    IData* mData;
    std::unique_ptr<IAsioService> mAsioService;

    std::unordered_set<TClientId> mConnected;
};

std::unique_ptr<IWorker> MakeWorker(IFactory* aFactory, IManager* aManager, IData* aData, size_t aId)
{
    assert(aFactory);
    assert(aManager);
    assert(aData);

    return std::make_unique<Worker>(aFactory, aManager, aData, aId);
}

std::vector<std::unique_ptr<IWorker>> MakeWorkers(
    IFactory* aFactory, IManager *aManager, IData* aData, size_t aThreadsCount)
{
    assert(aFactory);
    assert(aManager);
    assert(aData);

    std::vector<std::unique_ptr<IWorker>> result;
    for (size_t i = 0; i < aThreadsCount; ++i)
    {
        result.push_back(MakeWorker(aFactory, aManager, aData, i));
    }
    return result;
}

}

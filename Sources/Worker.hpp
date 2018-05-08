#pragma once

#include <cmath>
#include <iostream>
#include <unordered_set>

#include "IFactory.hpp"
#include "RawMessageTools.hpp"

namespace RatingService
{

constexpr size_t MaxConnectedContainersPerWorker(size_t aThreadsCount)
{
    return SendingIntervalsCount / aThreadsCount + 1;
}

constexpr size_t GlobalConnectedContainersId(TClientId aClientId)
{
    return aClientId % SendingIntervalsCount;
}

constexpr size_t LocalConnectedContainersId(TClientId aClientId, size_t aWorkerId)
{
    return GlobalConnectedContainersId(aClientId) / aWorkerId;
}

constexpr size_t NearestWorkerId(TTime aNowSec, size_t aThreadsCount)
{
    auto minStart = aNowSec / SpecificClientSendingIntervalSec * SpecificClientSendingIntervalSec;
    auto nextSend = aNowSec / SendingIntervalSec * SendingIntervalSec + SendingIntervalSec;
    return (nextSend - minStart) / SendingIntervalSec % aThreadsCount;
}


struct Worker : IWorker
{
    const size_t Id;
    const size_t ThreadsCount;

    Worker(IFactory* aFactory, IManager* aManager, IData* aData, size_t aId, size_t aThreadsCount)
        : Id(aId)
        , ThreadsCount(aThreadsCount)
        , mFactory((assert(aFactory), aFactory))
        , mManager(aManager)
        , mData(aData)
        , mAsioService(aFactory->MakeAsioService())
        , mConnected(MaxConnectedContainersPerWorker(aThreadsCount))
    {
        assert(mManager);
        assert(mData);
        for (auto& e : mConnected)
        {
            e.reserve(100'000);
        }
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
            mData->Rename(clientId, std::move(aTask), aLength);
            break;

        case MessageType::DealWon:
        {
            std::cout << "DealWon: " << std::endl;
            if (aLength != RawMessageTools::DealWonSize)
            {
                std::cout << "Invalid size:" << aLength << std::endl;
                break;
            }
            if (RawMessageTools::GetTime(aTask.get()) < mTradingPeriodStartUs)
            {
                std::cout << "Invalid time:" << std::endl;
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
            ProcessConnected(RawMessageTools::GetClientId(aTask.get()));
            break;

        case MessageType::Disconnected:
        {
            std::cout << "Disconnected" << std::endl;
            if (aLength != RawMessageTools::ConnectedSize)
            {
                std::cout << "Invalid size:" << aLength << std::endl;
                break;
            }

            auto clientId = RawMessageTools::GetClientId(aTask.get());
            auto erased = mConnected[LocalConnectedContainersId(clientId, Id)].erase(clientId);
            if (!erased)
            {
                std::cout << "Worker::Process: " << "Disconnected: " << "No such an id: " << clientId << std::endl;
            }

            break;
        }

        default:
            std::cerr << "Unknown message type." << std::endl;
            break;
        }
    }

    void Process(std::shared_future<void> aFuture) override
    {
        (void)aFuture;
    }

    void Process(std::chrono::seconds aNewMondaySec) override
    {
        mTradingPeriodStartUs = aNewMondaySec.count() * 1000 * 1000;
        mData->Drop(Id);
    }

private:

    void ProcessConnected(TClientId aClientId)
    {
        auto now = std::chrono::duration_cast<std::chrono::seconds>(mClock.now().time_since_epoch()).count();
        auto workerId = NearestWorkerId(now, ThreadsCount);
        if (workerId == Id)
        {
            auto emplaced = mConnected[LocalConnectedContainersId(aClientId, Id)].emplace(aClientId);
            if (!emplaced.second)
            {
                std::cout << "Worker::ProcessConnected: " << "Already exists: " << aClientId << std::endl;
            }
        }
        else
        {
//            auto worker = mManager->GetWorker(workerId);
//            worker->Post(TConnectedTask{worker, TConnected::SendInfo, aClientId});
        }
    }

private:

    IFactory* mFactory;
    IManager* mManager;
    IData* mData;
    std::unique_ptr<IAsioService> mAsioService;

    std::vector<std::unordered_set<TClientId>> mConnected;

    TTime mTradingPeriodStartUs {};

    std::chrono::system_clock mClock;
};

std::unique_ptr<IWorker> MakeWorker(
    IFactory* aFactory, IManager* aManager, IData* aData, size_t aId, size_t aThreadsCount)
{
    assert(aFactory);
    assert(aManager);
    assert(aData);

    return std::make_unique<Worker>(aFactory, aManager, aData, aId, aThreadsCount);
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
        result.push_back(MakeWorker(aFactory, aManager, aData, i, aThreadsCount));
    }
    return result;
}

}

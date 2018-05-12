#pragma once

#include <cmath>
#include <cstring>
#include <iostream>
#include <unordered_set>

#include "IFactory.hpp"
#include "RawMessageTools.hpp"

namespace RatingService
{

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

        std::cout << "Worker #" << Id << " was stopped." << std::endl;
    }

    void Stop(bool aForce) override
    {
        mAsioService->Stop(aForce);
    }

    void Post(TSharedRawMessageTask aMessage) override
    {
        mAsioService->Post(std::move(aMessage));
    }

    void Post(TWaitTask aMessage) override
    {
        mAsioService->Post(std::move(aMessage));
    }

    void Post(TSendInfoTask aMessage) override
    {
        mAsioService->Post(std::move(aMessage));
    }

    void Post(TDropDataTask aMessage) override
    {
        mAsioService->Post(std::move(aMessage));
    }

    void Post(TConnectedTask aMessage) override
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
            auto erased = mConnected[LocalConnectedContainerIdByClient(clientId, Id)].erase(clientId);
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
        mManager->Unlock(Id);
        aFuture.get();
        mManager->Lock(Id);
    }

    void Process(TSharedPromise aPromise) override
    {
        for (size_t i = 0; i < ThreadsCount; ++i)
        {
            if (i != Id)
            {
                mManager->Lock(i);
            }
        }

        auto dataCopy = mData->Copy();

        for (size_t i = 0; i < ThreadsCount; ++i)
        {
            if (i != Id)
            {
                mManager->Unlock(i);
            }
        }

        aPromise->set_value();

        SendRating(dataCopy);

        for (const auto& e : mForeigners)
        {
            auto worker = mManager->GetWorker(e % ThreadsCount);
            worker->Post(TConnectedTask{worker, TConnected::SendBack, e});
        }
        mForeigners.clear();
    }

    void Process(std::chrono::seconds aNewMondaySec) override
    {
        mTradingPeriodStartUs = aNewMondaySec.count() * 1000 * 1000;
        mData->Drop(Id);
    }

    void Process(TConnected aType, TClientId aClientId) override
    {
        if (aType == TConnected::JustConnected)
        {
            mForeigners.push_back(aClientId);
        }
        else if (aType == TConnected::SendBack)
        {
            PutConnected(aClientId);
        }
    }

private:

    void SendRating(std::vector<DataEntry>& aData)
    {
        std::sort(aData.begin(), aData.end(),
            [](const auto& lh, const auto& rh)
            {
                return lh.Total > rh.Total;
            });

        auto copyCount = std::min(aData.size(), static_cast<size_t>(RawMessageTools::SendingBlockSize));

        auto top = std::make_unique<TByte[]>(
            RawMessageTools::SendingMessageSize(copyCount) - RawMessageTools::SendingBlockSize * 2);

        RawMessageTools::SetMessageType(top.get(), RawMessageTools::MessageType::Rating);

        auto topSize = 0u;
        for (size_t i = 0; i < copyCount; ++i)
        {
            topSize += RawMessageTools::Serialize(
                top.get() + sizeof(TClientId) + sizeof(RawMessageTools::MessageType), aData[i]);
        }

        auto now = std::chrono::duration_cast<std::chrono::seconds>(mClock.now().time_since_epoch()).count();
        auto& connected = mConnected[LocalConnectedContainerIdByTime(now, ThreadsCount)];

        for (const auto& e : connected)
        {
            SendRating(e, aData, top.get(), topSize);
        }
        for (const auto& e : mForeigners)
        {
            SendRating(e, aData, top.get(), topSize);
        }
    }

    void SendRating(TClientId aClientId, std::vector<DataEntry>& aData, const TByte* aTop, size_t aTopSize)
    {
        auto copyCount = std::min(aData.size(), static_cast<size_t>(RawMessageTools::SendingBlockSize));

        // 1. Allocate.
        auto tempPtr = std::make_unique<TByte>(RawMessageTools::SendingMessageSize(copyCount));
        auto message = std::shared_ptr<TByte>(tempPtr.release(), std::default_delete<TByte[]>());
        auto writePtr = message.get();

        // 2. Copy the top (pointer).
        std::memcpy(writePtr, aTop, aTopSize);
        writePtr += aTopSize;

        // 3. Copy the client (ClientId).
        // 3.1 Copy ClientId to the header.
        RawMessageTools::SetClientId(message.get(), aClientId);

        auto clientId = [](const auto& e)
        {
            return RawMessageTools::GetClientId(e.ClientInfo.data());
        };

        DataEntry entry;
        RawMessageTools::SetClientId(entry.ClientInfo.data(), aClientId);
        auto it = std::lower_bound(aData.cbegin(), aData.cend(), entry,
            [clientId](const auto& lh, const auto& rh){ return clientId(lh) < clientId(rh); });
        if (clientId(*it) != aClientId)
        {
            std::cerr << "Worker::SendRating: " << "Not found: " << aClientId << std::endl;
            return;
        }

        auto serialize = [&writePtr](const auto& e)
        {
            writePtr += RawMessageTools::Serialize(writePtr, e);
        };

        serialize(*it);

        // 4. Copy plus/minus.
        size_t prevCount = std::distance(it, aData.cbegin());
        std::for_each(std::prev(it, std::min(copyCount, prevCount)), it, serialize);

        size_t afterCount = aData.size() - prevCount - 1;
        std::for_each(it + 1, std::next(it + 1, std::min(copyCount, afterCount)), serialize);

        // 5. Send.
        mManager->Post(TManagerSharedRawMessageTask{mManager, message, writePtr - message.get()});
    }

    void ProcessConnected(TClientId aClientId)
    {
        auto now = std::chrono::duration_cast<std::chrono::seconds>(mClock.now().time_since_epoch()).count();
        auto workerId = NearestWorkerId(now, ThreadsCount);
        if (workerId == Id)
        {
            PutConnected(aClientId);
        }
        else
        {
            auto worker = mManager->GetWorker(workerId);
            worker->Post(TConnectedTask{worker, TConnected::JustConnected, aClientId});
        }
    }

    void PutConnected(TClientId aClientId)
    {
        auto emplaced = mConnected[LocalConnectedContainerIdByClient(aClientId, Id)].emplace(aClientId);
        if (!emplaced.second)
        {
            std::cout << "Worker::ProcessConnected: " << "Already exists: " << aClientId << std::endl;
        }
    }

private:

    IFactory* mFactory;
    IManager* mManager;
    IData* mData;
    std::unique_ptr<IAsioService> mAsioService;

    std::vector<std::unordered_set<TClientId>> mConnected;
    std::vector<TClientId> mForeigners;

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

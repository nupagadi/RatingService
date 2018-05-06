#pragma once

#include <cmath>
#include <iostream>
#include <thread>
#include <unordered_set>

#include "IFactory.hpp"

namespace RatingService
{

enum class MessageType : uint32_t
{
    UNKNOWN = 0,
    Registered = 1,
    Renamed = 2,
    DealWon = 3,
    Connected = 4,
    Disconnected = 5,
    Rating = 6,
};

const constexpr size_t MinRegisteredSize = 9;
const constexpr size_t MaxRegisteredSize = 64;
const constexpr size_t DealWonSize = 24;
const constexpr size_t ConnectedSize = 8;

struct Worker : IWorker
{
    Worker(IFactory* aFactory, IManager* aManager, IData* aData)
        : mFactory((assert(aFactory), aFactory))
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
        mAsioService->Run();
    }

    void Post(TSharedRawMessageTask aMessage) override
    {
        mAsioService->Post(std::move(aMessage));
    }

    void Process(TSharedRawMessage aTask, size_t aLength) override
    {
        auto clientId = GetClientId(aTask.get());
        switch (GetMessageType(aTask.get()))
        {
        case MessageType::Registered:
            std::cout << "Registered" << std::endl;
            if (aLength < MinRegisteredSize || MaxRegisteredSize < aLength)
            {
                std::cout << "Invalid size:" << aLength << std::endl;
                break;
            }
            mData->Register(clientId, std::move(aTask));
            break;

        case MessageType::Renamed:
            std::cout << "Renamed" << std::endl;
            if (aLength < MinRegisteredSize || MaxRegisteredSize < aLength)
            {
                std::cout << "Invalid size:" << aLength << std::endl;
                break;
            }
            mData->Register(clientId, std::move(aTask));
            break;

        case MessageType::DealWon:
        {
            std::cout << "DealWon: " << std::endl;
            if (aLength != DealWonSize)
            {
                std::cout << "Invalid size:" << aLength << std::endl;
                break;
            }
            auto amount = GetAmountOneShot(aTask.get());
            std::cout << "Amount: " << amount << std::endl;
            mData->AddDeal(clientId, amount);
        }
            break;

        case MessageType::Connected:
            std::cout << "Connected" << std::endl;
            if (aLength != ConnectedSize)
            {
                std::cout << "Invalid size:" << aLength << std::endl;
                break;
            }
            mConnected.emplace(GetClientId(aTask.get()));
            break;

        case MessageType::Disconnected:
            std::cout << "Disconnected" << std::endl;
            if (aLength != ConnectedSize)
            {
                std::cout << "Invalid size:" << aLength << std::endl;
                break;
            }
            mConnected.erase(GetClientId(aTask.get()));
            break;

        default:
            std::cerr << "Unknown message type." << std::endl;
            break;
        }
    }

private:

    static constexpr MessageType GetMessageType(const TByte* aRaw)
    {
        const constexpr int offset = 1;
        return reinterpret_cast<const MessageType*>(aRaw)[offset];
    }

    static constexpr TClientId GetClientId(const TByte* aRaw)
    {
        const constexpr int offset = 0;
        return reinterpret_cast<const TClientId*>(aRaw)[offset];
    }

    // Corrupt the message.
    static double GetAmountOneShot(TByte* aRaw)
    {
        const constexpr int offset = 2;
        const constexpr int powerOffset = 7;
        auto amountPtr = reinterpret_cast<uint64_t*>(aRaw) + offset;
        auto power = std::pow(10, std::exchange(reinterpret_cast<TByte*>(amountPtr)[powerOffset], 0));
        return amountPtr[0] * power;
    }

private:

    IFactory* mFactory;
    IManager* mManager;
    IData* mData;
    std::unique_ptr<IAsioService> mAsioService;

    std::unordered_set<TClientId> mConnected;
};

std::unique_ptr<IWorker> MakeWorker(IFactory* aFactory, IManager *aManager, IData* aData)
{
    assert(aFactory);
    assert(aManager);
    assert(aData);

    return std::make_unique<Worker>(aFactory, aManager, aData);
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
        result.push_back(MakeWorker(aFactory, aManager, aData));
    }
    return result;
}

}

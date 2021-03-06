#pragma once

#include <cstring>
#include <iostream>
#include <unordered_map>
#include <algorithm>

#include "IData.hpp"
#include "RawMessageTools.hpp"

namespace RatingService
{

struct Data : IData
{
    static const constexpr size_t MaxClients = 1'000'000;

    Data(size_t aThreadsCount)
        : mThreadsCount(aThreadsCount)
        , mEntries(mThreadsCount)
    {
        mClientIdToPosition.reserve(MaxClients);
        for (auto& e : mEntries)
        {
            e.reserve(MaxClients / mThreadsCount);
        }
    }

    void Register(TClientId aClientId, TSharedRawMessage aName, size_t aLength) override
    {
        auto& workerEntries = mEntries[aClientId % mThreadsCount];
        auto emplaceResult = mClientIdToPosition.emplace(aClientId, workerEntries.size());
        if (!emplaceResult.second)
        {
            std::cerr << "Data::Register" << "Already registered: " << aClientId << std::endl;
            return;
        }
        // Realloc is thread-safe.
        workerEntries.emplace_back();
        workerEntries.back().Total = 0;

        SetName(&workerEntries.back(), aName.get(), aLength);
    }

    void Rename(TClientId aClientId, TSharedRawMessage aName, size_t aLength) override
    {
        if (auto ptr = GetEntryPointer(aClientId))
        {
            SetName(ptr, aName.get(), aLength);
        }
    }

    void AddDeal(TClientId aClientId, double aAmount) override
    {
        if (auto ptr = GetEntryPointer(aClientId))
        {
            ptr->Total += aAmount;
        }
    }

    std::vector<DataEntry> Copy() const override
    {
        auto size = std::accumulate(
            mEntries.cbegin(), mEntries.cend(), 0,
            [](size_t size, const auto& entry)
            { return size + entry.size(); });

        decltype(Copy()) result(size);

        auto begin = result.begin();
        for (const auto& e : mEntries)
        {
            // memmove here:
            begin = std::copy(e.cbegin(), e.cend(), begin);
        }

        return std::move(result);
    }

    void Drop(size_t aWorkerId) override
    {
        assert(aWorkerId < mEntries.size());
        for (auto& e : mEntries[aWorkerId])
        {
            e.Total = 0;
        }
    }

private:

    DataEntry* GetEntryPointer(TClientId aClientId)
    {
        auto& workerEntries = mEntries[aClientId % mThreadsCount];

        auto it = mClientIdToPosition.find(aClientId);
        assert(it != mClientIdToPosition.cend());
        assert(workerEntries.size() > it->second);

        if (it == mClientIdToPosition.cend() || workerEntries.size() <= it->second)
        {
            std::cerr << "Data: " << "Not found: " << aClientId << std::endl;
            return nullptr;
        }

        return &workerEntries[it->second];
    }

    void SetName(DataEntry* aEntry, const TByte* aFrom, size_t aLength)
    {
        aEntry->InfoSize = aLength;
        std::memcpy(aEntry->ClientInfo.data(), aFrom, aLength);
    }

private:

    size_t mThreadsCount;

    std::vector<std::vector<DataEntry>> mEntries;
    std::unordered_map<TClientId, size_t> mClientIdToPosition;
};

std::unique_ptr<IData> MakeData(size_t aThreadsCount)
{
    return std::make_unique<Data>(aThreadsCount);
}

}

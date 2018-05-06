#pragma once

#include <cstring>
#include <iostream>
#include <unordered_map>

#include "IData.hpp"
#include "RawMessageTools.hpp"

namespace RatingService
{

struct Data : IData
{
    Data(size_t aThreadsCount)
        : mThreadsCount(aThreadsCount)
    {
    }

    void Register(TClientId aClientId, TSharedRawMessage aName, size_t aLength) override
    {
        auto& workerEntries = mEntries[aClientId % mThreadsCount];
        auto emplaceResult = mClientIdToPosition.emplace(aClientId, workerEntries.size());
        assert(emplaceResult.second);
        if (!emplaceResult.second)
        {
            std::cerr << "Data::Register" << "Already registered: " << aClientId << std::endl;
            return;
        }
        workerEntries.emplace_back();
        workerEntries.back().Total = 0;

        SetName(&workerEntries.back(), aName.get(), aLength);
    }

    void Rename(TClientId aClientId, TSharedRawMessage aName, size_t aLength) override
    {
        auto& workerEntries = mEntries[aClientId % mThreadsCount];

        auto it = mClientIdToPosition.find(aClientId);
        assert(it != mClientIdToPosition.cend());
        assert(workerEntries.size() > it->second);

        if (it == mClientIdToPosition.cend() || workerEntries.size() <= it->second)
        {
            std::cerr << "Data::Rename" << "Not found: " << aClientId << std::endl;
            return;
        }

        SetName(&workerEntries[it->second], aName.get(), aLength);
    }

    void AddDeal(TClientId /*aClientId*/, double /*aAmount*/) override
    {

    }

    std::vector<DataEntry> Copy() const override
    {
        return {};
    }

private:

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

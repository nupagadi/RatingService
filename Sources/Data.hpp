#pragma once

#include <unordered_map>

#include "IData.hpp"

namespace RatingService
{

struct Data : IData
{
    Data(size_t aThreadsCount)
        : mThreadsCount(aThreadsCount)
    {
    }

    void Register(TClientId /*aClientId*/, TSharedRawMessage /*aName*/) override
    {
//        mEntries.emplace_back();
//        mClientIdToPosition[aClientId]
    }

    void Rename(TClientId /*aClientId*/, TSharedRawMessage /*aName*/) override
    {

    }

    void AddDeal(TClientId /*aClientId*/, double /*aAmount*/) override
    {

    }

    std::vector<DataEntry> Copy() const override
    {
        return {};
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

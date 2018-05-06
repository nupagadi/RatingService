#pragma once

#include <cstdint>
#include <memory>
#include <vector>
#include <array>

#include "Types.hpp"

namespace RatingService
{

struct DataEntry
{
    static const constexpr size_t Size = sizeof(TClientId) + sizeof(TMessageType) + NameMaxSize;

    size_t InfoSize;
    std::array<TByte, Size> ClientInfo;
    double Total;
};

static_assert(std::is_trivial<DataEntry>(), "DataEntry should be trivial.");

struct IData
{
    virtual ~IData() = default;

    virtual void Register(TClientId aClientId, TSharedRawMessage aName, size_t aLength) = 0;

    virtual void Rename(TClientId aClientId, TSharedRawMessage aName, size_t aLength) = 0;

    virtual void AddDeal(TClientId aClientId, double aAmount) = 0;

    virtual std::vector<DataEntry> Copy() const = 0;

    virtual void Drop(size_t aWorkerId) = 0;
};

struct IFactory;

std::unique_ptr<IData> MakeData(size_t aThreadsCount);

}

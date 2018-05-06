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
    std::array<TByte, Size> ClientInfo;
    double Total {};
};

struct IData
{
    virtual ~IData() = default;

    virtual void Register(TClientId aClientId, TSharedRawMessage aName) = 0;

    virtual void Rename(TClientId aClientId, TSharedRawMessage aName) = 0;

    virtual void AddDeal(TClientId aClientId, TSharedRawMessage aName) = 0;

    virtual std::vector<DataEntry> Copy() const = 0;
};

struct IFactory;

std::unique_ptr<IData> MakeData();

}

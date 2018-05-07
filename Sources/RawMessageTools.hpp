#pragma once

#include <cmath>

#include "IFactory.hpp"

namespace RatingService
{

struct RawMessageTools
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

    static const constexpr size_t MinRegisteredSize = 9;
    static const constexpr size_t MaxRegisteredSize = 64;
    static const constexpr size_t DealWonSize = 24;
    static const constexpr size_t ConnectedSize = 8;

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

    static constexpr TTime GetTime(const TByte* aRaw)
    {
        const constexpr int offset = 1;
        return reinterpret_cast<const TTime*>(aRaw)[offset];
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
};

};

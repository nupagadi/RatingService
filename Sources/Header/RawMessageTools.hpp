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

    static constexpr size_t SetMessageType(TByte* aRaw, MessageType aMessageType)
    {
        const constexpr int offset = 1;
        reinterpret_cast<MessageType*>(aRaw)[offset] = aMessageType;
        return sizeof(MessageType);
    }

    static constexpr TClientId GetClientId(const TByte* aRaw)
    {
        const constexpr int offset = 0;
        return reinterpret_cast<const TClientId*>(aRaw)[offset];
    }

    static constexpr size_t SetClientId(TByte* aRaw, TClientId aClientId)
    {
        const constexpr int offset = 0;
        reinterpret_cast<TClientId*>(aRaw)[offset] = aClientId;
        return sizeof(TClientId);
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

    static constexpr size_t SetAmount(TByte* aRaw, double aAmount)
    {
        const constexpr int offset = 2;
        // TODO: Sure, it cannot be this simple. Write double serialization.
        reinterpret_cast<uint64_t*>(aRaw)[offset] = aAmount;
        return sizeof(uint64_t);
    }

    static const constexpr size_t SendingBlockSize = 10;

    static constexpr size_t SendingMessageSize(size_t aSendingBlockSize)
    {
        return sizeof(TClientId)
            + sizeof(RawMessageTools::MessageType)
            + aSendingBlockSize * 3 * sizeof(RatingService::DataEntry)
            + sizeof(RatingService::DataEntry);
    }

    static size_t Serialize(TByte* aDestination, const RatingService::DataEntry& aEntry)
    {
        std::memcpy(aDestination, aEntry.ClientInfo.data(), aEntry.InfoSize);

        return aEntry.InfoSize + SetAmount(aDestination, aEntry.Total);
    }
};

};

#pragma once

#include <cstdint>
#include <memory>
#include <future>

namespace RatingService
{

using TByte = uint8_t;
using TRawMessage = std::unique_ptr<TByte[]>;
using TSharedRawMessage = std::shared_ptr<TByte>;
using TSharedPromise = std::shared_ptr<std::promise<void>>;

using TTime = uint64_t;
using TClientId = uint32_t;
using TMessageType = uint32_t;
const constexpr size_t NameMaxSize = 64;

static const constexpr size_t SendingIntervalSec = 3;  // Possible to use milliseconds.
static const constexpr size_t SpecificClientSendingIntervalSec = 60;
static_assert(!(SpecificClientSendingIntervalSec % SendingIntervalSec), "Should be divisible");
static const constexpr size_t SendingIntervalsCount = SpecificClientSendingIntervalSec / SendingIntervalSec;

static const constexpr size_t TradingPeriodSec = 7 * 24 * 60 * 60;
static const constexpr size_t SomeMondaySec = 1525046400;

constexpr size_t GlobalConnectedContainerIdByTime(TTime aNowSec)
{
    auto minStart = aNowSec / SpecificClientSendingIntervalSec * SpecificClientSendingIntervalSec;
    auto nextSend = aNowSec / SendingIntervalSec * SendingIntervalSec + SendingIntervalSec;
    return (nextSend - minStart) / SendingIntervalSec;
}

constexpr size_t NearestWorkerId(TTime aNowSec, size_t aThreadsCount)
{
    return GlobalConnectedContainerIdByTime(aNowSec) % aThreadsCount;
}

constexpr size_t LocalConnectedContainerIdByTime(TTime aNowSec, size_t aThreadsCount)
{
    return GlobalConnectedContainerIdByTime(aNowSec) / aThreadsCount;
}

constexpr size_t MaxConnectedContainersPerWorker(size_t aThreadsCount)
{
    return SendingIntervalsCount / aThreadsCount + 1;
}

constexpr size_t GlobalConnectedContainerIdByClient(TClientId aClientId)
{
    return aClientId % SendingIntervalsCount;
}

constexpr size_t LocalConnectedContainerIdByClient(TClientId aClientId, size_t aWorkerId)
{
    return GlobalConnectedContainerIdByClient(aClientId) / aWorkerId;
}

}

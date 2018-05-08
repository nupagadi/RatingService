#pragma once

#include <cstdint>
#include <memory>

namespace RatingService
{

using TByte = uint8_t;
using TRawMessage = std::unique_ptr<TByte[]>;
using TSharedRawMessage = std::shared_ptr<TByte>;

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

}

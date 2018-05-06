#pragma once

#include <cstdint>
#include <memory>

namespace RatingService
{

using TByte = uint8_t;
using TRawMessage = std::unique_ptr<TByte[]>;
using TSharedRawMessage = std::shared_ptr<TByte>;

using TClientId = uint32_t;
using TMessageType = uint32_t;
const constexpr size_t NameMaxSize = 64;

}

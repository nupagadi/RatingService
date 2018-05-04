#include "Factory.hpp"

namespace RatingService
{

std::unique_ptr<IFactory> MakeFactory(short aPort, size_t aThreadsCount)
{
    return std::make_unique<Factory>(aPort, aThreadsCount);
}

}

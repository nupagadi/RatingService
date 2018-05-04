#include "Factory.hpp"

namespace RatingService
{

std::unique_ptr<IFactory> MakeFactory()
{
    return std::make_unique<Factory>();
}

}

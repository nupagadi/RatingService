#pragma once

#include <memory>

namespace RatingService
{

struct IService
{
    virtual ~IService() = default;

    // TODO: Use Poll instead.
    virtual void Run() = 0;
};

std::shared_ptr<IService> MakeSharedService(short aPort);

}

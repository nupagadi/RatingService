#pragma once

#include <memory>
#include <boost/system/error_code.hpp>

namespace RatingService
{

struct IService
{
    virtual ~IService() = default;

    // TODO: Use Poll instead.
    virtual void Run() = 0;

    virtual void Receive(const boost::system::error_code& aErrorCode) = 0;
};

std::shared_ptr<IService> MakeSharedService(short aPort);

}

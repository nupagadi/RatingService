#pragma once

#include <memory>
#include <boost/optional.hpp>
#include <boost/system/error_code.hpp>

namespace RatingService
{

struct IManager;
struct IFactory;

struct IService
{
    virtual ~IService() = default;

    // TODO: Use Poll instead.
    virtual void Run() = 0;

    // TODO: Ok?
    virtual void Stop(bool aForce) = 0;

    virtual void OnAccept(const boost::system::error_code& aErrorCode) = 0;

    virtual void OnReceive(const boost::system::error_code& aErrorCode, const size_t& aLength) = 0;

    virtual size_t Notify(size_t aTimePointEpochSec, size_t aRepeatSec) = 0;
};

std::shared_ptr<IService> MakeSharedService(IFactory *aFactory, IManager *aManager, short aAcceptorPort);

}

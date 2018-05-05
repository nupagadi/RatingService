#pragma once

#include <memory>
#include <boost/system/error_code.hpp>

#include "Handlers.hpp"

namespace RatingService
{

struct IService;

struct IAsioService
{
    virtual ~IAsioService() = default;

    // TODO: Use Poll instead.
    virtual void Run() = 0;

    virtual void Post(TSharedRawMessage) = 0;

    // TODO: Ok?
    virtual void Stop(bool aForce) = 0;
};

struct IAsioSocket
{
    // Shoudl comply Boost.Asio Read handler requirements.
    using TReadCallback = Callback<IService, void(const boost::system::error_code&, const size_t&)>;

    virtual ~IAsioSocket() = default;

    virtual void Receive(uint8_t* aBuffer, size_t aMaxLength, TReadCallback) = 0;
};

// TODO: Template for IService.
struct IAsioAcceptor
{
    // Should comply Boost.Asio Accept handler requirements.
    using TAcceptCallback = Callback<IService, void(const boost::system::error_code&)>;

    virtual ~IAsioAcceptor() = default;

    virtual void Accept(IAsioSocket* aSocket, TAcceptCallback aCallback) = 0;
};

}

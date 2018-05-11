#pragma once

#include <memory>
#include <chrono>
#include <boost/system/error_code.hpp>

#include "Handlers.hpp"

namespace RatingService
{

struct IService;

struct IAsioService
{
    virtual ~IAsioService() = default;

    virtual void Run() = 0;

    virtual void Post(TSharedRawMessageTask) = 0;

    virtual void Post(TManagerSharedRawMessageTask) = 0;

    virtual void Post(TWaitTask) = 0;

    virtual void Post(TSendInfoTask) = 0;

    virtual void Post(TDropDataTask) = 0;

    virtual void Post(TConnectedTask) = 0;

    // TODO: Ok?
    virtual void Stop(bool aForce) = 0;
};

struct IAsioSocket
{
    // Should comply Boost.Asio Read (Write) handler requirements.
    using TReadCallback = Callback<IService, void(const boost::system::error_code&, const size_t&)>;
    using TWriteCallback = std::function<void(const boost::system::error_code&, const size_t&)>;

    virtual ~IAsioSocket() = default;

    virtual void Receive(TByte* aBuffer, size_t aMaxLength, TReadCallback) = 0;

    virtual void Send(const TByte* aBuffer, size_t aLength, TWriteCallback) = 0;

    virtual void Close() = 0;
};

// TODO: Template for IService.
struct IAsioAcceptor
{
    // Should comply Boost.Asio Accept handler requirements.
    using TAcceptCallback = Callback<IService, void(const boost::system::error_code&)>;

    virtual ~IAsioAcceptor() = default;

    virtual void Accept(IAsioSocket* aSocket, TAcceptCallback aCallback) = 0;

    virtual void Cancel() = 0;
};

struct IAsioTimer
{
    virtual ~IAsioTimer() = default;

    virtual std::chrono::system_clock::time_point ExpiresAt() const = 0;

    virtual void ExpiresAt(const std::chrono::system_clock::time_point& aTimePoint) = 0;

    virtual void Wait(const std::function<void(const boost::system::error_code&)>& aCallback) = 0;

    virtual void Cancel() = 0;
};

struct IAsioSignals
{
    virtual ~IAsioSignals() = default;

    virtual void Wait(
        const std::function<void(const boost::system::error_code&, const int& aSignalNumber)>& aCallback) = 0;
};

std::unique_ptr<IAsioService> MakeAsioService();

std::unique_ptr<IAsioSocket> MakeAsioSocket(IAsioService* aAsioService);

std::unique_ptr<IAsioAcceptor> MakeAsioAcceptor(IAsioService* aAsioService, short aPort);

std::unique_ptr<IAsioTimer> MakeAsioTimer(IAsioService* aAsioService);

std::unique_ptr<IAsioSignals> MakeAsioSignals(IAsioService* aAsioService);

}

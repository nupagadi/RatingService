#pragma once

#include <memory>
#include <boost/system/error_code.hpp>

namespace RatingService
{

template <typename TCallee, typename TCall>
struct Callback;

template <typename TCallee, typename R, typename ... TArgs>
struct Callback<TCallee, R (TArgs...)>
{
    using TCall = R(TCallee::*)(TArgs...);

    Callback(TCallee& aCallee, TCall aCall)
        : mCallee(aCallee)
        , mCall(aCall)
    {
    }

    R operator()(TArgs&&... aArgs)
    {
        return (mCallee.*mCall)(std::forward<TArgs>(aArgs)...);
    }

    bool operator ==(const Callback& aRighthand) const
    {
        return &mCallee == &aRighthand.mCallee && mCall == aRighthand.mCall;
    }

private:

    TCallee& mCallee;
    TCall mCall;
};

struct IService;

struct IAsioService
{
    virtual ~IAsioService() = default;

    // TODO: Use Poll instead.
    virtual void Run() = 0;
};

struct IAsioSocket
{
    // Shoudl comply Boost.Asio Read handler requirements.
    using TReadCallback = Callback<IService, void(const boost::system::error_code&, const size_t&)>;

    virtual ~IAsioSocket() = default;

    virtual void Receive(char* aBuffer, size_t aMaxLength, TReadCallback) = 0;
};

// TODO: Template for IService.
struct IAsioAcceptor
{
    // Should comply Boost.Asio Accept handler requirements.
    using TAcceptCallback = Callback<IService, void(const boost::system::error_code&)>;

    virtual ~IAsioAcceptor() = default;

    virtual void Accept(IAsioSocket* aSocket, TAcceptCallback aCallback) = 0;
};

template <typename ...TArgs>
std::unique_ptr<IAsioService> MakeAsioService(TArgs&&...);
template <typename ...TArgs>
std::unique_ptr<IAsioSocket> MakeAsioSocket(TArgs&&...);
template <typename ...TArgs>
std::unique_ptr<IAsioAcceptor> MakeAsioAcceptor(TArgs&&...);

}

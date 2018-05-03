#include <boost/asio.hpp>

#include "IAsio.hpp"

namespace RatingService
{

struct AsioService : IAsioService
{
    friend class AsioSocket;
    friend class AsioAcceptor;

    void Run() override
    {
        mIoService.run();
    }

private:

    boost::asio::io_service mIoService;
};

struct AsioSocket : IAsioSocket
{
    friend class AsioAcceptor;

    AsioSocket(AsioService* aService)
        : mSocket((assert(aService), aService->mIoService))
    {
    }

    void Receive(
        char* aBuffer,
        size_t aMaxLength,
        std::function<void(boost::system::error_code, size_t)> aCallback) override
    {
        mSocket.async_read_some(boost::asio::buffer(aBuffer, aMaxLength), std::move(aCallback));
    }

private:

    boost::asio::ip::tcp::socket mSocket;
};

struct AsioAcceptor : IAsioAcceptor
{
    AsioAcceptor(AsioService* aService, short aPort)
        : mAcceptor(
            (assert(aService), aService->mIoService),
            boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), aPort))
    {
    }

    void Accept(IAsioSocket* aAsioSocket, TAcceptCallback aCallback) override
    {
        assert(aAsioSocket);
        auto concreteSocket = dynamic_cast<AsioSocket*>(aAsioSocket);
        assert(concreteSocket);
        mAcceptor.async_accept(concreteSocket->mSocket, aCallback);
    }

private:

    boost::asio::ip::tcp::acceptor mAcceptor;
};

template <typename ...TArgs>
std::unique_ptr<IAsioService> MakeAsioService(TArgs&&...aArgs)
{
    return std::make_unique<AsioService>(std::forward<TArgs>(aArgs)...);
}

template <typename ...TArgs>
std::unique_ptr<IAsioSocket> MakeAsioSocket(TArgs&&...aArgs)
{
    return std::make_unique<AsioSocket>(std::forward<TArgs>(aArgs)...);
}

template <typename ...TArgs>
std::unique_ptr<IAsioAcceptor> MakeAsioAcceptor(TArgs&&...aArgs)
{
    return std::make_unique<AsioAcceptor>(std::forward<TArgs>(aArgs)...);
}

}

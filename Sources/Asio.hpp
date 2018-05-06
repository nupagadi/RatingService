#include <boost/asio.hpp>
#include <boost/optional.hpp>

#include "IWorker.hpp"
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
        assert(mIoService.stopped());
    }

    void Post(TSharedRawMessageTask aMessage) override
    {
        mIoService.post(std::move(aMessage));
    }

    void Stop(bool aForce) override
    {
        mWork = boost::none;
        if (aForce)
        {
            mIoService.stop();
        }
        // TODO: else?
    }

private:

    boost::asio::io_service mIoService;
    boost::optional<boost::asio::io_service::work> mWork{mIoService};
};

struct AsioSocket : IAsioSocket
{
    friend class AsioAcceptor;

    AsioSocket(AsioService* aService)
        : mSocket((assert(aService), aService->mIoService))
    {
    }

    void Receive(
        uint8_t* aBuffer,
        size_t aMaxLength,
        TReadCallback aCallback) override
    {
        mSocket.async_read_some(boost::asio::buffer(aBuffer, aMaxLength), aCallback);
    }

    void Close()
    {
        boost::system::error_code ec;
        mSocket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
        mSocket.close();
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

std::unique_ptr<IAsioService> MakeAsioService()
{
    return std::make_unique<AsioService>();
}

std::unique_ptr<IAsioSocket> MakeAsioSocket(IAsioService* aAsioService)
{
    return std::make_unique<AsioSocket>(dynamic_cast<AsioService*>(aAsioService));
}

std::unique_ptr<IAsioAcceptor> MakeAsioAcceptor(IAsioService* aAsioService, short aPort)
{
    // TODO: Operate:
    // terminate called after throwing an instance of
    // 'boost::exception_detail::clone_impl<boost::exception_detail::error_info_injector<boost::system::system_error> >'
    // what():  bind: Address already in use
    return std::make_unique<AsioAcceptor>(dynamic_cast<AsioService*>(aAsioService), aPort);
}

}

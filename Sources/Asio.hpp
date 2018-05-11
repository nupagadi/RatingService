#include <boost/asio.hpp>
#include <boost/optional.hpp>
#include <boost/asio/system_timer.hpp>

#include "IWorker.hpp"
#include "IManager.hpp"
#include "IAsio.hpp"

namespace RatingService
{

struct AsioService : IAsioService
{
    friend class AsioTimer;
    friend class AsioSocket;
    friend class AsioAcceptor;
    friend class AsioSignals;

    void Run() override
    {
        mIoService.run();
        assert(mIoService.stopped());
    }

    void Post(TSharedRawMessageTask aMessage) override
    {
        mIoService.post(std::move(aMessage));
    }

    void Post(TManagerSharedRawMessageTask aMessage) override
    {
        mIoService.post(std::move(aMessage));
    }

    void Post(TWaitTask aMessage) override
    {
        mIoService.post(std::move(aMessage));
    }

    void Post(TSendInfoTask aMessage) override
    {
        mIoService.post(std::move(aMessage));
    }

    void Post(TDropDataTask aMessage) override
    {
        mIoService.post(std::move(aMessage));
    }

    void Post(TConnectedTask aMessage) override
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

    void Receive(uint8_t* aBuffer, size_t aMaxLength, TReadCallback aCallback) override
    {
        mSocket.async_read_some(boost::asio::buffer(aBuffer, aMaxLength), aCallback);
    }

    void Send(const TByte* aBuffer, size_t aLength, TWriteCallback aCallback) override
    {
        boost::asio::async_write(mSocket, boost::asio::buffer(aBuffer, aLength), aCallback);
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

struct AsioTimer : IAsioTimer
{
    AsioTimer(AsioService* aService)
        : mTimer((assert(aService), aService->mIoService))
    {
    }

    std::chrono::system_clock::time_point ExpiresAt() const override
    {
        return mTimer.expires_at();
    }

    void ExpiresAt(const std::chrono::system_clock::time_point& aTimePoint) override
    {
        mTimer.expires_at(aTimePoint);
    }

    void Wait(const std::function<void(const boost::system::error_code&)>& aCallback) override
    {
        mTimer.async_wait(aCallback);
    }

private:

    boost::asio::system_timer mTimer;
};

struct AsioSignals : IAsioSignals
{
    AsioSignals(AsioService* aService)
        : mSignals((assert(aService), aService->mIoService), SIGINT, SIGTERM)
    {
    }

    void Wait(
        const std::function<void(const boost::system::error_code&, const int& aSignalNumber)>& aCallback) override
    {
        mSignals.async_wait(aCallback);
    }

private:

    boost::asio::signal_set mSignals;
};

std::unique_ptr<IAsioService> MakeAsioService()
{
    return std::make_unique<AsioService>();
}

std::unique_ptr<IAsioSocket> MakeAsioSocket(IAsioService* aAsioService)
{
    return std::make_unique<AsioSocket>(dynamic_cast<AsioService*>(aAsioService));
}

std::unique_ptr<IAsioTimer> MakeAsioTimer(IAsioService* aAsioService)
{
    return std::make_unique<AsioTimer>(dynamic_cast<AsioService*>(aAsioService));
}

std::unique_ptr<IAsioAcceptor> MakeAsioAcceptor(IAsioService* aAsioService, short aPort)
{
    // TODO: Operate:
    // terminate called after throwing an instance of
    // 'boost::exception_detail::clone_impl<boost::exception_detail::error_info_injector<boost::system::system_error> >'
    // what():  bind: Address already in use
    return std::make_unique<AsioAcceptor>(dynamic_cast<AsioService*>(aAsioService), aPort);
}

std::unique_ptr<IAsioSignals> MakeAsioSignals(IAsioService* aAsioService)
{
    return std::make_unique<AsioSignals>(dynamic_cast<AsioService*>(aAsioService));
}

}

#include "IService.hpp"
#include "IAsio.hpp"

namespace RatingService
{

struct Service : std::enable_shared_from_this<Service>, IService
{
    Service(
        std::unique_ptr<IAsioService>&& aService,
        std::unique_ptr<IAsioAcceptor>&& aAcceptor,
        std::unique_ptr<IAsioSocket>&& aSocket,
        short aPort)
        : mService(std::move(aService))
        , mAcceptor(std::move(aAcceptor))
        , mSocket(std::move(aSocket))
        , mAcceptCallback(*this, &IService::OnAccept)
        , mReadCallback(*this, &IService::OnReceive)
        , mPort(aPort)
    {
    }

    void Run()
    {
        Accept();
    }

    void OnAccept(const boost::system::error_code& aErrorCode) override
    {
        if (!aErrorCode)
        {
            Receive();
        }
        else
        {
            // TODO: Use logger.
            std::cerr << "Service::OnAccept: " << aErrorCode << std::endl;
        }

    }

    void OnReceive(const boost::system::error_code &aErrorCode, const size_t& aLength) override
    {
        // TODO: Post a task.
    }

private:

    void Accept()
    {
        mAcceptor->Accept(mSocket.get(), mAcceptCallback);
    }

    void Receive()
    {
        mBuffer = std::make_unique<char[]>(MaxPacketSize);
        mSocket->Receive(mBuffer.get(), MaxPacketSize, mReadCallback);
    }

    void Send();

private:

    // TODO: May be decrease it.
    static const constexpr size_t MaxPacketSize = 1024;

    std::unique_ptr<IAsioService> mService;
    std::unique_ptr<IAsioAcceptor> mAcceptor;
    std::unique_ptr<IAsioSocket> mSocket;

    IAsioAcceptor::TAcceptCallback mAcceptCallback;
    IAsioSocket::TReadCallback mReadCallback;

    std::unique_ptr<char[]> mBuffer;

    // TODO: Remove?
    short mPort {};
};

}

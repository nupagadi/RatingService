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
        , mAcceptCallback(&IService::OnAccept)
        , mReadCallback(&IService::OnReceive)
        , mPort(aPort)
    {
    }

    void Run()
    {
        mAcceptCallback.SetCallee(shared_from_this());
        mReadCallback.SetCallee(shared_from_this());

        // TODO: Set signals.
        // TODO: Set timers.

        Accept();

        mService->Run();
    }

    // TODO: Implement?
    void Stop();

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
        if (!aErrorCode)
        {
            // TODO: Post a task.

            std::cout << "Length: " << aLength;
            std::cout.write(mBuffer.get(), aLength) << std::flush;
//            std::cout << mBuffer.get() << std::flush;

            Receive();
        }
        else
        {
            // TODO: Use logger.
            std::cerr << "Service::OnReceive: " << aErrorCode << std::endl;
        }
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

    // TODO: Implement?
    void Send();

private:

    // TODO: May be decrease it.
    static const constexpr size_t MaxPacketSize = 16;

    std::unique_ptr<IAsioService> mService;
    std::unique_ptr<IAsioAcceptor> mAcceptor;
    std::unique_ptr<IAsioSocket> mSocket;

    IAsioAcceptor::TAcceptCallback mAcceptCallback;
    IAsioSocket::TReadCallback mReadCallback;

    std::unique_ptr<char[]> mBuffer;

    // TODO: Remove? Need for Acceptor recreation?
    short mPort {};
};

}

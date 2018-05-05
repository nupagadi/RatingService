#pragma once

#include <iostream>

#include "IManager.hpp"
#include "IService.hpp"
#include "IAsio.hpp"
#include "IFactory.hpp"

namespace RatingService
{

struct Service : std::enable_shared_from_this<Service>, IService
{
    Service(
        IFactory* aFactory,
        std::unique_ptr<IAsioService>&& aService,
        std::unique_ptr<IAsioAcceptor>&& aAcceptor,
        std::unique_ptr<IAsioSocket>&& aSocket,
        IManager* aManager)
        : mFactory(aFactory)
        , mService(std::move(aService))
        , mAcceptor(std::move(aAcceptor))
        , mSocket(std::move(aSocket))
        , mManager(aManager)
        , mAcceptCallback(&IService::OnAccept)
        , mReadCallback(&IService::OnReceive)
    {
        assert(mFactory);
        assert(mService);
        assert(mAcceptor);
        assert(mSocket);
        assert(mManager);
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
    void Stop(bool aForce)
    {
        mService->Stop(aForce);
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
        if (!aErrorCode)
        {
            std::cout << "Length: " << aLength;
            std::cout.write(reinterpret_cast<const char*>(mBuffer.get()), aLength) << std::flush;
//            std::cout << mBuffer.get() << std::flush;

            // TODO: Check if the message is complete (\r\n --> \0).
            mManager->ProcessMessageFromNet(std::move(mBuffer), aLength);

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
        mBuffer = std::make_unique<uint8_t[]>(MaxPacketSize);
        mSocket->Receive(mBuffer.get(), MaxPacketSize, mReadCallback);
    }

    // TODO: Implement?
    void Send();

private:

    // TODO: May be decrease it.
    static const constexpr size_t MaxPacketSize = 1024;

    IFactory* mFactory;

    std::unique_ptr<IAsioService> mService;
    std::unique_ptr<IAsioAcceptor> mAcceptor;
    std::unique_ptr<IAsioSocket> mSocket;
    IManager* mManager;

    IAsioAcceptor::TAcceptCallback mAcceptCallback;
    IAsioSocket::TReadCallback mReadCallback;

    std::unique_ptr<uint8_t[]> mBuffer;
};

std::shared_ptr<IService> MakeSharedService(IFactory* aFactory, IManager* aManager, short aAcceptorPort)
{
    auto asioService = MakeAsioService();
    auto asioSocket = MakeAsioSocket(asioService.get());
    auto asioAcceptor = MakeAsioAcceptor(asioService.get(), aAcceptorPort);

    return std::make_shared<Service>(
        aFactory, std::move(asioService), std::move(asioAcceptor), std::move(asioSocket), aManager);
}

}

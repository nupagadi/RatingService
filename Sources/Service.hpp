#pragma once

#include <cstring>
#include <iostream>
#include <array>
#include <boost/asio/error.hpp>

#include "IManager.hpp"
#include "IService.hpp"
#include "IAsio.hpp"
#include "IFactory.hpp"

// TODO: Remove.
#include <boost/asio/system_timer.hpp>
#include <chrono>
#include <thread>

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
        mTimers.reserve(2);
    }

    void OnTimer(const boost::system::error_code& aErrorCode, size_t aPeriod, size_t aId, IAsioTimer& aTimer)
    {
        if (aErrorCode)
        {
            std::cout << "Service::OnTimer: " << aErrorCode << std::endl;
            return;
        }

        auto now = aTimer.ExpiresAt();
        aTimer.ExpiresAt(now + std::chrono::seconds(aPeriod));
        aTimer.Wait(
            std::bind(&Service::OnTimer, shared_from_this(), std::placeholders::_1, aPeriod, aId, std::ref(aTimer)));

        mManager->ProcessNotify(aId, std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count());
    }

    void Run() override
    {
        mAcceptCallback.SetCallee(shared_from_this());
        mReadCallback.SetCallee(shared_from_this());

        Accept();

        mService->Run();
    }

    // TODO: Implement?
    void Stop(bool aForce) override
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
            std::cout << "Length:" << aLength << std::endl;
            std::cout.write(reinterpret_cast<const char*>(mBuffer.data()), aLength) << std::endl;
//            std::cout << mBuffer.get() << std::flush;

            auto begin = mBuffer.data();
            auto findBegin = begin;
            while (auto end = static_cast<uint8_t*>(
                std::memchr(findBegin, '\r', mBuffer.data() + aLength - findBegin)))
            {
                if (end[1] != '\n')
                {
                    findBegin = end + 1;
                    continue;
                }

                auto remainderBufferSize = mRemainderBuffer.size();
                auto message = std::make_unique<TByte[]>(end - begin + remainderBufferSize);
                std::memcpy(message.get(), mRemainderBuffer.data(), remainderBufferSize);
                std::memcpy(message.get() + remainderBufferSize, begin, end - begin);
                mManager->ProcessMessageFromNet(std::move(message), end - begin + remainderBufferSize);
                mRemainderBuffer.clear();

                findBegin = begin = end + 2;
            }

            auto remainderCount = mBuffer.data() + aLength - findBegin;
            if (remainderCount)
            {
                mRemainderBuffer.assign(mBuffer.data(), mBuffer.data() + remainderCount);
            }

            Receive();
        }
        else if (aErrorCode == boost::asio::error::eof)
        {
            mSocket->Close();
            std::cout << "Connection lost." << std::endl;
            Accept();
        }
        else
        {
            // TODO: Use logger.
            std::cerr << "Service::OnReceive: " << aErrorCode << std::endl;
        }
    }

    size_t Notify(size_t aTimePointEpochSec, size_t aRepeatSec) override
    {
        static size_t id = 1;

        mTimers.emplace_back(mFactory->MakeAsioTimer(mService.get()));

        mTimers.back()->ExpiresAt(std::chrono::system_clock::time_point(std::chrono::seconds(aTimePointEpochSec)));

        mTimers.back()->Wait(std::bind(
            &Service::OnTimer,
            shared_from_this(),
            std::placeholders::_1,
            aRepeatSec,
            id,
            std::ref(*mTimers.back())));

        return id++;
    }

private:

    void Accept()
    {
        mAcceptor->Accept(mSocket.get(), mAcceptCallback);
    }

    void Receive()
    {
        mSocket->Receive(mBuffer.data(), MaxNetPacketSize, mReadCallback);
    }

    // TODO: Implement?
    void Send();

private:

    // TODO: May be decrease it.
    static const constexpr size_t MaxNetPacketSize = 1024;

    IFactory* mFactory;

    std::unique_ptr<IAsioService> mService;
    std::unique_ptr<IAsioAcceptor> mAcceptor;
    std::unique_ptr<IAsioSocket> mSocket;
    std::vector<std::unique_ptr<IAsioTimer>> mTimers;

    IManager* mManager;

    IAsioAcceptor::TAcceptCallback mAcceptCallback;
    IAsioSocket::TReadCallback mReadCallback;

    std::array<TByte, MaxNetPacketSize> mBuffer;
    // Will be used rarely.
    std::string mRemainderBuffer;
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

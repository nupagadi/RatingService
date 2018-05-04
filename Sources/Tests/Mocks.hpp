#pragma once

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "../IAsio.hpp"
#include "../IManager.hpp"
#include "../IService.hpp"

namespace RatingService
{

namespace Tests
{

using ::testing::_;
using ::testing::StrictMock;
using ::testing::Ref;

struct AsioServiceMock : IAsioService
{
    MOCK_METHOD0(Run, void());

    MOCK_METHOD1(Stop, void(bool aForce));
};

struct AsioAcceptorMock : IAsioAcceptor
{
    MOCK_METHOD2(Accept, void(IAsioSocket* aSocket, TAcceptCallback aCallback));
};

struct AsioSocketMock : IAsioSocket
{
     MOCK_METHOD3(Receive, void(char* aBuffer, size_t aMaxLength, TReadCallback aCallback));
};

struct ManagerMock : IManager
{
    MOCK_METHOD0(Run, void());

    // GMock doesn't support move-only arguments.
    MOCK_METHOD1(ProcessMessageFromNet, void(const std::unique_ptr<char[]>& aMessage));
};

struct ServiceMock : IService
{
    MOCK_METHOD0(Run, void());

    MOCK_METHOD1(Stop, void(bool aForce));

    MOCK_METHOD1(OnAccept, void(const boost::system::error_code& aErrorCode));

    MOCK_METHOD2(OnReceive, void(const boost::system::error_code& aErrorCode, const size_t& aLength));
};

}

}

#pragma once

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "../IAsio.hpp"
#include "../IManager.hpp"

namespace RatingService
{

namespace Tests
{

using ::testing::_;
using ::testing::StrictMock;

struct AsioServiceMock : IAsioService
{
    MOCK_METHOD0(Run, void());
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
    MOCK_METHOD1(ProcessMessageFromNet, void(const std::unique_ptr<char[]>& aMessage));
};

}

}

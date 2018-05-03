#pragma once

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "../IAsio.hpp"

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
     MOCK_METHOD3(Receive,
        void(char* aBuffer, size_t aMaxLength, std::function<void(boost::system::error_code, size_t)> aCallback));
};

}

}

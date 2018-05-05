#include <cstring>

#include "Mocks.hpp"
#include "../Service.hpp"

namespace RatingService
{

namespace Tests
{

struct ServiceTests : ::testing::Test
{
    MockFactory Factory;

    std::shared_ptr<RatingService::Service> Service;
    AsioServiceMock* AsioService;
    AsioSocketMock* AsioSocket;
    AsioAcceptorMock* AsioAcceptor;
    std::unique_ptr<ManagerMock> Manager;

    void SetUp() override
    {
        auto asioService = Factory.MakeMock<StrictMock<AsioServiceMock>>();
        auto asioSocket = Factory.MakeMock<StrictMock<AsioSocketMock>>();
        auto asioAcceptor = Factory.MakeMock<StrictMock<AsioAcceptorMock>>();

        AsioService = asioService.get();
        AsioSocket = asioSocket.get();
        AsioAcceptor = asioAcceptor.get();
        Manager = Factory.MakeMock<StrictMock<ManagerMock>>();

        // Try Factory method.
        Service = std::make_shared<RatingService::Service>(
            &Factory, std::move(asioService), std::move(asioAcceptor), std::move(asioSocket), Manager.get());
    }

    void TearDown() override
    {
        // It seems GMock can't work well with shared_from_this() objects.
        ::testing::Mock::AllowLeak(AsioAcceptor);
        ::testing::Mock::AllowLeak(AsioService);
        ::testing::Mock::AllowLeak(AsioSocket);
    }
};

// TODO: Use Poll instead.
TEST_F(ServiceTests, ShouldAcceptOnRun)
{
    IAsioAcceptor::TAcceptCallback callback{Service, &IService::OnAccept};

    EXPECT_CALL(*AsioAcceptor, Accept(AsioSocket, callback));

    EXPECT_CALL(*AsioService, Run());

    Service->Run();
}

TEST_F(ServiceTests, ShouldReceiveOnAccessSucceeded)
{
    EXPECT_CALL(*AsioAcceptor, Accept(_, _));

    EXPECT_CALL(*AsioService, Run());

    Service->Run();

    // TODO: Use buffer factory.
    EXPECT_CALL(*AsioSocket, Receive(_, 1024, IAsioSocket::TReadCallback{Service, &IService::OnReceive}));

    auto success = boost::system::error_code{};
    Service->OnAccept(success);
}

TEST_F(ServiceTests, ShouldPassNetworkMessageToManager)
{
    size_t length = 10;
    auto message = std::make_unique<uint8_t[]>(length);
    std::memcpy(message.get(), "12345678\r\n", length);

    EXPECT_CALL(*AsioSocket, Receive(_, _, _));
    Service->OnAccept({});

    // TODO: Cannot check passed string content. Need buffer factory.
    EXPECT_CALL(*Manager, ProcessMessageFromNetProxy(_, _));
    EXPECT_CALL(*AsioSocket, Receive(_, _, _));

    auto success = boost::system::error_code{};
    Service->OnReceive(success, length);
}

// TODO: Need buffer factory.
// TODO: Assume message will not be too long?
TEST_F(ServiceTests, ShouldJoinIfSeveralPackets)
{

}

// TODO: Need buffer factory.
TEST_F(ServiceTests, ShouldChangeMessageEndWithNull)
{

}

}
}



















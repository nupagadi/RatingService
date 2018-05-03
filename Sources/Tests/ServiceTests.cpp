#include <cstring>

#include "Mocks.hpp"
#include "../Service.hpp"

namespace RatingService
{

namespace Tests
{

struct MockedService
{
    std::shared_ptr<RatingService::Service> Service;
    AsioServiceMock& AsioService;
    AsioSocketMock& AsioSocket;
    AsioAcceptorMock& AsioAcceptor;
    std::unique_ptr<ManagerMock> Manager;
};

auto MakeMockedService()
{
    auto asioService = std::make_unique<StrictMock<AsioServiceMock>>();
    auto asioSocket = std::make_unique<StrictMock<AsioSocketMock>>();
    auto asioAcceptor = std::make_unique<StrictMock<AsioAcceptorMock>>();
    auto manager = std::make_unique<StrictMock<ManagerMock>>();

    auto mocked = MockedService{nullptr, *asioService, *asioSocket, *asioAcceptor, std::move(manager)};
    mocked.Service =
        std::make_shared<Service>(std::move(asioService), std::move(asioAcceptor), std::move(asioSocket), 21345);
    return std::move(mocked);
}

struct ServiceTests : ::testing::Test
{
    std::shared_ptr<RatingService::Service> Service;
    AsioServiceMock* AsioService;
    AsioSocketMock* AsioSocket;
    AsioAcceptorMock* AsioAcceptor;
    std::unique_ptr<ManagerMock> Manager;

    void SetUp() override
    {
        auto asioService = std::make_unique<StrictMock<AsioServiceMock>>();
        auto asioSocket = std::make_unique<StrictMock<AsioSocketMock>>();
        auto asioAcceptor = std::make_unique<StrictMock<AsioAcceptorMock>>();

        AsioService = asioService.get();
        AsioSocket = asioSocket.get();
        AsioAcceptor = asioAcceptor.get();
        Manager = std::make_unique<StrictMock<ManagerMock>>();

        Service = std::make_shared<RatingService::Service>(
            std::move(asioService), std::move(asioAcceptor), std::move(asioSocket), 21345);
    }

    void TearDown() override
    {
        // Seems GMock can't work properly with shared_from_this() objects.
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

    // TODO: Create buffer factory.
    EXPECT_CALL(
        *AsioSocket,
        Receive(::testing::StrEq(""), 1024, IAsioSocket::TReadCallback{Service, &IService::OnReceive}));

    auto success = boost::system::error_code{};
    Service->OnAccept(success);
}

TEST_F(ServiceTests, ShouldPassNetworkMessageToManager)
{
    EXPECT_CALL(*AsioSocket, Receive(_, _, _));
    Service->OnAccept({});

    size_t length = 10;
    auto message = std::make_unique<char[]>(length);
    std::strncpy(message.get(), "12345678\r\n", length);

    EXPECT_CALL(*Manager, ProcessMessageFromNet(Ref(message)));
    EXPECT_CALL(*AsioSocket, Receive(_, _, _));

    auto success = boost::system::error_code{};
    Service->OnReceive(success, length);
}

}
}



















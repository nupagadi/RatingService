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

// TODO: Use Poll instead.
TEST(ServiceTests, ShouldAcceptOnRun)
{
    auto mocked = MakeMockedService();

    EXPECT_CALL(
        mocked.AsioAcceptor,
        Accept(&mocked.AsioSocket, IAsioAcceptor::TAcceptCallback{mocked.Service, &IService::OnAccept}));

    mocked.Service->Run();
}

TEST(ServiceTests, ShouldReceiveAccessSucceeded)
{
    auto mocked = MakeMockedService();

    // TODO: Create buffer factory.
    EXPECT_CALL(
        mocked.AsioSocket,
        Receive(::testing::StrEq(""), 1024, IAsioSocket::TReadCallback{mocked.Service, &IService::OnReceive}));

    auto success = boost::system::error_code{};
    mocked.Service->OnAccept(success);
}

}
}

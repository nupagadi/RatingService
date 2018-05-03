//#include <boost/system/error_code.hpp>

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
};

auto MakeMockedService()
{
    auto asioService = std::make_unique<StrictMock<AsioServiceMock>>();
    auto asioSocket = std::make_unique<StrictMock<AsioSocketMock>>();
    auto asioAcceptor = std::make_unique<StrictMock<AsioAcceptorMock>>();

    auto mocked = MockedService{nullptr, *asioService, *asioSocket, *asioAcceptor};
    mocked.Service =
        std::make_shared<Service>(std::move(asioService), std::move(asioAcceptor), std::move(asioSocket), 21345);
    return std::move(mocked);
}

// TODO: Use Poll instead.
TEST(ServiceTests, ShouldAcceptOnRun)
{
    auto mocked = MakeMockedService();

    EXPECT_CALL(mocked.AsioAcceptor, Accept(&mocked.AsioSocket, _));

    mocked.Service->Run();
}

TEST(ServiceTests, ShouldPushTaskOnReceive)
{
    auto mocked = MakeMockedService();

    EXPECT_CALL(mocked.AsioAcceptor, Accept(&mocked.AsioSocket, _));

    mocked.Service->Receive(boost::system::error_code{});
}

}
}

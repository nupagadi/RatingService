#include "Mocks.hpp"
#include "../Manager.hpp"

namespace RatingService
{

namespace Tests
{

struct ManagerTests : ::testing::Test
{
    RatingService::Manager Manager;
    ServiceMock* Service;

    void SetUp() override
    {
        auto service = std::make_unique<StrictMock<ServiceMock>>();

        Service = service.get();

//        Manager = std::make_shared<RatingService::Service>(
//            std::move(asioService), std::move(asioAcceptor), std::move(asioSocket), Manager.get(), 21345);
    }

    void TearDown() override
    {
        // Seems GMock can't work properly with shared_from_this() objects.
//        ::testing::Mock::AllowLeak(AsioAcceptor);
//        ::testing::Mock::AllowLeak(AsioService);
//        ::testing::Mock::AllowLeak(AsioSocket);
    }
};


}
}

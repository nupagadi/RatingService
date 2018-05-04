#include "Mocks.hpp"
#include "../Manager.hpp"

namespace RatingService
{

namespace Tests
{

struct ManagerTests : ::testing::Test
{
    MockFactory Factory;

    std::unique_ptr<RatingService::Manager> Manager;
    ServiceMock* Service;
    std::vector<IWorker*> Workers;

    void SetUp() override
    {
        Service = new StrictMock<ServiceMock>;

        EXPECT_CALL(Factory, MakeSharedServiceProxy(_)).WillOnce(Return(Service));
        Manager = std::make_unique<RatingService::Manager>(&Factory);
    }

    void TearDown() override
    {
        // Seems GMock can't work properly with shared_from_this() objects.
//        ::testing::Mock::AllowLeak(AsioAcceptor);
//        ::testing::Mock::AllowLeak(AsioService);
//        ::testing::Mock::AllowLeak(AsioSocket);
    }
};

// TODO: ShouldPollService.
TEST_F(ManagerTests, ShouldRunAllOnRun)
{
    EXPECT_CALL(*Service, Run());

    Manager->Run();
}

TEST_F(ManagerTests, ShouldPostToAppropriateWorker)
{
}

}
}

#include "Mocks.hpp"
#include "../Worker.hpp"

namespace RatingService
{

namespace Tests
{

struct WorkerTests : ::testing::Test
{
    MockFactory Factory;

    std::unique_ptr<RatingService::Worker> Worker;

    void SetUp() override
    {
        Factory.MakeManager(&Factory);
        Worker = std::make_unique<RatingService::Worker>(&Factory, Factory.Manager);
    }

    void TearDown() override
    {
    }
};

TEST_F(WorkerTests, ShouldRunServiceOnRun)
{
    ASSERT_TRUE(Factory.AsioService);
    EXPECT_CALL(*Factory.AsioService, Run());

    Worker->Run();
}

}
}

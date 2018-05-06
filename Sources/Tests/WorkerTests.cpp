#include "Mocks.hpp"

namespace RatingService
{

namespace Tests
{

struct WorkerTests : ::testing::Test
{
    MockFactory Factory;

    std::unique_ptr<RatingService::IWorker> Worker;

    void SetUp() override
    {
        Factory.MakeManager(&Factory);
        Worker = RatingService::MakeWorker(&Factory, Factory.Manager);
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

TEST_F(WorkerTests, ShouldPostToAsioServiceOnPost)
{
    ASSERT_TRUE(Factory.AsioService);
    EXPECT_CALL(*Factory.AsioService, Post(_));

    Worker->Post(TSharedRawMessageTask{Worker.get()});
}

}
}

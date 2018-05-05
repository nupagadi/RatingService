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


}
}

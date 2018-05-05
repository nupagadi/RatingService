#include <cstring>

#include "Mocks.hpp"
#include "../Manager.hpp"

namespace RatingService
{

namespace Tests
{

template <size_t N>
std::unique_ptr<uint8_t[]> MakeRawMessage(const char(& aLiteral)[N])
{
    auto result = std::make_unique<uint8_t[]>(N);
    std::memcpy(result.get(), aLiteral, N);
    return result;
}

struct ManagerTests : ::testing::Test
{
    short Port = 11111;
    size_t ThreadsCount = 4;

public:

    MockFactory Factory {Port, ThreadsCount};

    std::unique_ptr<RatingService::Manager> Manager;
    std::vector<IWorker*> Workers;

    void SetUp() override
    {
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
    EXPECT_CALL(*Factory.Service, Run());
    for (size_t i = 0; i < ThreadsCount; ++i)
    {
        EXPECT_CALL(*Factory.Workers[i], Run());
    }

    Manager->Run();
}

TEST_F(ManagerTests, ShouldPostToAppropriateWorker)
{
    EXPECT_CALL(*Factory.Workers[1], PostProxy(_));

    Manager->ProcessMessageFromNet(MakeRawMessage("90\0\0"), 4);

    EXPECT_CALL(*Factory.Workers[0], PostProxy(_));

    Manager->ProcessMessageFromNet(MakeRawMessage("\0\0\0\011"), 6);
}

}
}

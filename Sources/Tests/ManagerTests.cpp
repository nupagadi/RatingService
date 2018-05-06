#include <cstring>

#include "Mocks.hpp"

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

    std::unique_ptr<RatingService::IManager> Manager;

    void SetUp() override
    {
        Manager = RatingService::MakeManager(&Factory);
    }

    void TearDown() override
    {
    }
};

// TODO: ShouldPollService.
TEST_F(ManagerTests, ShouldRunAllOnRun)
{
    // TODO: Can't expect from another thread now. Will try this:
    // https://stackoverflow.com/questions/10767131/expecting-googlemock-calls-from-another-thread
//    InSequence s;
//    for (size_t i = 0; i < ThreadsCount; ++i)
//    {
//        EXPECT_CALL(*Factory.Workers[i], Run());
//    }
//    EXPECT_CALL(*Factory.Service, Run());

//    Manager->Run();
}

TEST_F(ManagerTests, ShouldPostToAppropriateWorker)
{
    EXPECT_CALL(*Factory.Workers[1], PostProxy(_));
    Manager->ProcessMessageFromNet(MakeRawMessage("90\0\032\r\n"), 4); // 12345

    EXPECT_CALL(*Factory.Workers[0], PostProxy(_));
    Manager->ProcessMessageFromNet(MakeRawMessage("\0\0\0\011\r\n"), 6);

    EXPECT_CALL(*Factory.Workers[3], PostProxy(_));
    Manager->ProcessMessageFromNet(MakeRawMessage("\3\0\0\011\r\n"), 6);
}

TEST_F(ManagerTests, ShouldTrimEOL)
{
    auto raw = MakeRawMessage("90\000\00032\r\n");

    // A crutch.
    TSharedRawMessage sptr(raw.get(), [](uint8_t*){});
    auto w = Factory.Workers[1];
    TSharedRawMessageTask task(w, std::move(sptr), 6);

    EXPECT_CALL(*w, PostProxy(Pointee(Truly(
        std::bind(&TSharedRawMessageTask::operator==, std::cref(task), std::placeholders::_1)))));

    Manager->ProcessMessageFromNet(std::move(raw), 8);
}

}
}

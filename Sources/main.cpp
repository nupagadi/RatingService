#include <memory>

// TODO: Remove.
#include <iostream>

#include "IFactory.hpp"
#include "IService.hpp"

struct SimpleManager
{
    SimpleManager(
        std::shared_ptr<RatingService::IService> aService,
        size_t /*aThreadsCount*/)
        : mService(std::move(aService))
    {
    }

    void Run()
    {
        // TODO: Use Poll instead.
        mService->Run();
    }

private:

    std::shared_ptr<RatingService::IService> mService;
};

// TODO: Arguments.
int main(int argc, char** argv)
{
    if (argc != 3)
    {
        std::cerr << "Should supply a port and threads count" << std::endl;
        return EXIT_FAILURE;
    }

    auto factory = RatingService::MakeFactory();

    SimpleManager manager(factory->MakeSharedService(nullptr, std::atoi(argv[1])), std::atoi(argv[2]));

    manager.Run();

    return 0;
}


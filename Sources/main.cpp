#include <memory>

// TODO: Remove.
#include <iostream>

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
int main()
{
//    SimpleManager manager(RatingService::MakeSharedService(21345));

//    manager.Run();

    return 0;
}


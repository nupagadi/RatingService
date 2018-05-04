
// TODO: Remove.
#include <iostream>

#include "IFactory.hpp"
#include "IService.hpp"

// TODO: Create directories.

int main(int argc, char** argv)
{
    if (argc != 3)
    {
        std::cerr << "Should supply a port and threads count" << std::endl;
        return EXIT_FAILURE;
    }

    auto factory = RatingService::MakeFactory(std::atoi(argv[1]), std::atoi(argv[2]));

    auto manager = factory->MakeManager(factory.get());

    manager->Run();

    return EXIT_SUCCESS;
}


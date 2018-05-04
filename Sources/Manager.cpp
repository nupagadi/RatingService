#include "Manager.hpp"

namespace RatingService
{

std::unique_ptr<IService> MakeManager(std::unique_ptr<IService>&& aService, size_t aThreadsCount)
{
//    auto asioService = MakeAsioService();
//    auto asioSocket = MakeAsioSocket(asioService.get());
//    auto asioAcceptor = MakeAsioAcceptor(asioService.get(), aPort);

//    return std::make_shared<Service>(
//        std::move(asioService), std::move(asioAcceptor), std::move(asioSocket), aManager, 0);
}

}

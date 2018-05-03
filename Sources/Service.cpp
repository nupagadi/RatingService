#include <iostream>

#include "IAsio.hpp"
#include "Service.hpp"

namespace RatingService
{

std::shared_ptr<IService> MakeSharedService(short aPort)
{
    auto asioService = MakeAsioService();
    auto asioSocket = MakeAsioSocket(asioService.get());
    auto asioAcceptor = MakeAsioAcceptor(asioService.get(), aPort);

    // TODO: Remove port?
    return std::make_shared<Service>(std::move(asioService), std::move(asioAcceptor), std::move(asioSocket), 0);
}

}

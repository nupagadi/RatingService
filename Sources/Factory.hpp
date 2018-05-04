#pragma once

#include "IFactory.hpp"
#include "Service.hpp"
#include "Asio.hpp"

namespace RatingService
{

struct Factory : IFactory
{
    std::shared_ptr<IService> MakeSharedService(IManager* aManager, short aPort) override
    {
        auto asioService = MakeAsioService();
        auto asioSocket = MakeAsioSocket(asioService.get());
        auto asioAcceptor = MakeAsioAcceptor(asioService.get(), aPort);

        // TODO: Remove port?
        return std::make_shared<Service>(
            this, std::move(asioService), std::move(asioAcceptor), std::move(asioSocket), aManager, 0);
    }

    std::unique_ptr<IAsioService> MakeAsioService() override
    {
        return std::make_unique<AsioService>();
    }

    std::unique_ptr<IAsioSocket> MakeAsioSocket(IAsioService* aAsioService) override
    {
        return std::make_unique<AsioSocket>(dynamic_cast<AsioService*>(aAsioService));
    }

    std::unique_ptr<IAsioAcceptor> MakeAsioAcceptor(IAsioService* aAsioService, short aPort) override
    {
        return std::make_unique<AsioAcceptor>(dynamic_cast<AsioService*>(aAsioService), aPort);
    }

};

}

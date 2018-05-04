#pragma once

#include "IManager.hpp"
#include "IService.hpp"
#include "IAsio.hpp"

namespace RatingService
{

struct IFactory
{
    virtual ~IFactory() = default;

    virtual std::shared_ptr<IService> MakeSharedService(IManager *aManager, short aPort) = 0;

    virtual std::unique_ptr<IAsioService> MakeAsioService() = 0;

    virtual std::unique_ptr<IAsioSocket> MakeAsioSocket(IAsioService* aAsioService) = 0;

    virtual std::unique_ptr<IAsioAcceptor> MakeAsioAcceptor(IAsioService* aAsioService, short aPort) = 0;
};

std::unique_ptr<IFactory> MakeFactory();

}

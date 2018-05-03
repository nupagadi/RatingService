#include "IService.hpp"
#include "IAsio.hpp"

namespace RatingService
{

struct Service : std::enable_shared_from_this<Service>, IService
{
    Service(
        std::unique_ptr<IAsioService>&& aService,
        std::unique_ptr<IAsioAcceptor>&& aAcceptor,
        std::unique_ptr<IAsioSocket>&& aSocket,
        short aPort)
        : mService(std::move(aService))
        , mAcceptor(std::move(aAcceptor))
        , mSocket(std::move(aSocket))
        , mPort(aPort)
    {
    }

    void Run()
    {
        Accept();
    }

private:

    void Accept()
    {
        mAcceptor->Accept(mSocket.get(), std::bind(&Service::Receive, this, std::placeholders::_1));
    }

public:

    void Receive(boost::system::error_code aErrorCode)
    {

    }

    void Send();

private:

    std::unique_ptr<IAsioService> mService;
    std::unique_ptr<IAsioAcceptor> mAcceptor;
    std::unique_ptr<IAsioSocket> mSocket;

    short mPort {};
};

}

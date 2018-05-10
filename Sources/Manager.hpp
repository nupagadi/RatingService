#pragma once

#include <iostream>
#include <thread>
#include <mutex>

#include "IFactory.hpp"
#include "IData.hpp"
#include "IManager.hpp"
#include "IService.hpp"

namespace RatingService
{

struct Manager : IManager
{
    Manager(IFactory* aFactory)
        : mService(aFactory->MakeSharedService(this))
        , mData(aFactory->MakeData())
        , mWorkers(aFactory->MakeWorkers(aFactory, this, mData.get()))
        , mMutexes(mWorkers.size())
    {
    }

    void Run() override
    {
        std::vector<std::thread> threads;
        for (auto& e : mWorkers)
        {
            threads.emplace_back([&e]{ e->Run(); });
        }

        SetupTimers();

        mService->Run();

        for (auto& e : threads)
        {
            e.join();
        }
    }

    void ProcessMessageFromNet(std::unique_ptr<uint8_t[]> aMessage, size_t aLength) override
    {
        auto& w = mWorkers[WorkerId(aMessage.get())];

        std::cout << "WorkerId:" << WorkerId(aMessage.get()) << std::endl;

        if (aMessage[aLength - 2] == '\r' && aMessage[aLength - 1] == '\n')
        {
            aLength -= 2;
        }
        // TODO: Pass shared_ptr from Worker.
        w->Post(TSharedRawMessageTask{
            w.get(),
            TSharedRawMessage(aMessage.release(), std::default_delete<uint8_t[]>()),
            aLength});
    }

    void ProcessNotify(size_t aTimerId, size_t aNowSec) override
    {
        std::cout << "ProcessNotify: " << aTimerId << std::endl;

        if (aTimerId == mTradingPeriodTimerId)
        {
            assert((aNowSec - SomeMondaySec) % TradingPeriodSec == 0);
            for (auto& w : mWorkers)
            {
                w->Post(TDropDataTask{w.get(), std::chrono::seconds{aNowSec}});
            }
            mTradingPeriodStart = aNowSec;
        }
        else if (aTimerId == mSendingTimerId)
        {
            auto workerId = NearestWorkerId(aNowSec, mWorkers.size());

            auto promise = std::make_shared<std::promise<void>>();
            std::shared_future<void> future(promise->get_future());
            for (size_t i = 0; i < mWorkers.size(); ++i)
            {
                auto& w = mWorkers[i];
                if (i != workerId)
                {
                    w->Post(TWaitTask{w.get(), future});
                }
                else
                {
                    w->Post(TSendInfoTask{w.get(), std::move(promise)});
                }
            }
        }
    }

    void Lock(size_t aId) override
    {
        assert(aId < mMutexes.size());
        mMutexes[aId].lock();
    }

    void Unlock(size_t aId) override
    {
        assert(aId < mMutexes.size());
        mMutexes[aId].unlock();
    }

    IWorker* GetWorker(size_t aWorkerId) override
    {
        assert(aWorkerId < mWorkers.size());
        return mWorkers[aWorkerId].get();
    }

    void Post(TManagerSharedRawMessageTask aMessage) override
    {
        mService->GetAsioService()->Post(std::move(aMessage));
    }

    void Process(TSharedRawMessage aTask, size_t aLength) override
    {
        mService->Send(std::move(aTask), aLength);
    }

private:

    void SetupTimers()
    {
        auto now = std::chrono::duration_cast<std::chrono::seconds>(mClock.now().time_since_epoch()).count();
        assert(now > SomeMondaySec);

        auto tp = now;
        tp -= SomeMondaySec;
        tp /= TradingPeriodSec;
        tp *= TradingPeriodSec;
        tp += SomeMondaySec;
        mTradingPeriodStart = tp;

        tp = now;
        tp /= SendingIntervalSec;
        tp *= SendingIntervalSec;
        tp += SendingIntervalSec;

        std::cout << "Timers: " << mTradingPeriodStart + TradingPeriodSec << ", " << SendingIntervalSec << std::endl;

        mTradingPeriodTimerId = mService->Notify(mTradingPeriodStart + TradingPeriodSec, TradingPeriodSec);
        mSendingTimerId = mService->Notify(tp, SendingIntervalSec);

        ProcessNotify(mTradingPeriodTimerId, mTradingPeriodStart);
    }

    size_t WorkerId(const uint8_t* aMessage)
    {
        return *reinterpret_cast<const uint32_t*>(aMessage) % mWorkers.size();
    }

private:

    size_t mTradingPeriodStart {};
    std::chrono::system_clock mClock;
    size_t mTradingPeriodTimerId = -1, mSendingTimerId = -1;

    std::shared_ptr<IService> mService;
    std::unique_ptr<IData> mData;
    const std::vector<std::unique_ptr<IWorker>> mWorkers;

    std::vector<std::mutex> mMutexes;
};

std::unique_ptr<IManager> MakeManager(IFactory* aFactory)
{
    return std::make_unique<Manager>(aFactory);
}

}

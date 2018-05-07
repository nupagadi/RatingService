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
    // TODO: Move to some config?
    static const constexpr size_t SendingIntervalSec = 3;
    static const constexpr size_t TradingPeriodSec = 7 * 24 * 60 * 60;
    static const constexpr size_t SomeMondaySec = 1525046400;

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

    void ProcessNotify(size_t aTimerId) override
    {
        std::cout << "ProcessNotify: " << aTimerId << std::endl;

        if (aTimerId == mTradingPeriodTimerId)
        {
            for (auto& w : mWorkers)
            {
                w->Post(TDropDataTask{w.get(), TaskType::DropData});
            }
        }
    }

    void Lock(size_t aId) override
    {
        mMutexes[aId].lock();
    }

    void Unlock(size_t aId) override
    {
        mMutexes[aId].unlock();
    }

private:

//    void DropData()
//    {
//        std::promise<void> promise;
//        std::shared_future<void> future(promise.get_future());

//        for (auto& w : mWorkers)
//        {
//            w->Post(TWaitTask{w.get(), future});
//        }

//        for (auto& m : mMutexes)
//        {
//            m.lock();
//        }

//        for (auto& w : mWorkers)
//        {
//            mData->Drop(w.Id);
//        }
//    }

    void SetupTimers()
    {
        auto now = std::chrono::duration_cast<std::chrono::seconds>(mClock.now().time_since_epoch()).count();
        assert(now > SomeMondaySec);

        auto tp = now;
        tp -= SomeMondaySec;
        tp /= TradingPeriodSec;
        tp *= TradingPeriodSec;
        tp += SomeMondaySec;
        mPreviousMonday = tp;

        tp = now;
        tp /= SendingIntervalSec;
        tp *= SendingIntervalSec;
        tp += SendingIntervalSec;

        std::cout << "Timers: " << mPreviousMonday + TradingPeriodSec << ", " << SendingIntervalSec << std::endl;

        mTradingPeriodTimerId = mService->Notify(mPreviousMonday + TradingPeriodSec, TradingPeriodSec);
        mSendingTimerId = mService->Notify(tp, SendingIntervalSec);
    }

    size_t WorkerId(const uint8_t* aMessage)
    {
        return *reinterpret_cast<const uint32_t*>(aMessage) % mWorkers.size();
    }

private:

    size_t mPreviousMonday {};
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

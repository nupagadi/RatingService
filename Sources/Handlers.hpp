#pragma once

#include <future>

#include "Types.hpp"

namespace RatingService
{

template <typename TCallee, typename TCall>
struct Callback;

template <typename TCallee, typename R, typename ... TArgs>
struct Callback<TCallee, R (TArgs...)>
{
    using TCall = R(TCallee::*)(TArgs...);

    Callback(const std::shared_ptr<TCallee>& aCallee, TCall aCall)
        : mCallee(aCallee)
        , mCall(aCall)
    {
    }

    Callback(TCall aCall)
        : mCall(aCall)
    {
    }

    void SetCallee(std::shared_ptr<TCallee>&& aCallee)
    {
        if (mCallee != aCallee)
        {
            mCallee = std::move(aCallee);
        }
    }

    R operator()(TArgs&&... aArgs)
    {
        return (*mCallee.*mCall)(std::forward<TArgs>(aArgs)...);
    }

    bool operator ==(const Callback& aRighthand) const
    {
        return mCallee == aRighthand.mCallee && mCall == aRighthand.mCall;
    }

private:

    std::shared_ptr<TCallee> mCallee;
    TCall mCall;
};

template <typename TWorker, typename ...TArgs>
struct Task
{
    template <typename ...UArgs>
    Task(TWorker* aWorker, UArgs&&... aTask)
        : mTask(std::forward<UArgs>(aTask)...)
        , mWorker(aWorker)
    {
    }

    void operator()()
    {
        if (mWorker)
        {
            DoCall(std::index_sequence_for<TArgs...>{});
        }
        mWorker = nullptr;
    }

    bool operator==(const Task& aRight) const
    {
        return mWorker == aRight.mWorker && mTask == aRight.mTask;
    }

private:

    template <size_t ...S>
    void DoCall(std::index_sequence<S...>)
    {
        mWorker->Process(std::move(std::get<S>(mTask))...);
    }

    std::tuple<TArgs...> mTask;
    TWorker* mWorker;
};

struct IWorker;

template <typename ...TArgs>
using TWorkerTask = Task<IWorker, TArgs...>;

using TRawMessageTask = TWorkerTask<TRawMessage, size_t>;
using TSharedRawMessageTask = TWorkerTask<TSharedRawMessage, size_t>;
using TWaitTask = TWorkerTask<std::shared_future<void>>;
using TDropDataTask = TWorkerTask<std::chrono::seconds>;

enum class TConnected
{
    JustConnected,
    SendBack,
};

using TConnectedTask = TWorkerTask<TConnected, TClientId>;

}

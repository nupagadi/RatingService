#pragma once

#include <memory>

namespace RatingService
{

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

struct IWorker
{
    template <typename ...TArgs>
    using TWorkerTask = Task<IWorker, TArgs...>;
    using TRawMessage = TWorkerTask<std::unique_ptr<uint8_t[]>, size_t>;

    virtual ~IWorker() = default;

    virtual void Run() = 0;

    virtual void Post(TRawMessage) = 0;

    virtual void Process(std::unique_ptr<uint8_t[]> aTask, size_t aLength) = 0;
};

}

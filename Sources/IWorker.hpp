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
            mWorker->Process(std::move(mTask));
        }
        mWorker = nullptr;
    }

private:

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

private:

    virtual void Process(std::unique_ptr<char[]>) = 0;
};

}

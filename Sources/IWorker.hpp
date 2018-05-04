#pragma once

#include <memory>

namespace RatingService
{

template <typename T, typename TWorker>
struct Task
{
    Task(T&& aTask, TWorker* aWorker)
        : mTask(aTask)
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

    TWorker* mWorker;
    Task mTask;
};

struct IWorker
{
    template <typename T>
    using TWorkerTask = Task<T, IWorker>;

    virtual ~IWorker() = default;

    virtual void Run() = 0;

    virtual void Post(TWorkerTask<std::unique_ptr<char[]>>) = 0;

    virtual void Process(std::unique_ptr<char[]>) = 0;
};

}

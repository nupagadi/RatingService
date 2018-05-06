#pragma once

#include "IData.hpp"

namespace RatingService
{

struct Data : IData
{
    void Register(TClientId /*aClientId*/, TSharedRawMessage /*aName*/) override
    {

    }

    void Rename(TClientId /*aClientId*/, TSharedRawMessage /*aName*/) override
    {

    }

    void AddDeal(TClientId /*aClientId*/, TSharedRawMessage /*aName*/) override
    {

    }

    std::vector<DataEntry> Copy() const override
    {
        return {};
    }

};

std::unique_ptr<IData> MakeData()
{
    return std::make_unique<Data>();
}

}

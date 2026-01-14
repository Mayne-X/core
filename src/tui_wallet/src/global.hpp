#pragma once
#include "data/interface.hpp"
#include <memory>
namespace global {
struct Globals {
    DataRetrievalContext dataRetrievalContext;
    DataInterface dataInterface;
    Globals(DataRetrievalContext init)
        : dataRetrievalContext(std::move(init))
    {
    }
};

inline std::unique_ptr<Globals> ptr;
inline void init(DataRetrievalContext g)
{
    ptr = std::make_unique<Globals>(std::move(g));
}
inline auto& globals() { return *ptr; }
inline auto& data_retrieval_context() { return ptr->dataRetrievalContext; }
inline auto& endpoint() { return data_retrieval_context().endpoint; }
}

namespace {
inline struct Services {
    Services()
    {
        ECC_Start();
    }
    ~Services()
    {
        ECC_Stop();
    }
} services;
}

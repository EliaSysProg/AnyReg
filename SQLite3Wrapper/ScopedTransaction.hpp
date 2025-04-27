#pragma once

#include "DatabaseConnection.hpp"

namespace sql
{
    class ScopedTransaction
    {
    public:
        ScopedTransaction(const DatabaseConnection& db);
        ~ScopedTransaction();

        ScopedTransaction(const ScopedTransaction& other) = delete;
        ScopedTransaction(ScopedTransaction&& other) noexcept = delete;
        ScopedTransaction& operator=(const ScopedTransaction& other) = delete;
        ScopedTransaction& operator=(ScopedTransaction&& other) noexcept = delete;

    private:
        const DatabaseConnection* _db;
    };
}

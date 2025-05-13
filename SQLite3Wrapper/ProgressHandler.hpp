#pragma once

#include "DatabaseConnection.hpp"

#include <functional>

namespace sql
{
    class ProgressHandler final
    {
    public:
        ProgressHandler(const DatabaseConnection& db, std::function<bool()> callback, int instructions_interval = 1000);
        ~ProgressHandler();

        ProgressHandler(const ProgressHandler& other) = delete;
        ProgressHandler(ProgressHandler&& other) noexcept = delete;
        ProgressHandler& operator=(const ProgressHandler& other) = delete;
        ProgressHandler& operator=(ProgressHandler&& other) noexcept = delete;

    private:
        const DatabaseConnection* _db;
        std::function<bool()> _callback;
    };
}

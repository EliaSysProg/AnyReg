#include "ScopedTransaction.hpp"

namespace sql
{
    ScopedTransaction::ScopedTransaction(const DatabaseConnection& db)
        : _db(&db)
    {
        _db->execute("BEGIN TRANSACTION;");
    }

    ScopedTransaction::~ScopedTransaction()
    {
        try
        {
            _db->execute("END;");
        }
        catch (...)
        {}
    }
}

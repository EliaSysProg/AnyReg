#include "Pch.hpp"

#include "AnyRegDbConnection.hpp"
#include "Registry.hpp"
#include "SQLiteWrapper/DatabaseConnection.hpp"

#include <iostream>
#include <exception>


int main()
{
    try
    {
        AnyRegDbConnection db(R"(C:\Windows\Temp\AnyReg.db)", SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE);
        for (const auto hive : {HKEY_LOCAL_MACHINE, HKEY_CURRENT_USER})
            db.index(hive);
        return EXIT_SUCCESS;
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    catch (...)
    {
        std::cerr << "An unknown exception occured!\n";
    }

    return EXIT_FAILURE;
}

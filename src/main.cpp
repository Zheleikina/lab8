#include <filesystem>
#include <iostream>
#include <stdexcept>

#include "Demos.h"
#include "RawSqliteDemo.h"
#include "TestData.h"

int main()
{
    try
    {
        // Очищаємо бази даних попередніх запусків, щоб ID і вибірки були чистими
        std::filesystem::remove("image_pipeline_raw.db");
        std::filesystem::remove("image_pipeline_orm.db");
        std::filesystem::remove("image_pipeline_queries.db");
        std::filesystem::remove("image_pipeline_no_transaction.db");

        runRawSqliteDemo();

        runOrmDemo();
        runRelationshipDemo();
        runRepositoryDemo();

        prepareTestData();
        runQueryDemo();
        runAggregateDemo();
        runJoinDemo();

        runTransactionDemo();
        runRollbackDemo();
        runWithoutTransactionDemo();
    }
    catch (const std::exception& ex)
    {
        std::cerr << "Application error: " << ex.what() << '\n';
        return 1;
    }

    return 0;
}
#include <filesystem>
#include <iostream>
#include <stdexcept>

#include <sqlite3.h>
#include <sqlite_orm/sqlite_orm.h>

#include "Database.h"
#include "IImageBatchRepository.h"
#include "RawSqliteDemo.h"
#include "SQLiteImageBatchRepository.h"
#include "TestData.h"

namespace
{

// 1. Базові операції ORM (insert / get_optional) на окремій БД.
void runOrmDemo()
{
    auto storage = createStorage("image_pipeline_orm.db");
    storage.sync_schema();

    ImageBatch batch{
        0,
        "ORM demo batch",
        "/data/incoming/orm_demo",
        "queued",
        "2026-11-02"
    };

    int id = storage.insert(batch);

    std::cout << "\n=== ORM DEMONSTRATION ===\n";
    std::cout << "Inserted ImageBatch ID: " << id << '\n';

    auto found = storage.get_optional<ImageBatch>(id);
    if (found)
    {
        std::cout << "Found: " << found->name
                   << " | " << found->sourcePath
                   << " | " << found->status << '\n';
    }
    else
    {
        std::cout << "ImageBatch not found\n";
    }
}

// 2. Зв'язок 1:N між ImageBatch і ProcessingTask, перевірка зовнішнього ключа.
void runRelationshipDemo()
{
    using namespace sqlite_orm;

    auto storage = createStorage("image_pipeline_orm.db");

    storage.on_open = [](sqlite3* db)
    {
        sqlite3_exec(db, "PRAGMA foreign_keys = ON;", nullptr, nullptr, nullptr);
    };
    storage.open_forever();
    storage.sync_schema();

    std::cout << "\n=== RELATIONSHIP DEMONSTRATION (1:N) ===\n";

    ImageBatch batch{
        0,
        "Drone survey batch",
        "/data/incoming/drone_survey",
        "processing",
        "2026-11-03"
    };
    int batchId = storage.insert(batch);
    std::cout << "ImageBatch ID: " << batchId << '\n';

    ProcessingTask t1{0, batchId, "resize",   "completed", 140.0, "2026-11-03"};
    ProcessingTask t2{0, batchId, "filter",   "completed", 260.0, "2026-11-03"};

    int taskId1 = storage.insert(t1);
    int taskId2 = storage.insert(t2);
    std::cout << "ProcessingTask records inserted: " << taskId1 << ", " << taskId2 << '\n';

    auto batchTasks = storage.get_all<ProcessingTask>(
        where(c(&ProcessingTask::batchId) == batchId)
    );

    std::cout << "\nTasks for batch " << batchId << ":\n";
    for (const auto& task : batchTasks)
    {
        std::cout
            << "ID: " << task.id
            << ", Operation: " << task.operationType
            << ", Duration: " << task.durationMs
            << '\n';
    }

    // Перевірка посилальної цілісності: некоректний batchId повинен бути відхилений.
    try
    {
        ProcessingTask invalidTask{0, -1, "resize", "pending", 50.0, "2026-11-04"};
        storage.insert(invalidTask);
        std::cout << "Unexpected insertion success\n";
    }
    catch (const std::exception& ex)
    {
        std::cout << "\nForeign key constraint test:\n";
        std::cout << "Insertion rejected: " << ex.what() << '\n';
    }
}

// 3. CRUD через патерн Repository (IImageBatchRepository / SQLiteImageBatchRepository).
void runRepositoryDemo()
{
    SQLiteImageBatchRepository sqliteRepository("image_pipeline_orm.db");
    IImageBatchRepository& repository = sqliteRepository;

    std::cout << "\n=== REPOSITORY DEMONSTRATION ===\n";

    // CREATE
    ImageBatch batch{
        0,
        "Repository demo batch",
        "/data/incoming/repo_demo",
        "queued",
        "2026-11-05"
    };
    int id = repository.add(batch);
    std::cout << "\nCREATE\nInserted ImageBatch ID: " << id << '\n';

    // READ
    std::cout << "\nREAD\n";
    auto found = repository.findById(id);
    if (found)
    {
        std::cout << "ID: " << found->id << '\n';
        std::cout << "Name: " << found->name << '\n';
        std::cout << "Status: " << found->status << '\n';
    }

    // UPDATE
    std::cout << "\nUPDATE\n";
    if (found)
    {
        found->status = "processing";
        bool updated = repository.update(*found);
        std::cout << "Updated: " << std::boolalpha << updated << '\n';

        auto afterUpdate = repository.findById(id);
        if (afterUpdate)
        {
            std::cout << "New status: " << afterUpdate->status << '\n';
        }
    }

    // READ ALL
    std::cout << "\nREAD ALL\n";
    for (const auto& item : repository.findAll())
    {
        std::cout
            << item.id << " | "
            << item.name << " | "
            << item.status << '\n';
    }

    // DELETE
    std::cout << "\nDELETE\n";
    bool removed = repository.remove(id);
    std::cout << "Removed: " << std::boolalpha << removed << '\n';

    auto afterDelete = repository.findById(id);
    std::cout << "Record exists after deletion: "
               << std::boolalpha << afterDelete.has_value() << '\n';
}

// 4. Фільтрація, сортування та обмеження кількості результатів.
void runQueryDemo()
{
    using namespace sqlite_orm;

    auto storage = createStorage("image_pipeline_queries.db");
    storage.sync_schema();

    std::cout << "\n=== ORM QUERY DEMONSTRATION ===\n";

    std::cout << "\n1. Completed image batches:\n";
    auto completedBatches = storage.get_all<ImageBatch>(
        where(c(&ImageBatch::status) == "completed")
    );
    for (const auto& item : completedBatches)
    {
        std::cout << item.id << " | " << item.name << " | " << item.status << '\n';
    }

    std::cout << "\n2. Image batches sorted by name:\n";
    auto sortedBatches = storage.get_all<ImageBatch>(
        order_by(&ImageBatch::name).asc()
    );
    for (const auto& item : sortedBatches)
    {
        std::cout << item.id << " | " << item.name << '\n';
    }

    std::cout << "\n3. Top 3 longest processing tasks:\n";
    auto longestTasks = storage.get_all<ProcessingTask>(
        order_by(&ProcessingTask::durationMs).desc(),
        limit(3)
    );
    for (const auto& task : longestTasks)
    {
        std::cout << task.id << " | " << task.operationType << " | " << task.durationMs << '\n';
    }

    std::cout << "\n4. Resize tasks sorted by date:\n";
    auto resizeTasks = storage.get_all<ProcessingTask>(
        where(c(&ProcessingTask::operationType) == "resize"),
        order_by(&ProcessingTask::processedDate).asc()
    );
    for (const auto& task : resizeTasks)
    {
        std::cout << task.id << " | " << task.processedDate << " | " << task.durationMs << '\n';
    }
}

// 5. Агрегатні запити: COUNT, SUM, AVG, GROUP BY.
void runAggregateDemo()
{
    using namespace sqlite_orm;

    auto storage = createStorage("image_pipeline_queries.db");
    storage.sync_schema();

    std::cout << "\n=== AGGREGATE QUERY DEMONSTRATION ===\n";

    auto taskCount = storage.count<ProcessingTask>();
    std::cout << "\n1. ProcessingTask count: " << taskCount << '\n';

    auto totalDuration = storage.sum(&ProcessingTask::durationMs);
    std::cout << "\n2. Total duration (ms): " << (totalDuration ? *totalDuration : 0.0) << '\n';

    auto averageDuration = storage.avg(&ProcessingTask::durationMs);
    std::cout << "\n3. Average duration (ms): " << averageDuration << '\n';

    std::cout << "\n4. Total duration by batch:\n";
    auto durationByBatch = storage.select(
        columns(
            &ProcessingTask::batchId,
            sum(&ProcessingTask::durationMs)
        ),
        group_by(&ProcessingTask::batchId)
    );
    for (const auto& [batchId, totalMs] : durationByBatch)
    {
        std::cout << "Batch ID: " << batchId
                   << " | Total duration: " << (totalMs ? *totalMs : 0.0) << '\n';
    }
}

// 6. Об'єднання таблиць ImageBatch і ProcessingTask (INNER JOIN).
void runJoinDemo()
{
    using namespace sqlite_orm;

    auto storage = createStorage("image_pipeline_queries.db");
    storage.sync_schema();

    std::cout << "\n=== JOIN QUERY DEMONSTRATION ===\n";

    std::cout << "\n1. Batch processing history:\n";
    auto records = storage.select(
        columns(
            &ImageBatch::name,
            &ProcessingTask::operationType,
            &ProcessingTask::status,
            &ProcessingTask::durationMs
        ),
        inner_join<ProcessingTask>(
            on(c(&ImageBatch::id) == &ProcessingTask::batchId)
        ),
        order_by(&ProcessingTask::id)
    );
    for (const auto& [name, operationType, status, durationMs] : records)
    {
        std::cout << name << " | " << operationType << " | " << status << " | " << durationMs << '\n';
    }

    std::cout << "\n2. Tasks longer than 150 ms:\n";
    auto longTasks = storage.select(
        columns(
            &ImageBatch::name,
            &ProcessingTask::operationType,
            &ProcessingTask::durationMs
        ),
        inner_join<ProcessingTask>(
            on(c(&ImageBatch::id) == &ProcessingTask::batchId)
        ),
        where(c(&ProcessingTask::durationMs) > 150.0),
        order_by(&ProcessingTask::durationMs).desc()
    );
    for (const auto& [name, operationType, durationMs] : longTasks)
    {
        std::cout << name << " | " << operationType << " | " << durationMs << '\n';
    }
}

// 7. Успішна транзакція: реєстрація нового пакету разом із початковим завданням.
void runTransactionDemo()
{
    auto storage = createStorage("image_pipeline_queries.db");

    storage.on_open = [](sqlite3* db)
    {
        if (sqlite3_exec(db, "PRAGMA foreign_keys = ON;", nullptr, nullptr, nullptr) != SQLITE_OK)
        {
            throw std::runtime_error("Cannot enable foreign keys");
        }
    };
    storage.sync_schema();

    std::cout << "\n=== TRANSACTION DEMONSTRATION (COMMIT) ===\n";

    auto batchesBefore = storage.count<ImageBatch>();
    auto tasksBefore = storage.count<ProcessingTask>();
    std::cout << "\nBefore transaction:\n";
    std::cout << "ImageBatch: " << batchesBefore << '\n';
    std::cout << "ProcessingTask: " << tasksBefore << '\n';

    bool committed = storage.transaction([&]()
    {
        ImageBatch batch{
            0,
            "Archive restoration",
            "/data/incoming/archive_restore",
            "processing",
            "2026-11-10"
        };
        int batchId = storage.insert(batch);

        ProcessingTask task{
            0,
            batchId,
            "resize",
            "pending",
            0.0,
            "2026-11-10"
        };
        storage.insert(task);

        std::cout << "\nRecords inserted inside transaction\n";
        std::cout << "ImageBatch ID: " << batchId << '\n';

        return true; // COMMIT
    });

    std::cout << "\nTransaction committed: " << std::boolalpha << committed << '\n';
    std::cout << "\nAfter transaction:\n";
    std::cout << "ImageBatch: " << storage.count<ImageBatch>() << '\n';
    std::cout << "ProcessingTask: " << storage.count<ProcessingTask>() << '\n';
}

// 8. Відкат транзакції (ROLLBACK) через порушення зовнішнього ключа.
void runRollbackDemo()
{
    using namespace sqlite_orm;

    auto storage = createStorage("image_pipeline_queries.db");

    storage.on_open = [](sqlite3* db)
    {
        if (sqlite3_exec(db, "PRAGMA foreign_keys = ON;", nullptr, nullptr, nullptr) != SQLITE_OK)
        {
            throw std::runtime_error("Cannot enable foreign keys");
        }
    };
    storage.sync_schema();

    std::cout << "\n=== ROLLBACK DEMONSTRATION ===\n";

    auto batchesBefore = storage.count<ImageBatch>();
    auto tasksBefore = storage.count<ProcessingTask>();
    std::cout << "\nBefore transaction:\n";
    std::cout << "ImageBatch: " << batchesBefore << '\n';
    std::cout << "ProcessingTask: " << tasksBefore << '\n';

    try
    {
        storage.transaction([&]()
        {
            ImageBatch batch{
                0,
                "Corrupted batch",
                "/data/incoming/corrupted",
                "processing",
                "2026-11-11"
            };
            int batchId = storage.insert(batch);
            std::cout << "\nImageBatch inserted, ID: " << batchId << '\n';

            // Навмисно некоректний зовнішній ключ.
            ProcessingTask invalidTask{0, -1, "resize", "pending", 10.0, "2026-11-11"};
            storage.insert(invalidTask);

            return true;
        });
    }
    catch (const std::exception& ex)
    {
        std::cout << "\nTransaction failed: " << ex.what() << '\n';
        std::cout << "Changes rolled back\n";
    }

    std::cout << "\nAfter transaction:\n";
    std::cout << "ImageBatch: " << storage.count<ImageBatch>() << '\n';
    std::cout << "ProcessingTask: " << storage.count<ProcessingTask>() << '\n';

    auto corruptedBatches = storage.get_all<ImageBatch>(
        where(c(&ImageBatch::name) == "Corrupted batch")
    );
    std::cout << "\nCorrupted batch exists: "
               << std::boolalpha << !corruptedBatches.empty() << '\n';
}

// 9. Ті самі дві пов'язані операції без явної транзакції — демонстрація часткового збереження.
void runWithoutTransactionDemo()
{
    const std::string databaseFile = "image_pipeline_no_transaction.db";
    std::filesystem::remove(databaseFile);

    auto storage = createStorage(databaseFile);

    storage.on_open = [](sqlite3* db)
    {
        if (sqlite3_exec(db, "PRAGMA foreign_keys = ON;", nullptr, nullptr, nullptr) != SQLITE_OK)
        {
            throw std::runtime_error("Cannot enable foreign keys");
        }
    };
    storage.sync_schema();

    std::cout << "\n=== WITHOUT TRANSACTION ===\n";

    try
    {
        ImageBatch batch{
            0,
            "Corrupted batch",
            "/data/incoming/corrupted",
            "processing",
            "2026-11-11"
        };
        int batchId = storage.insert(batch);
        std::cout << "\nImageBatch inserted, ID: " << batchId << '\n';

        ProcessingTask invalidTask{0, -1, "resize", "pending", 10.0, "2026-11-11"};
        storage.insert(invalidTask);
    }
    catch (const std::exception& ex)
    {
        std::cout << "\nOperation failed: " << ex.what() << '\n';
    }

    std::cout << "\nAfter operations:\n";
    std::cout << "ImageBatch: " << storage.count<ImageBatch>() << '\n';
    std::cout << "ProcessingTask: " << storage.count<ProcessingTask>() << '\n';
}

} // namespace

int main()
{
    try
    {
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

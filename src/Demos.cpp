#include "Demos.h"

#include <filesystem>
#include <iostream>
#include <stdexcept>

#include "BatchRegistrationService.h"
#include "Database.h"
#include "IImageBatchRepository.h"
#include "ProcessingTaskRepository.h"
#include "SQLiteImageBatchRepository.h"

void runOrmDemo()
{
    auto storage = openConfiguredStorage("image_pipeline_orm.db");

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

void runRelationshipDemo()
{
    SQLiteImageBatchRepository batchRepository("image_pipeline_orm.db");
    ProcessingTaskRepository taskRepository("image_pipeline_orm.db");

    std::cout << "\n=== RELATIONSHIP DEMONSTRATION (1:N) ===\n";

    ImageBatch batch{
        0,
        "Drone survey batch",
        "/data/incoming/drone_survey",
        "processing",
        "2026-11-03"
    };
    int batchId = batchRepository.add(batch);
    std::cout << "ImageBatch ID: " << batchId << '\n';

    ProcessingTask t1{ 0, batchId, "resize", "completed", 140.0, "2026-11-03" };
    ProcessingTask t2{ 0, batchId, "filter", "completed", 260.0, "2026-11-03" };

    int taskId1 = taskRepository.add(t1);
    int taskId2 = taskRepository.add(t2);
    std::cout << "ProcessingTask records inserted: " << taskId1 << ", " << taskId2 << '\n';

    auto batchTasks = taskRepository.findByBatchId(batchId);

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
        ProcessingTask invalidTask{ 0, -1, "resize", "pending", 50.0, "2026-11-04" };
        taskRepository.add(invalidTask);
        std::cout << "Unexpected insertion success\n";
    }
    catch (const std::exception& ex)
    {
        std::cout << "\nForeign key constraint test:\n";
        std::cout << "Insertion rejected: " << ex.what() << '\n';
    }
}

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

void runQueryDemo()
{
    SQLiteImageBatchRepository batchRepository("image_pipeline_queries.db");
    ProcessingTaskRepository taskRepository("image_pipeline_queries.db");

    std::cout << "\n=== ORM QUERY DEMONSTRATION ===\n";

    std::cout << "\n1. Completed image batches:\n";
    for (const auto& item : batchRepository.findByStatus("completed"))
    {
        std::cout << item.id << " | " << item.name << " | " << item.status << '\n';
    }

    std::cout << "\n2. Image batches sorted by name:\n";
    for (const auto& item : batchRepository.findAllSortedByName())
    {
        std::cout << item.id << " | " << item.name << '\n';
    }

    std::cout << "\n3. Top 3 longest processing tasks:\n";
    for (const auto& task : taskRepository.findTopLongest(3))
    {
        std::cout << task.id << " | " << task.operationType << " | " << task.durationMs << '\n';
    }

    std::cout << "\n4. Resize tasks sorted by date:\n";
    for (const auto& task : taskRepository.findByOperationSortedByDate("resize"))
    {
        std::cout << task.id << " | " << task.processedDate << " | " << task.durationMs << '\n';
    }
}

void runAggregateDemo()
{
    ProcessingTaskRepository taskRepository("image_pipeline_queries.db");

    std::cout << "\n=== AGGREGATE QUERY DEMONSTRATION ===\n";

    std::cout << "\n1. ProcessingTask count: " << taskRepository.countTasks() << '\n';
    std::cout << "\n2. Total duration (ms): " << taskRepository.totalDurationMs() << '\n';
    std::cout << "\n3. Average duration (ms): " << taskRepository.averageDurationMs() << '\n';

    std::cout << "\n4. Total duration by batch:\n";
    for (const auto& [batchId, totalMs] : taskRepository.totalDurationByBatch())
    {
        std::cout << "Batch ID: " << batchId
            << " | Total duration: " << totalMs << '\n';
    }
}

void runJoinDemo()
{
    ProcessingTaskRepository taskRepository("image_pipeline_queries.db");

    std::cout << "\n=== JOIN QUERY DEMONSTRATION ===\n";

    std::cout << "\n1. Batch processing history:\n";
    for (const auto& [name, operationType, status, durationMs] : taskRepository.getHistoryWithBatchName())
    {
        std::cout << name << " | " << operationType << " | " << status << " | " << durationMs << '\n';
    }

    std::cout << "\n2. Tasks longer than 150 ms:\n";
    for (const auto& [name, operationType, durationMs] : taskRepository.getTasksLongerThan(150.0))
    {
        std::cout << name << " | " << operationType << " | " << durationMs << '\n';
    }
}

void runTransactionDemo()
{
    SQLiteImageBatchRepository batchRepository("image_pipeline_queries.db");
    ProcessingTaskRepository taskRepository("image_pipeline_queries.db");
    BatchRegistrationService registrationService("image_pipeline_queries.db");

    std::cout << "\n=== TRANSACTION DEMONSTRATION (COMMIT) ===\n";

    std::cout << "\nBefore transaction:\n";
    std::cout << "ImageBatch: " << batchRepository.findAll().size() << '\n';
    std::cout << "ProcessingTask: " << taskRepository.countTasks() << '\n';

    ImageBatch batch{
        0,
        "Archive restoration",
        "/data/incoming/archive_restore",
        "processing",
        "2026-11-10"
    };
    ProcessingTask task{
        0,
        0,
        "resize",
        "pending",
        0.0,
        "2026-11-10"
    };

    RegistrationResult result = registrationService.registerBatchWithInitialTask(batch, task);

    std::cout << "\nRecords inserted inside transaction\n";
    std::cout << "ImageBatch ID: " << result.batchId << '\n';
    std::cout << "\nTransaction committed: " << std::boolalpha << result.committed << '\n';
    std::cout << "\nAfter transaction:\n";
    std::cout << "ImageBatch: " << batchRepository.findAll().size() << '\n';
    std::cout << "ProcessingTask: " << taskRepository.countTasks() << '\n';
}

void runRollbackDemo()
{
    SQLiteImageBatchRepository batchRepository("image_pipeline_queries.db");
    ProcessingTaskRepository taskRepository("image_pipeline_queries.db");
    BatchRegistrationService registrationService("image_pipeline_queries.db");

    std::cout << "\n=== ROLLBACK DEMONSTRATION ===\n";

    std::cout << "\nBefore transaction:\n";
    std::cout << "ImageBatch: " << batchRepository.findAll().size() << '\n';
    std::cout << "ProcessingTask: " << taskRepository.countTasks() << '\n';

    try
    {
        ImageBatch batch{
            0,
            "Corrupted batch",
            "/data/incoming/corrupted",
            "processing",
            "2026-11-11"
        };
        ProcessingTask invalidTask{ 0, 0, "resize", "pending", 10.0, "2026-11-11" };

        registrationService.registerBatchWithInitialTask(batch, invalidTask, -1);
    }
    catch (const std::exception& ex)
    {
        std::cout << "\nTransaction failed: " << ex.what() << '\n';
        std::cout << "Changes rolled back\n";
    }

    std::cout << "\nAfter transaction:\n";
    std::cout << "ImageBatch: " << batchRepository.findAll().size() << '\n';
    std::cout << "ProcessingTask: " << taskRepository.countTasks() << '\n';

    bool corruptedExists = false;
    for (const auto& item : batchRepository.findAll())
    {
        if (item.name == "Corrupted batch")
        {
            corruptedExists = true;
            break;
        }
    }
    std::cout << "\nCorrupted batch exists: " << std::boolalpha << corruptedExists << '\n';
}

void runWithoutTransactionDemo()
{
    const std::string databaseFile = "image_pipeline_no_transaction.db";
    std::filesystem::remove(databaseFile);

    auto storage = openConfiguredStorage(databaseFile);

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

        ProcessingTask invalidTask{ 0, -1, "resize", "pending", 10.0, "2026-11-11" };
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
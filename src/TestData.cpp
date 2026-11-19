#include "TestData.h"

#include <filesystem>
#include <iostream>
#include <vector>

#include "Database.h"

void prepareTestData()
{
    const std::string databaseFile = "image_pipeline_queries.db";

    // Видаляємо лише окрему демонстраційну БД перед кожним запуском,
    // щоб забезпечити повторюваність результатів запитів.
    std::filesystem::remove(databaseFile);

    auto storage = openConfiguredStorage(databaseFile);

    std::vector<ImageBatch> batches = {
        {0, "Vacation photos",    "/data/incoming/batch_1", "completed",  "2026-09-01"},
        {0, "Product catalog",    "/data/incoming/batch_2", "processing", "2026-09-10"},
        {0, "Satellite tiles",    "/data/incoming/batch_3", "completed",  "2026-09-20"},
        {0, "User avatars",       "/data/incoming/batch_4", "failed",     "2026-10-02"},
        {0, "Document scans",     "/data/incoming/batch_5", "queued",     "2026-10-15"}
    };

    std::vector<int> batchIds;
    for (const auto& batch : batches)
    {
        batchIds.push_back(storage.insert(batch));
    }

    std::vector<ProcessingTask> tasks = {
        {0, batchIds[0], "resize",    "completed", 120.0, "2026-09-01"},
        {0, batchIds[0], "filter",    "completed", 340.5, "2026-09-01"},
        {0, batchIds[0], "compress",  "completed",  80.0, "2026-09-02"},

        {0, batchIds[1], "resize",    "completed", 150.0, "2026-09-10"},
        {0, batchIds[1], "watermark", "pending",   200.0, "2026-09-11"},
        {0, batchIds[1], "compress",  "pending",    90.0, "2026-09-11"},

        {0, batchIds[2], "resize",    "completed", 500.0, "2026-09-20"},
        {0, batchIds[2], "filter",    "completed", 610.0, "2026-09-21"},
        {0, batchIds[2], "compress",  "completed", 220.0, "2026-09-21"},

        {0, batchIds[3], "resize",    "failed",     60.0, "2026-10-02"},
        {0, batchIds[3], "filter",    "failed",     75.0, "2026-10-02"},
        {0, batchIds[3], "compress",  "failed",     40.0, "2026-10-03"},

        {0, batchIds[4], "resize",    "pending",   130.0, "2026-10-15"},
        {0, batchIds[4], "filter",    "pending",   310.0, "2026-10-15"},
        {0, batchIds[4], "watermark", "pending",   175.0, "2026-10-16"}
    };

    for (const auto& task : tasks)
    {
        storage.insert(task);
    }

    std::cout << "Test database prepared\n";
    std::cout << "ImageBatch records: " << storage.count<ImageBatch>() << '\n';
    std::cout << "ProcessingTask records: " << storage.count<ProcessingTask>() << '\n';
}
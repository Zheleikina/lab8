#pragma once

#include <stdexcept>
#include <string>

#include <sqlite3.h>
#include <sqlite_orm/sqlite_orm.h>

#include "ImageBatch.h"
#include "ProcessingTask.h"

// Створює ORM-сховище з відображенням ImageBatch (1) -> ProcessingTask (N).
inline auto createStorage(const std::string& filename)
{
    using namespace sqlite_orm;

    return make_storage(
        filename,
        make_table(
            "ImageBatch",
            make_column("id", &ImageBatch::id, primary_key().autoincrement()),
            make_column("name", &ImageBatch::name, not_null()),
            make_column("sourcePath", &ImageBatch::sourcePath, not_null()),
            make_column("status", &ImageBatch::status, not_null()),
            make_column("createdDate", &ImageBatch::createdDate, not_null())
        ),
        make_table(
            "ProcessingTask",
            make_column("id", &ProcessingTask::id, primary_key().autoincrement()),
            make_column("batchId", &ProcessingTask::batchId),
            make_column("operationType", &ProcessingTask::operationType, not_null()),
            make_column("status", &ProcessingTask::status, not_null()),
            make_column("durationMs", &ProcessingTask::durationMs),
            make_column("processedDate", &ProcessingTask::processedDate, not_null()),
            foreign_key(&ProcessingTask::batchId).references(&ImageBatch::id)
        )
    );
}

using Storage = decltype(createStorage(""));

// Відкриває сховище, вмикає контроль зовнішніх ключів і синхронізує схему БД.
inline Storage openConfiguredStorage(const std::string& filename)
{
    auto storage = createStorage(filename);
    storage.on_open = [](sqlite3* db)
        {
            if (sqlite3_exec(db, "PRAGMA foreign_keys = ON;", nullptr, nullptr, nullptr) != SQLITE_OK)
            {
                throw std::runtime_error("Cannot enable foreign key constraints");
            }
        };
    storage.open_forever();
    storage.sync_schema();
    return storage;
}
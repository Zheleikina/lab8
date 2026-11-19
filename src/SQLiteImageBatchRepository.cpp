#include "SQLiteImageBatchRepository.h"

#include <stdexcept>

#include <sqlite3.h>

SQLiteImageBatchRepository::SQLiteImageBatchRepository(
    const std::string& databaseFile
)
    : storage_(createStorage(databaseFile))
{
    storage_.on_open = [](sqlite3* db)
    {
        if (sqlite3_exec(
                db,
                "PRAGMA foreign_keys = ON;",
                nullptr,
                nullptr,
                nullptr
            ) != SQLITE_OK)
        {
            throw std::runtime_error(
                "Cannot enable foreign key constraints"
            );
        }
    };

    storage_.open_forever();
    storage_.sync_schema();
}

int SQLiteImageBatchRepository::add(const ImageBatch& batch)
{
    return storage_.insert(batch);
}

std::optional<ImageBatch> SQLiteImageBatchRepository::findById(int id)
{
    return storage_.get_optional<ImageBatch>(id);
}

std::vector<ImageBatch> SQLiteImageBatchRepository::findAll()
{
    return storage_.get_all<ImageBatch>();
}

bool SQLiteImageBatchRepository::update(const ImageBatch& batch)
{
    auto existing = storage_.get_optional<ImageBatch>(batch.id);

    if (!existing)
    {
        return false;
    }

    storage_.update(batch);
    return true;
}

bool SQLiteImageBatchRepository::remove(int id)
{
    auto existing = storage_.get_optional<ImageBatch>(id);

    if (!existing)
    {
        return false;
    }

    storage_.remove<ImageBatch>(id);
    return true;
}

Storage& SQLiteImageBatchRepository::storage()
{
    return storage_;
}

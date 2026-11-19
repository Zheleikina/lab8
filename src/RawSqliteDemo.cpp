#include "RawSqliteDemo.h"

#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>

#include <sqlite3.h>

#include "ImageBatch.h"

namespace
{

int insertImageBatchRaw(sqlite3* db, const ImageBatch& batch)
{
    const char* sql = R"(
        INSERT INTO ImageBatch (name, sourcePath, status, createdDate)
        VALUES (?, ?, ?, ?);
    )";

    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK)
    {
        throw std::runtime_error(sqlite3_errmsg(db));
    }

    sqlite3_bind_text(stmt, 1, batch.name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, batch.sourcePath.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, batch.status.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, batch.createdDate.c_str(), -1, SQLITE_TRANSIENT);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE)
    {
        std::string error = sqlite3_errmsg(db);
        sqlite3_finalize(stmt);
        throw std::runtime_error(error);
    }

    int id = static_cast<int>(sqlite3_last_insert_rowid(db));
    sqlite3_finalize(stmt);
    return id;
}

std::optional<ImageBatch> findImageBatchByIdRaw(sqlite3* db, int id)
{
    const char* sql = R"(
        SELECT id, name, sourcePath, status, createdDate
        FROM ImageBatch
        WHERE id = ?;
    )";

    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK)
    {
        throw std::runtime_error(sqlite3_errmsg(db));
    }

    sqlite3_bind_int(stmt, 1, id);

    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW)
    {
        ImageBatch batch;
        batch.id = sqlite3_column_int(stmt, 0);
        batch.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        batch.sourcePath = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        batch.status = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        batch.createdDate = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));

        sqlite3_finalize(stmt);
        return batch;
    }

    if (rc != SQLITE_DONE)
    {
        std::string error = sqlite3_errmsg(db);
        sqlite3_finalize(stmt);
        throw std::runtime_error(error);
    }

    sqlite3_finalize(stmt);
    return std::nullopt;
}

} // namespace

void runRawSqliteDemo()
{
    sqlite3* db = nullptr;

    int rc = sqlite3_open("image_pipeline_raw.db", &db);
    if (rc != SQLITE_OK)
    {
        std::cerr << "Cannot open database: " << sqlite3_errmsg(db) << '\n';
        sqlite3_close(db);
        return;
    }

    const char* createSql = R"(
        CREATE TABLE IF NOT EXISTS ImageBatch (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT NOT NULL,
            sourcePath TEXT NOT NULL,
            status TEXT NOT NULL,
            createdDate TEXT NOT NULL
        );
    )";

    char* errorMessage = nullptr;
    rc = sqlite3_exec(db, createSql, nullptr, nullptr, &errorMessage);
    if (rc != SQLITE_OK)
    {
        std::cerr << "SQL error: " << (errorMessage ? errorMessage : "unknown") << '\n';
        sqlite3_free(errorMessage);
        sqlite3_close(db);
        return;
    }

    std::cout << "\n=== RAW SQLITE C API DEMONSTRATION ===\n";

    try
    {
        ImageBatch batch{
            0,
            "Raw API batch",
            "/data/incoming/batch_raw",
            "queued",
            "2026-11-01"
        };

        int id = insertImageBatchRaw(db, batch);
        std::cout << "Inserted ImageBatch ID: " << id << '\n';

        auto found = findImageBatchByIdRaw(db, id);
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
    catch (const std::exception& ex)
    {
        std::cerr << "Raw SQLite error: " << ex.what() << '\n';
    }

    sqlite3_close(db);
}

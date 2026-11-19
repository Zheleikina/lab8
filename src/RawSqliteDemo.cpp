#include "RawSqliteDemo.h"

#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>

#include <sqlite3.h>

#include "ImageBatch.h"

namespace
{

    void checkBindResult(int rc, sqlite3* db, sqlite3_stmt* stmt, const char* context)
    {
        if (rc != SQLITE_OK)
        {
            std::string error = std::string(context) + ": " + sqlite3_errmsg(db);
            sqlite3_finalize(stmt);
            throw std::runtime_error(error);
        }
    }

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

        checkBindResult(
            sqlite3_bind_text(stmt, 1, batch.name.c_str(), -1, SQLITE_TRANSIENT),
            db, stmt, "Failed to bind name"
        );
        checkBindResult(
            sqlite3_bind_text(stmt, 2, batch.sourcePath.c_str(), -1, SQLITE_TRANSIENT),
            db, stmt, "Failed to bind sourcePath"
        );
        checkBindResult(
            sqlite3_bind_text(stmt, 3, batch.status.c_str(), -1, SQLITE_TRANSIENT),
            db, stmt, "Failed to bind status"
        );
        checkBindResult(
            sqlite3_bind_text(stmt, 4, batch.createdDate.c_str(), -1, SQLITE_TRANSIENT),
            db, stmt, "Failed to bind createdDate"
        );

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

        checkBindResult(
            sqlite3_bind_int(stmt, 1, id),
            db, stmt, "Failed to bind id"
        );

        rc = sqlite3_step(stmt);
        if (rc == SQLITE_ROW)
        {
            const unsigned char* colName = sqlite3_column_text(stmt, 1);
            const unsigned char* colPath = sqlite3_column_text(stmt, 2);
            const unsigned char* colStatus = sqlite3_column_text(stmt, 3);
            const unsigned char* colDate = sqlite3_column_text(stmt, 4);

            if (!colName || !colPath || !colStatus || !colDate)
            {
                sqlite3_finalize(stmt);
                throw std::runtime_error("Unexpected NULL column in ImageBatch");
            }

            ImageBatch batch;
            batch.id = sqlite3_column_int(stmt, 0);
            batch.name = reinterpret_cast<const char*>(colName);
            batch.sourcePath = reinterpret_cast<const char*>(colPath);
            batch.status = reinterpret_cast<const char*>(colStatus);
            batch.createdDate = reinterpret_cast<const char*>(colDate);

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
        std::cerr << "Cannot open database: "
            << (db ? sqlite3_errmsg(db) : "out of memory") << '\n';
        if (db)
        {
            sqlite3_close(db);
        }
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
#pragma once

#include <string>

#include <sqlite_orm/sqlite_orm.h>

#include "ImageBatch.h"
#include "ProcessingTask.h"

// Створює ORM-сховище з відображенням ImageBatch (1) -> ProcessingTask (N).
inline auto createStorage(const std::string& filename)
{
    using namespace sqlite_orm;

    return make_storage(
        filename,

        // Таблиця пакетів зображень (основна сутність)
        make_table(
            "ImageBatch",
            make_column(
                "id",
                &ImageBatch::id,
                primary_key().autoincrement()
            ),
            make_column(
                "name",
                &ImageBatch::name,
                not_null()
            ),
            make_column(
                "sourcePath",
                &ImageBatch::sourcePath,
                not_null()
            ),
            make_column(
                "status",
                &ImageBatch::status,
                not_null()
            ),
            make_column(
                "createdDate",
                &ImageBatch::createdDate,
                not_null()
            )
        ),

        // Таблиця завдань обробки (залежна сутність)
        make_table(
            "ProcessingTask",
            make_column(
                "id",
                &ProcessingTask::id,
                primary_key().autoincrement()
            ),
            make_column(
                "batchId",
                &ProcessingTask::batchId
            ),
            make_column(
                "operationType",
                &ProcessingTask::operationType,
                not_null()
            ),
            make_column(
                "status",
                &ProcessingTask::status,
                not_null()
            ),
            make_column(
                "durationMs",
                &ProcessingTask::durationMs
            ),
            make_column(
                "processedDate",
                &ProcessingTask::processedDate,
                not_null()
            ),
            foreign_key(
                &ProcessingTask::batchId
            ).references(
                &ImageBatch::id
            )
        )
    );
}

using Storage = decltype(createStorage(""));

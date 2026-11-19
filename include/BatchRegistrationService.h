#pragma once

#include <optional>
#include <string>

#include "Database.h"
#include "ImageBatch.h"
#include "ProcessingTask.h"

struct RegistrationResult
{
    int batchId = 0;
    bool committed = false;
};

// Сервісний клас для атомарної реєстрації пакета зображень разом із початковим завданням
// у межах однієї транзакції (охоплює обидві сутності: ImageBatch та ProcessingTask).
class BatchRegistrationService
{
public:
    explicit BatchRegistrationService(const std::string& databaseFile);

    // Атомарно додає ImageBatch та ProcessingTask у межах транзакції.
    // Якщо передано forcedBatchId, воно підставляється у task.batchId (для перевірки ROLLBACK).
    RegistrationResult registerBatchWithInitialTask(
        const ImageBatch& batch,
        const ProcessingTask& initialTask,
        std::optional<int> forcedBatchId = std::nullopt
    );

private:
    Storage storage_;
};
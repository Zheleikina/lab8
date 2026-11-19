#include "BatchRegistrationService.h"

BatchRegistrationService::BatchRegistrationService(const std::string& databaseFile)
    : storage_(openConfiguredStorage(databaseFile))
{}

RegistrationResult BatchRegistrationService::registerBatchWithInitialTask(
    const ImageBatch& batch,
    const ProcessingTask& initialTask,
    std::optional<int> forcedBatchId
)
{
    RegistrationResult result{};

    result.committed = storage_.transaction([&]()
        {
            ImageBatch batchCopy = batch;
            batchCopy.id = 0;
            result.batchId = storage_.insert(batchCopy);

            ProcessingTask taskCopy = initialTask;
            taskCopy.id = 0;
            taskCopy.batchId = forcedBatchId.value_or(result.batchId);
            storage_.insert(taskCopy);

            return true; // COMMIT
        });

    return result;
}
#include "ProcessingTaskRepository.h"

ProcessingTaskRepository::ProcessingTaskRepository(const std::string& databaseFile)
    : storage_(openConfiguredStorage(databaseFile))
{}

int ProcessingTaskRepository::add(const ProcessingTask& task)
{
    return storage_.insert(task);
}

std::vector<ProcessingTask> ProcessingTaskRepository::findByBatchId(int batchId)
{
    using namespace sqlite_orm;
    return storage_.get_all<ProcessingTask>(
        where(c(&ProcessingTask::batchId) == batchId)
    );
}

std::vector<ProcessingTask> ProcessingTaskRepository::findAll()
{
    return storage_.get_all<ProcessingTask>();
}

std::vector<ProcessingTask> ProcessingTaskRepository::findTopLongest(int limitCount)
{
    using namespace sqlite_orm;
    return storage_.get_all<ProcessingTask>(
        order_by(&ProcessingTask::durationMs).desc(),
        limit(limitCount)
    );
}

std::vector<ProcessingTask> ProcessingTaskRepository::findByOperationSortedByDate(
    const std::string& operationType
)
{
    using namespace sqlite_orm;
    return storage_.get_all<ProcessingTask>(
        where(c(&ProcessingTask::operationType) == operationType),
        order_by(&ProcessingTask::processedDate).asc()
    );
}

int ProcessingTaskRepository::countTasks()
{
    return static_cast<int>(storage_.count<ProcessingTask>());
}

double ProcessingTaskRepository::totalDurationMs()
{
    auto total = storage_.sum(&ProcessingTask::durationMs);
    return total ? *total : 0.0;
}

double ProcessingTaskRepository::averageDurationMs()
{
    return storage_.avg(&ProcessingTask::durationMs);
}

std::vector<std::pair<int, double>> ProcessingTaskRepository::totalDurationByBatch()
{
    using namespace sqlite_orm;

    auto rows = storage_.select(
        columns(
            &ProcessingTask::batchId,
            sum(&ProcessingTask::durationMs)
        ),
        group_by(&ProcessingTask::batchId)
    );

    std::vector<std::pair<int, double>> result;
    result.reserve(rows.size());
    for (const auto& [batchId, totalMs] : rows)
    {
        result.emplace_back(batchId, totalMs ? *totalMs : 0.0);
    }
    return result;
}

std::vector<std::tuple<std::string, std::string, std::string, double>>
ProcessingTaskRepository::getHistoryWithBatchName()
{
    using namespace sqlite_orm;

    return storage_.select(
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
}

std::vector<std::tuple<std::string, std::string, double>>
ProcessingTaskRepository::getTasksLongerThan(double minDurationMs)
{
    using namespace sqlite_orm;

    return storage_.select(
        columns(
            &ImageBatch::name,
            &ProcessingTask::operationType,
            &ProcessingTask::durationMs
        ),
        inner_join<ProcessingTask>(
            on(c(&ImageBatch::id) == &ProcessingTask::batchId)
        ),
        where(c(&ProcessingTask::durationMs) > minDurationMs),
        order_by(&ProcessingTask::durationMs).desc()
    );
}
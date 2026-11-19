#include "SQLiteImageBatchRepository.h"

SQLiteImageBatchRepository::SQLiteImageBatchRepository(
    const std::string& databaseFile
)
    : storage_(openConfiguredStorage(databaseFile))
{}

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

    // Якщо до пакета прив'язані ProcessingTask, БД згенерує виняток FOREIGN KEY constraint failed.
    storage_.remove<ImageBatch>(id);
    return true;
}

std::vector<ImageBatch> SQLiteImageBatchRepository::findByStatus(const std::string& status)
{
    using namespace sqlite_orm;
    return storage_.get_all<ImageBatch>(
        where(c(&ImageBatch::status) == status)
    );
}

std::vector<ImageBatch> SQLiteImageBatchRepository::findAllSortedByName()
{
    using namespace sqlite_orm;
    return storage_.get_all<ImageBatch>(
        order_by(&ImageBatch::name).asc()
    );
}
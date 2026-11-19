#pragma once

#include <string>
#include <vector>

#include "Database.h"
#include "IImageBatchRepository.h"

// Реалізація Repository для ImageBatch через sqlite_orm.
class SQLiteImageBatchRepository : public IImageBatchRepository
{
public:
    explicit SQLiteImageBatchRepository(const std::string& databaseFile);

    int add(const ImageBatch& batch) override;

    std::optional<ImageBatch> findById(int id) override;
    std::vector<ImageBatch> findAll() override;

    bool update(const ImageBatch& batch) override;
    bool remove(int id) override;

    // ORM-вибірки (where / order_by виконуються на рівні БД)
    std::vector<ImageBatch> findByStatus(const std::string& status);
    std::vector<ImageBatch> findAllSortedByName();

private:
    Storage storage_;
};
#pragma once

#include <optional>
#include <vector>

#include "ImageBatch.h"

// Інтерфейс доступу до даних основної сутності ImageBatch (патерн Repository).
class IImageBatchRepository
{
public:
    virtual ~IImageBatchRepository() = default;

    // Create
    virtual int add(const ImageBatch& batch) = 0;

    // Read
    virtual std::optional<ImageBatch> findById(int id) = 0;
    virtual std::vector<ImageBatch> findAll() = 0;

    // Update
    virtual bool update(const ImageBatch& batch) = 0;

    // Delete
    virtual bool remove(int id) = 0;
};

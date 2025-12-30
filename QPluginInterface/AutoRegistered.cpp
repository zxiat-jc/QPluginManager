#include "AutoRegistered.h"

#include <mutex>

RegistryHub::RegistryHub() { }
RegistryHub::~RegistryHub() { }

RegistryHub& RegistryHub::Instance()
{
    static RegistryHub instance;
    return instance;
}

void RegistryHub::add(std::string_view baseKey, const char* type_key, std::any creator)
{
    std::unique_lock lock(mtx_);
    auto& bucket = ensure_bucket_unlocked(baseKey);

    // Copy-on-write 策略：复制旧列表，添加新项，原子替换
    auto newVec = std::make_shared<std::vector<RawEntry>>(*bucket.items);
    newVec->push_back(RawEntry { type_key, std::move(creator) });

    bucket.items = newVec;
    bucket.ver.fetch_add(1, std::memory_order_release);
}

std::shared_ptr<const std::vector<RawEntry>> RegistryHub::snapshot(std::string_view baseKey) const
{
    std::shared_lock lock(mtx_);
    auto it = map_.find(std::string(baseKey)); // 注意：map key 是 string
    if (it == map_.end()) {
        static auto empty = std::make_shared<const std::vector<RawEntry>>();
        return empty;
    }
    return it->second.items;
}

std::size_t RegistryHub::version(std::string_view baseKey) const
{
    std::shared_lock lock(mtx_);
    auto it = map_.find(std::string(baseKey));
    if (it == map_.end())
        return 0;
    return it->second.ver.load(std::memory_order_acquire);
}

RegistryHub::Bucket& RegistryHub::ensure_bucket_unlocked(std::string_view baseKey)
{
    // 假设调用者已经持有写锁
    return map_[std::string(baseKey)];
}
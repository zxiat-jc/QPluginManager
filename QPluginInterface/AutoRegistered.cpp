#include <algorithm>

#include "AutoRegistered.h"

RegistryHub::RegistryHub()
{
}

RegistryHub::~RegistryHub()
{
}

RegistryHub& RegistryHub::Instance()
{
    static RegistryHub hub;
    return hub;
}

auto RegistryHub::ensure_bucket_unlocked(std::string_view baseKey) -> Bucket&
{
    auto [it, inserted] = map_.try_emplace(std::string(baseKey)); // 仅在插入时默认构造 Bucket
    return it->second;
}

void RegistryHub::add(std::string_view baseKey, const char* typeKey, std::function<void*()> creator)
{
    std::unique_lock lk(mtx_);
    Bucket& b = ensure_bucket_unlocked(baseKey);

    // 基于旧快照拷贝构建新快照，读侧只读共享_ptr，安全无锁
    std::vector<RawEntry> next = b.items ? *b.items : std::vector<RawEntry> {};
    next.push_back(RawEntry { typeKey, creator });
    b.items = std::shared_ptr<const std::vector<RawEntry>>(new std::vector<RawEntry>(std::move(next)));
    ++b.ver;
}

std::shared_ptr<const std::vector<RawEntry>> RegistryHub::snapshot(std::string_view baseKey) const
{
    std::shared_lock lock(mtx_);
    std::string key(baseKey);
    if (auto it = map_.find(key); it != map_.end() && it->second.items)
        return it->second.items;
    static const auto kEmpty = std::make_shared<const std::vector<RawEntry>>();
    return kEmpty;
}

std::size_t RegistryHub::version(std::string_view baseKey) const
{
    std::shared_lock lock(mtx_);
    std::string key(baseKey);
    if (auto it = map_.find(key); it != map_.end())
        return it->second.ver.load(std::memory_order_relaxed);
    return 0;
}

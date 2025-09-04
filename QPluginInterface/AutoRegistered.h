#pragma once

#include <QtCore/qglobal.h>

#ifndef BUILD_STATIC
#if defined(QPLUGININTERFACE_LIB)
#define QPLUGININTERFACE_EXPORT Q_DECL_EXPORT
#else
#define QPLUGININTERFACE_EXPORT Q_DECL_IMPORT
#endif
#else
#define QPLUGININTERFACE_EXPORT
#endif

#include <atomic>
#include <concepts>
#include <functional>
#include <memory>
#include <shared_mutex>
#include <string>
#include <string_view>
#include <typeinfo>
#include <unordered_map>
#include <utility>
#include <vector>

struct RawEntry {
    const char* type_key;
    std::function<void*()> creator;
};

class QPLUGININTERFACE_EXPORT RegistryHub {
private:
    RegistryHub();

public:
    ~RegistryHub();

    static RegistryHub& Instance();

    /**
     * @brief 添加
     * @param baseKey
     * @param type_key
     * @param creator
     */
    void add(std::string_view baseKey, const char* type_key, std::function<void*()> creator);

    /**
     * @brief 获取该BaseKey的注册信息快照（读侧无锁安全）
     * @param baseKey
     * @return
     */
    std::shared_ptr<const std::vector<RawEntry>> snapshot(std::string_view baseKey) const;

    /**
     * @brief 版本号（每次 add 递增）
     * @param baseKey
     * @return
     */
    std::size_t version(std::string_view baseKey) const;

private:
    using H = std::hash<std::string_view>;
    using Eq = std::equal_to<>;

    struct Bucket {
        std::shared_ptr<const std::vector<RawEntry>> items { std::make_shared<const std::vector<RawEntry>>() };
        std::atomic_size_t ver { 0 };
    };

    /**
     * @brief 需持有写锁
     * @param baseKey
     * @return
     */
    Bucket& ensure_bucket_unlocked(std::string_view baseKey);

    mutable std::shared_mutex mtx_;
    std::unordered_map<std::string, Bucket, H, Eq> map_;
};

// BaseKey 默认：typeid(Base).name()
template <typename Base>
inline const char* RegistryBaseKey()
{
    return typeid(Base).name();
}

// type_key 默认：typeid(T).name()
template <typename T>
inline const char* RegistryTypeKey()
{
    return typeid(T).name();
}

// 显式指定 BaseKey
#ifndef DECLARE_REGISTRY_BASE_KEY
#define DECLARE_REGISTRY_BASE_KEY(BASE, KEY_LITERAL) \
    template <>                                      \
    inline const char* RegistryBaseKey<BASE>() { return KEY_LITERAL; }
#endif

// 显式指定 type_key
#ifndef DECLARE_REGISTRY_TYPE_KEY
#define DECLARE_REGISTRY_TYPE_KEY(TYPE, KEY_LITERAL) \
    template <>                                      \
    inline const char* RegistryTypeKey<TYPE>() { return KEY_LITERAL; }
#endif

/**
 * @brief 跨动态库共享的静态注册表
 * @tparam Base
 */
template <typename Base>
class StaticRegistry {
public:
    using Factory = std::function<std::unique_ptr<Base>()>;

    struct Entry {
        const char* type_key;
        Factory factory;
    };

    /**
     * @brief 列举：返回本-DSO 的类型化缓存；当版本变化时自动刷新
     * @return
     */
    static const std::vector<Entry>& entries()
    {
        static std::vector<Entry> cache;
        static std::size_t cachedVersion = std::numeric_limits<std::size_t>::max();
        static const std::string baseKey = RegistryBaseKey<Base>();

        auto& hub = RegistryHub::Instance();
        const auto v = hub.version(baseKey);
        if (v != cachedVersion) {
            cache.clear();
            auto snap = hub.snapshot(baseKey);
            cache.reserve(snap->size());
            for (const auto& re : *snap) {
                // 将 RawEntry 的 creator 转成强类型工厂（包一层 unique_ptr）
                Factory f = [c = re.creator]() -> std::unique_ptr<Base> {
                    return std::unique_ptr<Base>(static_cast<Base*>(c()));
                };

                cache.push_back(Entry { re.type_key, std::move(f) });
            }
            cachedVersion = v;
        }
        return cache;
    }

    /**
     * @brief 列出所有子类键
     * @return
     */
    static std::vector<std::string_view> type_keys()
    {
        const auto& es = entries();
        std::vector<std::string_view> out;
        out.reserve(es.size());
        for (const auto& e : es)
            out.push_back(e.type_key);
        return out;
    }

    /**
     * @brief 通过字符串键创建；不存在则返回 nullptr
     * @param type_key
     * @return
     */
    static std::unique_ptr<Base> create(std::string_view type_key)
    {
        const auto& es = entries();
        for (const auto& e : es) {
            if (e.type_key == type_key)
                return e.factory();
        }
        return nullptr;
    }

    /**
     * @brief 返回某个键的工厂（可能为空）
     * @param type_key
     * @return
     */
    static Factory factoryOf(std::string_view type_key)
    {
        const auto& es = entries();
        for (const auto& e : es) {
            if (e.type_key == type_key)
                return e.factory;
        }
        return {};
    }

    /**
     * @brief 模板方式获取 Derived 的工厂（要求 Derived 派生自 Base）
     * @tparam Derived
     * @return
     */
    template <typename Derived>
        requires std::derived_from<Derived, Base>
    static std::function<std::unique_ptr<Derived>()> factoryOf()
    {
        const char* key = RegistryTypeKey<Derived>();
        const auto& es = entries();
        for (const auto& e : es) {
            if (e.type_key == key) {
                auto bf = e.factory;
                return [bf = std::move(bf)]() -> std::unique_ptr<Derived> {
                    std::unique_ptr<Base> b = bf();
                    return std::unique_ptr<Derived>(static_cast<Derived*>(b.release()));
                };
            }
        }
        return {};
    }

    /**
     * @brief 是否已注册
     * @param type_key
     * @return
     */
    static bool IsRegistered(std::string_view type_key)
    {
        const auto& es = entries();
        for (const auto& e : es)
            if (e.type_key == type_key)
                return true;
        return false;
    }

    template <typename Derived>
        requires std::derived_from<Derived, Base>
    static bool IsRegistered()
    {
        return IsRegistered(RegistryTypeKey<Derived>());
    }

    /**
     * @brief 供自动注册使用（不建议业务侧直接调用）
     * @param type_key
     * @param creator
     */
    static void AddRaw(const char* type_key, std::function<void*()> creator)
    {
        RegistryHub::Instance().add(RegistryBaseKey<Base>(), type_key, creator);
    }
};

/**
 * @brief 自动注册基类AUTO_REGISTER(Derived, Base)
 */
template <typename Base>
class AutoRegistered {
public:
    template <typename Derived>
        requires std::derived_from<Derived, Base>
    struct Registrar {
        /**
         * @brief 零捕获跳板：new Derived()，以 Base* 形式返回并擦成 void*
         * @return
         */
        static void* CreateTrampoline()
        {
            return static_cast<Base*>(new Derived());
        }

        Registrar()
        {
            StaticRegistry<Base>::AddRaw(RegistryTypeKey<Derived>(), &CreateTrampoline);
        }
    };

    template <typename Derived>
        requires std::derived_from<Derived, Base>
    static Registrar<Derived>& RegInstance()
    {
        static Registrar<Derived> r;
        return r;
    }
};

#ifndef AUTO_REGISTER
#define AUTO_REGISTER(CLASS, BASE)                               \
    namespace {                                                  \
        AutoRegistered<BASE>::Registrar<CLASS> auto_reg_##CLASS; \
    }
#endif
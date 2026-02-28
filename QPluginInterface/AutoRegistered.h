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

#include <any>
#include <atomic>
#include <concepts>
#include <functional>
#include <limits>
#include <memory>
#include <shared_mutex>
#include <string>
#include <string_view>
#include <type_traits>
#include <typeinfo>
#include <unordered_map>
#include <utility>
#include <vector>

/**
 * @brief std::any 存储创建器，创建器的签名(参数)是不确定的
 */
struct RawEntry {
    const char* type_key;
    /**
     * @brief 存储 std::function<Base*(Args...)>
     */
    std::any creator;
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
    void add(std::string_view baseKey, const char* type_key, std::any creator);

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

    mutable std::shared_mutex _mtx;
    std::unordered_map<std::string, Bucket, H, Eq> _map;
};

/**
 * @brief 从编译器函数签名中提取干净的类名
 * @tparam T
 * @return
 */
template <typename T>
inline const char* ExtractClassName()
{
    static std::string cleanName;
    if (cleanName.empty()) {
#if defined(_MSC_VER)
        // MSVC: __FUNCSIG__ 格式类似 "const char *__cdecl ExtractClassName<class MyNamespace::MyClass>(void)"
        constexpr std::string_view sig = __FUNCSIG__;
        constexpr std::string_view prefix = "ExtractClassName<";
        constexpr std::string_view suffix = ">(void)";

        auto start = sig.find(prefix);
        if (start != std::string_view::npos) {
            start += prefix.length();
            // 跳过 "class ", "struct ", "enum " 等关键字
            while (start < sig.length() && sig[start] == ' ')
                ++start;
            if (sig.substr(start, 6) == "class ")
                start += 6;
            else if (sig.substr(start, 7) == "struct ")
                start += 7;
            else if (sig.substr(start, 5) == "enum ")
                start += 5;

            auto end = sig.find(suffix, start);
            if (end != std::string_view::npos) {
                cleanName = std::string(sig.substr(start, end - start));
            }
        }
#elif defined(__GNUC__) || defined(__clang__)
        // GCC/Clang: __PRETTY_FUNCTION__ 格式类似 "const char* ExtractClassName() [with T = MyNamespace::MyClass]"
        constexpr std::string_view sig = __PRETTY_FUNCTION__;
        constexpr std::string_view prefix = "[with T = ";
        constexpr char suffix = ']';

        auto start = sig.find(prefix);
        if (start != std::string_view::npos) {
            start += prefix.length();
            auto end = sig.find(suffix, start);
            if (end != std::string_view::npos) {
                cleanName = std::string(sig.substr(start, end - start));
            }
        }
#else
        // 回退方案：使用 typeid
        cleanName = typeid(T).name();
#endif
    }
    return cleanName.c_str();
}

/**
 * @brief BaseKey 默认：干净的类名（不带修饰符）
 * @tparam Base
 * @return
 */
template <typename Base>
inline const char* RegistryBaseKey()
{
    return ExtractClassName<Base>();
}

/**
 * @brief type_key 默认：干净的类名（不带修饰符）
 * @tparam T
 * @return
 */
template <typename T>
inline const char* RegistryTypeKey()
{
    return ExtractClassName<T>();
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
 * @tparam Base 基类类型
 * @tparam Args 构造函数参数类型列表（默认为空，即无参构造）
 */
template <typename Base, typename... Args>
class StaticRegistry {
public:
    /**
     * @brief 工厂函数返回 unique_ptr<Base>，接受 Args...
     */
    using Factory = std::function<std::unique_ptr<Base>(Args...)>;
    /**
     * @brief 原始创建者函数签名：返回 void*，接受 Args...
     */
    using RawCreator = std::function<void*(Args...)>;

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
                // 将 RawEntry 的 std::any 转回具体的 RawCreator
                try {
                    auto rawFunc = std::any_cast<RawCreator>(re.creator);

                    // 包装成类型安全的 Factory
                    Factory f = [c = std::move(rawFunc)](Args... args) -> std::unique_ptr<Base> {
                        // 转发参数
                        return std::unique_ptr<Base>(static_cast<Base*>(c(std::forward<Args>(args)...)));
                    };
                    cache.push_back(Entry { re.type_key, std::move(f) });
                } catch (const std::bad_any_cast&) {
                    // 如果签名不匹配（例如在这个 Base 下注册了错误参数的子类），这里会忽略
                    continue;
                }
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
        for (const auto& e : es) {
            out.push_back(e.type_key);
        }
        return out;
    }

    /**
     * @brief 通过字符串键创建（支持传参）
     * @param type_key
     * @param args 构造参数
     * @return
     */
    static std::unique_ptr<Base> create(std::string_view type_key, Args... args)
    {
        const auto& es = entries();
        for (const auto& e : es) {
            if (e.type_key == type_key) {
                return e.factory(std::forward<Args>(args)...);
            }
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
            if (e.type_key == type_key) {
                return e.factory;
            }
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
    static std::function<std::unique_ptr<Derived>(Args...)> factoryOf()
    {
        const char* key = RegistryTypeKey<Derived>();
        const auto& es = entries();
        for (const auto& e : es) {
            if (e.type_key == key) {
                auto bf = e.factory;
                return [bf = std::move(bf)](Args... args) -> std::unique_ptr<Derived> {
                    std::unique_ptr<Base> b = bf(std::forward<Args>(args)...);
                    return std::unique_ptr<Derived>(static_cast<Derived*>(b.release()));
                };
            }
        }
        return {};
    }

    static bool IsRegistered(std::string_view type_key)
    {
        const auto& es = entries();
        for (const auto& e : es) {
            if (e.type_key == type_key) {
                return true;
            }
        }
        return false;
    }

    template <typename Derived>
        requires std::derived_from<Derived, Base>
    static bool IsRegistered()
    {
        return IsRegistered(RegistryTypeKey<Derived>());
    }

    /**
     * @brief 供自动注册使用
     */
    static void AddRaw(const char* type_key, RawCreator creator)
    {
        // 存入 std::any
        RegistryHub::Instance().add(RegistryBaseKey<Base>(), type_key, std::any(creator));
    }
};

/**
 * @brief 自动注册基类
 * @tparam Base 基类
 * @tparam Args 构造参数类型列表
 */
template <typename Base, typename... Args>
class AutoRegistered {
public:
    template <typename Derived>
        requires std::derived_from<Derived, Base>
    struct Registrar {
        /**
         * @brief 跳板函数：接收 Args... 并 new Derived(args...)
         */
        static void* CreateTrampoline(Args... args)
        {
            return static_cast<Base*>(new Derived(std::forward<Args>(args)...));
        }

        Registrar()
        {
            // 仅当 Derived 是非抽象且可用 Args 构造时才进行注册
            if constexpr (!std::is_abstract_v<Derived> && std::is_constructible_v<Derived, Args...>) {
                StaticRegistry<Base, Args...>::AddRaw(
                    RegistryTypeKey<Derived>(),
                    std::function<void*(Args...)>(&CreateTrampoline));
            }
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

// 宏定义修改：支持变长参数 (__VA_ARGS__)
// 用法 1 (无参): AUTO_REGISTER(MyClass, MyBase)
// 用法 2 (带参): AUTO_REGISTER(MyClass, MyBase, int, std::string)
#ifndef AUTO_REGISTER
#define CONCAT_IMPL(a, b) a##b
#define CONCAT(a, b) CONCAT_IMPL(a, b)

#define AUTO_REGISTER(CLASS, BASE, ...)                                     \
    namespace {                                                             \
        /* CLASS 和 BASE 可以是 ns::MyClass、ns::MyBase 等带 :: 的限定名 */ \
        AutoRegistered<BASE, ##__VA_ARGS__>::Registrar<CLASS>               \
            CONCAT(auto_reg_, __COUNTER__);                                 \
    }
#endif
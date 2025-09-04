#pragma once

#include <QObject>

/**
 * @brief 注册表模板
 * @tparam Base
 */
template <typename Base>
class StaticRegistry {
public:
    using Factory = std::function<std::unique_ptr<Base>()>;
    struct Entry {
        const char* typeName;
        Factory factory;
    };
    static std::vector<Entry>& entries()
    {
        static std::vector<Entry> e;
        return e;
    }
    static void add(const char* typeName, Factory factory)
    {
        entries().push_back({ typeName, std::move(factory) });
    }
};

/**
 * @brief 自动注册基类
 * @tparam Base
 */
template <typename Base>
class AutoRegistered {
public:
    template <typename Derived>
    struct Registrar {
        Registrar()
        {
            StaticRegistry<Base>::add(typeid(Derived).name(), [] {
                return std::make_unique<Derived>();
            });
        }
    };
    template <typename Derived>
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
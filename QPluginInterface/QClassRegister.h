#pragma once

#include <QCoreApplication>
#include <QVariant>
#include <qDebug>

#ifndef GetQValueClassName
#define GetQValueClassName(value) [](const QObject* obj) { return obj->metaObject()->className(); }(value)
#endif

#ifndef GetQClassName
#define GetQClassName(className) #className
#endif

#ifndef RegisterQClass
#define RegisterQClass(className)                                          \
    [&]() {                                                                \
        auto&& rs = qApp->property(#className).value<void*>();             \
        if (rs == nullptr) {                                               \
            rs = new className();                                          \
            qApp->setProperty(#className, QVariant::fromValue((void*)rs)); \
        };                                                                 \
        assert(rs != nullptr);                                             \
        return static_cast<className*>(rs);                                \
    }()
#endif

#ifndef RegisterQClassP
#define RegisterQClassP(className, parent)                                 \
    [&]() {                                                                \
        auto&& rs = qApp->property(#className).value<void*>();             \
        if (rs == nullptr) {                                               \
            rs = new className(parent);                                    \
            qApp->setProperty(#className, QVariant::fromValue((void*)rs)); \
        };                                                                 \
        assert(rs != nullptr);                                             \
        return static_cast<className*>(rs);                                \
    }()
#endif

#ifndef GetRegisterQClass
// 获取注册的对象
#define GetRegisterQClass(className)                                        \
    [&]() {                                                                 \
        auto rs = qApp->property(#className).value<void*>();                \
        if (rs == nullptr)                                                  \
            qDebug() << "GetRegisterQClass:" << #className << "is nullptr"; \
        assert(rs != nullptr);                                              \
        return static_cast<className*>(rs);                                 \
    }()
#endif

#ifndef RegisterMetaType
#define RegisterMetaType(className)                                             \
    QT_BEGIN_NAMESPACE template <>                                              \
    struct QMetaTypeId<className> {                                             \
        enum { Defined = 1 };                                                   \
        static int qt_metatype_id()                                             \
        {                                                                       \
            static QBasicAtomicInt metatype_id = Q_BASIC_ATOMIC_INITIALIZER(0); \
            if (!metatype_id)                                                   \
                metatype_id = qRegisterMetaType<className>(#className);         \
            return metatype_id;                                                 \
        }                                                                       \
    };                                                                          \
    QT_END_NAMESPACE
#endif // !RegisterMetaType

// 注册指定类型指针转为属性值
#ifndef RegisterPropertyPtr
#define RegisterPropertyPtr(key, ptr)                          \
    [&] {                                                      \
        auto&& p = ptr;                                        \
        qApp->setProperty(key, QVariant::fromValue((void*)p)); \
        return p;                                              \
    }()
#endif

// 获取app属性值转为指定类型指针
#ifndef GetAppPropertyPtr
#define GetAppPropertyPtr(key, className)                            \
    [&]() -> className* {                                            \
        auto&& rs = qApp->property(key).value<void*>();              \
        if (rs == nullptr) {                                         \
            qDebug() << "GetAppPropertyPtr:" << key << "is nullptr"; \
            return nullptr;                                          \
        }                                                            \
        return reinterpret_cast<className*>(rs);                     \
    }()
#endif // !GetAppPropertyPtr

// set app属性值
#ifndef SetAppPropertyPtr
#define SetAppPropertyPtr(key, value) qApp->setProperty(key, QVariant::fromValue((void*)value))
#endif

#ifdef QPLUGINMANAGER
#ifndef GetPluginPtr
#define GetPluginPtr(className)                                  \
    [&](const QString& name) -> std::optional<className*> {      \
        auto&& opt = QPluginManager::Instance().load(name);      \
        assert(opt.has_value());                                 \
        if (!opt.has_value()) {                                  \
            qDebug() << "GetPluginPtr:" << name << "is nullptr"; \
            return { std::nullopt };                             \
        }                                                        \
        return { reinterpret_cast<className*>(opt.value()) };    \
    }(#className)
#endif // !GetPluginPtr
#else
#ifndef GetPluginPtr
#define GetPluginPtr(className) std::optional<className*>(std::nullopt)
#pragma message("QPluginManager is not defined, GetPluginPtr is nullptr")
#endif // !GetPluginPtr
#endif // !QPLUGINMANAGER
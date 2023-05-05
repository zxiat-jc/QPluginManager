#pragma once

#include <QCoreApplication>
#include <QVariant>

#ifndef GetQValueClassName
#define GetQValueClassName(value) [](const QObject* obj) { return obj->metaObject()->className(); }(value)
#endif

#ifndef GetQClassName
#define GetQClassName(className) #className
#endif

#ifndef RegisterQClass
#define RegisterQClass(className)                                          \
    []() {                                                                 \
        auto&& rs = qApp->property(#className).value<void*>();             \
        if (rs == nullptr) {                                               \
            rs = new className();                                          \
            qApp->setProperty(#className, QVariant::fromValue((void*)rs)); \
        };                                                                 \
        assert(rs != nullptr);                                             \
        return static_cast<className*>(rs);                                \
    }()
#endif

#ifndef GetRegisterQClass
// 获取注册的对象
#define GetRegisterQClass(className)                                          \
    []() {                                                                    \
        auto rs = qApp->property(#className).value<void*>();                  \
        if (rs == nullptr)                                                    \
            qWarning() << "GetRegisterQClass:" << #className << "is nullptr"; \
        assert(rs != nullptr);                                                \
        return static_cast<className*>(rs);                                   \
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
                metatype_id = qRegisterMetaType<className>(#className,          \
                    reinterpret_cast<className*>(quintptr(-1)));                \
            return metatype_id;                                                 \
        }                                                                       \
    };                                                                          \
    QT_END_NAMESPACE
#endif // !RegisterMetaType

#ifdef QPLUGINMANAGER
#ifndef GetPluginPtr
#define GetPluginPtr(className)                                                      \
    [](const QString& name) -> std::optional<className*> {                           \
        auto&& opt = QPluginManager::Instance().load(name);                          \
        assert(opt.has_value());                                                     \
        if (!opt.has_value()) {                                                      \
            qWarning() << "GetPluginPtr:" << name << "is nullptr";                   \
        }                                                                            \
        return std::optional<className*>(reinterpret_cast<className*>(opt.value())); \
    }(#className)
#endif // !GetPluginPtr
#else
#ifndef GetPluginPtr
#define GetPluginPtr(className) std::optional<className*>(nullptr)
#endif // !GetPluginPtr
#endif // !QPLUGINMANAGER
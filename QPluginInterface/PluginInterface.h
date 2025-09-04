#pragma once

#include <QObject>

#ifndef BUILD_STATIC
#if defined(QPLUGININTERFACE_LIB)
#define QPLUGININTERFACE_EXPORT Q_DECL_EXPORT
#else
#define QPLUGININTERFACE_EXPORT Q_DECL_IMPORT
#endif
#else
#define QPLUGININTERFACE_EXPORT
#endif

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#pragma execution_character_set("utf-8")
#endif

class QPLUGININTERFACE_EXPORT PluginInterface : public QObject {
    Q_OBJECT;

public:
    virtual ~PluginInterface() = 0;

    /**
     * @brief 批量初始化
     * @param args 程序启动参数
     * @param error 初始化错误信息
     * @return 初始化状态
     */
    virtual bool initialize(const QStringList& args, QString& error) = 0;

    /**
     * @brief 初始化之后扩展初始化
     * @return 初始化状态
     */
    virtual bool extensionsInitialize() = 0;

    /**
     * @brief 延迟初始化，执行信号功能
     * @return 初始化状态
     */
    virtual bool delayedInitialize() = 0;

    /**
     * @brief 卸载时释放
     */
    virtual void release();
};

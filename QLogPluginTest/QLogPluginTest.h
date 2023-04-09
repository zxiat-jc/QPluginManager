#pragma once

#include <QtCore/qglobal.h>

#ifndef BUILD_STATIC
#if defined(QLOGPLUGINTEST_LIB)
#define QLOGPLUGINTEST_EXPORT Q_DECL_EXPORT
#else
#define QLOGPLUGINTEST_EXPORT Q_DECL_IMPORT
#endif
#else
#define QLOGPLUGINTEST_EXPORT
#endif

#include "PluginInterface.h"

/**
 * @brief 测试插件
 */
class QLOGPLUGINTEST_EXPORT QLogPluginTest : public PluginInterface {
    Q_OBJECT;

public:
    virtual ~QLogPluginTest() = 0;

    /**
     * @brief 输出测试信息
     */
    virtual bool log() = 0;

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
};

QT_BEGIN_NAMESPACE
Q_DECLARE_INTERFACE(QLogPluginTest, "cn.hiyj.QLogPluginTest")
QT_END_NAMESPACE
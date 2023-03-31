﻿#pragma once

#include "qlogplugintest_global.h"

#include "PluginInterface.h"

/**
 * @brief 测试插件
 */
class QLOGPLUGINTEST_EXPORT QLogPluginTest : public PluginInterface {
    Q_OBJECT;

public:
    virtual ~QLogPluginTest();

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
};

QT_BEGIN_NAMESPACE
Q_DECLARE_INTERFACE(QLogPluginTest, "cn.hiyj.QLogPluginTest")
QT_END_NAMESPACE
#pragma once

#include "qlogplugintest_global.h"

#include <QObject>

/**
 * @brief 测试插件
 */
class QLOGPLUGINTEST_EXPORT QLogPluginTest {
public:
    virtual ~QLogPluginTest();

    /**
     * @brief 输出测试信息
     */
    virtual bool log() = 0;
};

QT_BEGIN_NAMESPACE
Q_DECLARE_INTERFACE(QLogPluginTest, "cn.hiyj.QLogPluginTest")
QT_END_NAMESPACE
#pragma once

#include <QObject>

#include "QLogPluginTest.h"

class QLogPluginTestImpl : public QObject, public QLogPluginTest {
    Q_OBJECT;
    Q_INTERFACES(QLogPluginTest)
    Q_PLUGIN_METADATA(IID "cn.hiyj.QLogPluginTest" FILE "QLogPluginTest.json")
public:
    virtual ~QLogPluginTestImpl();

    bool log() override;
};

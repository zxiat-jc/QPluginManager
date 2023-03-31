#include "QLogPluginTestImpl.h"

#include <QDebug>

QLogPluginTestImpl::~QLogPluginTestImpl()
{
    qInfo() << "QLogPluginTestImpl::~QLogPluginTestImpl()";
}

bool QLogPluginTestImpl::log()
{
    qInfo() << "This is QLogPluginTest plugin";
    return true;
}

bool QLogPluginTestImpl::initialize(const QStringList& args, QString& error)
{
    qInfo() << "QLogPluginTest initialize";
    return true;
}

bool QLogPluginTestImpl::extensionsInitialize()
{
    qInfo() << "QLogPluginTest extensionsInitialize";
    return true;
}

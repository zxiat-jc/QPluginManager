#include "QLogPluginTestImpl.h"

#include <QDebug>

QLogPluginTestImpl::~QLogPluginTestImpl()
{
}

bool QLogPluginTestImpl::log()
{
    qInfo() << "This is QLogPluginTest plugin";
    return true;
}

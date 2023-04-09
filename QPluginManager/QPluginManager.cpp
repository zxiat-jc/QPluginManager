#include "QPluginManager.h"

#include <QDebug>

#include "QPluginManagerImpl.h"

QPluginManager::QPluginManager()
{
    qDebug() << "QPluginManager::QPluginManager()";
    this->_impl = new QPluginManagerImpl();
}

QPluginManager& QPluginManager::Instance()
{
    static QPluginManager instance;
    return instance;
}

QPluginManager::~QPluginManager()
{
    qDebug() << "QPluginManager::~QPluginManager()";
    delete this->_impl;
}

void QPluginManager::loadPlugin(const QString& path)
{
    this->_impl->loadPlugin(path);
}

void QPluginManager::loadPlugins(const QString& path)
{
    this->_impl->loadPlugins(path);
}

void QPluginManager::findLoadPlugins(const QString& path)
{
    this->_impl->findLoadPlugins(path);
}

bool QPluginManager::isLoad(const QString& name)
{
    return this->_impl->isLoad(name);
}

std::optional<PluginInterface*> QPluginManager::load(const QString& name)
{
    return this->_impl->load(name);
}

QList<QString> QPluginManager::pluginNames() const
{
    return this->_impl->pluginNames();
}

bool QPluginManager::initializes(const QStringList& args, QString& error)
{
    return this->_impl->initializes(args, error);
}

bool QPluginManager::extensionsInitialized()
{
    return this->_impl->extensionsInitialized();
}

bool QPluginManager::delayedInitialize()
{
    return this->_impl->delayedInitialize();
}
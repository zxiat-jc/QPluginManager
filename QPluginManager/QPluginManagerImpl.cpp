#include "QPluginManagerImpl.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QLibrary>

#include <algorithm>
#include <ranges>

QPluginManagerImpl::~QPluginManagerImpl()
{
    qDebug() << "QPluginManagerImpl::~QPluginManagerImpl()";
    for (size_t i = 0; i < _paths.size(); i++) {
        QString path = _paths.at(_paths.size() - 1);
        if (_pathNameMap.contains(path)) {
            QString name = _pathNameMap[path];
            _objMap.remove(name);
            _pathNameMap.remove(path);
        }
        if (!this->_pluginMap.contains(path)) {
            _paths.pop_back();
            continue;
        }
        auto&& loader = this->_pluginMap[path];
        if (loader) {
            loader->unload();
        }
        _paths.pop_back();
        this->_pluginMap.remove(path);
        qInfo() << "卸载插件:" << path;
    }
}

void QPluginManagerImpl::loadPlugin(const QString& path)
{
    QFileInfo fileInfo(path);
    if (!fileInfo.isFile() || fileInfo.suffix() != PLUGIN_SUFFIX) {
        qInfo() << "不是dll文件";
        return;
    }
    qInfo() << "加载插件路径:" << path;
    if (this->_pluginMap.contains(path)) {
        qInfo() << "插件已加载:" << path;
        return;
    }
    QSharedPointer<QPluginLoader> loader = QSharedPointer<QPluginLoader>(new QPluginLoader(path));
    if (!loader->load()) {
        loader->unload();
        return;
    }
    if (QObject* obj = loader->instance()) {
        auto&& meta = loader->metaData().value("MetaData").toObject();
        if (!obj->inherits("PluginInterface") || meta.isEmpty() || !meta.contains(NAME)) {
            loader->unload();
            return;
        }
        qInfo() << "元信息:" << meta;
        _pluginMap.insert(path, loader);
        _pathNameMap.insert(path, meta.value(NAME).toString());
        _objMap.insert(path, reinterpret_cast<PluginInterface*>(obj));
        _paths.push_back(path);
    }
}

void QPluginManagerImpl::loadPlugins(const QString& path)
{
    QDir pluginsDir(path);
    for (auto&& p : pluginsDir.entryList(QDir::AllEntries | QDir::NoDotAndDotDot)) {
        QFileInfo fi(path + "/" + p);
        if (fi.isFile() && fi.suffix() == PLUGIN_SUFFIX) {
            this->loadPlugin(path + "/" + p);
        }
    }
}

void QPluginManagerImpl::findLoadPlugins(const QString& path)
{
    QDir pluginsDir(path);
    for (auto&& p : pluginsDir.entryList(QDir::AllEntries | QDir::NoDotAndDotDot)) {
        QFileInfo fi(path + "/" + p);
        // 如果是目录
        if (fi.isDir()) {
            findLoadPlugins(path + "/" + p);
        } else if (fi.isFile() && fi.suffix() == PLUGIN_SUFFIX) {
            this->loadPlugin(path + "/" + p);
        }
    }
}

bool QPluginManagerImpl::isLoad(const QString& name)
{
    return _objMap.contains(name) && _objMap[name] != nullptr;
}

std::optional<PluginInterface*> QPluginManagerImpl::load(const QString& name)
{
    if (_objMap.contains(name) && _objMap[name] != nullptr) {
        return { _objMap[name] };
    }
    return { std::nullopt };
}

QList<QString> QPluginManagerImpl::pluginNames() const
{
    return _objMap.keys();
}

bool QPluginManagerImpl::initializes(const QStringList& args, QString& error)
{
    for (auto&& plugin : _objMap) {
        if (plugin) {
            plugin->initialize(args, error);
        }
    }
    return true;
}

bool QPluginManagerImpl::extensionsInitialized()
{
    std::ranges::for_each(std::views::reverse(_objMap.values()), [](const auto& plugin) { plugin->extensionsInitialize(); });
    return true;
}

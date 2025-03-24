#include "QPluginManagerImpl.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QLibrary>

#include <algorithm>

QPluginManagerImpl::~QPluginManagerImpl()
{
    qDebug() << "QPluginManagerImpl::~QPluginManagerImpl()";
    for (size_t i = 0; i < _paths.size(); i++) {
        QString path = _paths.at(i);
        if (_pathNameMap.contains(path)) {
            QString name = _pathNameMap[path];
            if (name == "QPluginManager") {
                continue;
            }
            _objMap[name]->release();
            _objMap[name]->deleteLater();
            _objMap.remove(name);
            _pathNameMap.remove(path);
            qInfo() << "卸载插件:" << path;
        }
    }
    _pathNameMap.clear();
    _objMap.clear();
    // 等待消息执行结束
    QCoreApplication::processEvents();
}

void QPluginManagerImpl::loadPlugin(const QString& path)
{
    QFileInfo fileInfo(path);
    if (!fileInfo.isFile() || fileInfo.suffix() != PLUGIN_SUFFIX) {
        qDebug() << "不是dll文件";
        return;
    }
    qDebug() << "加载插件路径:" << path;
    if (this->_paths.contains(path)) {
        qDebug() << "定制插件已加载:" << path;
        return;
    }
    QSharedPointer<QPluginLoader> loader(new QPluginLoader(path));
    if (loader->isLoaded()) {
        qDebug() << "普通插件已加载:" << path;
        return;
    }
    auto&& meta = loader->metaData().value("MetaData").toObject();
    if (meta.isEmpty() || !meta.contains(NAME) || !(meta.value("Interface").toString() == "PluginInterface")) {
        return;
    }
    if (!loader->load()) {
        qDebug() << "加载失败:" << loader->errorString();
        loader->unload();
        return;
    }
    if (QObject* obj = loader->instance()) {
        if (!obj->inherits("PluginInterface")) {
            return;
        }
        qDebug() << "元信息:" << meta;
        qInfo() << "加载插件名称:" << meta.value(NAME).toString();
        _pathNameMap.insert(path, meta.value(NAME).toString());
        _objMap.insert(meta.value(NAME).toString(), reinterpret_cast<PluginInterface*>(obj));
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
    auto&& values = _objMap.values();
    auto rbegin = std::make_reverse_iterator(values.end());
    auto rend = std::make_reverse_iterator(values.begin());

    std::for_each(rbegin, rend, [](const auto& plugin) {
        plugin->extensionsInitialize();
    });
    return true;
}

bool QPluginManagerImpl::delayedInitialize()
{
    auto&& values = _objMap.values();
    auto rbegin = std::make_reverse_iterator(values.end());
    auto rend = std::make_reverse_iterator(values.begin());

    std::for_each(rbegin, rend, [](const auto& plugin) {
        plugin->delayedInitialize();
    });
    return true;
}
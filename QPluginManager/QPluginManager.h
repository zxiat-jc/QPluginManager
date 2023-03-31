#pragma once

#include "qpluginmanager_global.h"

#include <QObject>

#include <optional>

class QPluginManagerImpl;
class QPLUGINMANAGER_EXPORT QPluginManager {
protected:
    QPluginManager();
    QPluginManagerImpl* _impl = nullptr;

public:
    /**
     * @brief 单例
     * @return 自身对象引用
     */
    static QPluginManager& Instance();

    /**
     * @brief 释放内部类
     */
    ~QPluginManager();

    /**
     * @brief 加载指定路径下的单个插件
     * @param path 精确到dll路径全名称
     */
    void loadPlugin(const QString& path);

    /**
     * @brief 加载指定路径下的所有插件
     * @param path 精确到dll路径全名称上级目录
     */
    void loadPlugins(const QString& path);

    /**
     * @brief 查找并加载指定目录下的所有插件
     * @param path 递归当前路径
     */
    void findLoadPlugins(const QString& path);

    /**
     * @brief 是否已经加载指定插件名
     * @param name 插件名
     * @return 是否已经加载
     */
    bool isLoad(const QString& name);

    /**
     * @brief 获取加载的插件实例指针
     * @param name 插件实例名
     * @return 插件实例指针
     */
    std::optional<QObject*> load(const QString& name);

    /**
     * @brief 获取插件列表
     * @return 插件名列表
     */
    QList<QString> pluginNames() const;

    /**
     * @brief 批量初始化
     * @param args 程序启动参数
     * @param error 初始化错误信息
     * @return 初始化状态
     */
    bool initializes(const QStringList& args, QString& error);

    /**
     * @brief 初始化之后扩展初始化
     * @return 初始化状态
     */
    bool extensionsInitialized();
};
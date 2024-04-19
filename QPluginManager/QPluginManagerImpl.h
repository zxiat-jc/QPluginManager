#pragma once

#include <QObject>

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#pragma execution_character_set("utf-8")
#endif

#include <QPluginLoader>
#include <QSharedPointer>

#include <optional>

#include "QPluginManager.h"

constexpr auto PLUGIN_SUFFIX = "dll";
constexpr auto NAME = "Name";

class QPluginManagerImpl : public QObject {
    Q_OBJECT
private:
    /**
     * @brief 加载器{路径，对象名}Map表
     */
    QMap<QString, QString> _pathNameMap;
    /**
     * @brief {对象名，指针}Map表
     */
    QMap<QString, PluginInterface*> _objMap;
    /**
     * @brief 加载顺序表
     */
    QList<QString> _paths;

public:
    ~QPluginManagerImpl() override;

    /**
     * @brief 加载指定路径下的单个插件
     * @param path 精确到dll路径全名称
     */
    void loadPlugin(const QString& path);

    /**
     * @brief 加载指定路径目录下的所有插件
     * @param path 精确到dll路径全名称上级目录
     */
    void loadPlugins(const QString& path);

    /**
     * @brief 递归查找并加载指定目录下的所有插件
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
    std::optional<PluginInterface*> load(const QString& name);

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

    /**
     * @brief 延迟初始化，执行信号功能
     * @return 初始化状态
     */
    bool delayedInitialize();
};

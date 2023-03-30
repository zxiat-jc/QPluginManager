#include "pch.h"

#include "CppUnitTest.h"

#include <QDir>
#include <QObject>

#include "QLogPluginTest.h"
#include "QPluginManager.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace QPluginManagerUnitTest {
TEST_CLASS(QPluginManagerUnitTest)
{
public:
    ;
    TEST_METHOD(loadPlugin)
    {
        QPluginManager::Instance().findLoadPlugins(QDir("..").absolutePath());
        auto&& plugins = QPluginManager::Instance().pluginNames();
        Assert::AreEqual(plugins.isEmpty(), false);
    }
    TEST_METHOD(isLoad)
    {
        QPluginManager::Instance().findLoadPlugins(QDir("..").absolutePath());
        auto&& plugins = QPluginManager::Instance().pluginNames();
        Assert::AreEqual(plugins.isEmpty(), false);
        Assert::AreEqual(QPluginManager::Instance().isLoad(plugins[0]), true);
    }
    TEST_METHOD(GetPtr)
    {
        QPluginManager::Instance().findLoadPlugins(QDir("..").absolutePath());
        auto&& plugins = QPluginManager::Instance().pluginNames();
        Assert::AreEqual(plugins.isEmpty(), false);
        Assert::AreEqual(QPluginManager::Instance().isLoad(plugins[0]), true);
        auto&& opt = QPluginManager::Instance().load(plugins[0]);
        Assert::AreEqual(opt.has_value(), true);
        auto&& ptr = qobject_cast<QLogPluginTest*>(opt.value());
        Assert::AreEqual(ptr->log(), true);
    }
};
}

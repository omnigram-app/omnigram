/*
This file is part of Omnigram,
an advanced fork of Telegram Desktop porting macOS aesthetics.

For license and copyright information please follow this link:
https://github.com/omnigram-app/omnigram/blob/main/LICENSE
*/
#include "omnigram/plugins/plugin_manager.h"
#include "core/application.h"
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>

namespace Omnigram::Plugins {

PluginManager &PluginManager::Instance() {
	static PluginManager instance;
	return instance;
}

void PluginManager::init(const QString &customPluginsPath) {
	if (!customPluginsPath.isEmpty()) {
		_pluginsDirectory = customPluginsPath;
	} else {
		_pluginsDirectory = cWorkingDir() + "plugins";
	}
	QDir().mkpath(_pluginsDirectory);
	discoverPlugins();
}

void PluginManager::discoverPlugins() {
	_plugins.clear();
	const auto dir = QDir(_pluginsDirectory);
	const auto entries = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
	for (const auto &info : entries) {
		const auto manifestPath = info.filePath() + "/manifest.json";
		QFile manifestFile(manifestPath);
		if (!manifestFile.open(QIODevice::ReadOnly)) {
			continue;
		}
		const auto doc = QJsonDocument::fromJson(manifestFile.readAll());
		if (!doc.isObject()) {
			continue;
		}
		const auto obj = doc.object();
		PluginManifest manifest;
		manifest.id = obj.value("id").toString(info.fileName());
		manifest.name = obj.value("name").toString(info.fileName());
		manifest.version = obj.value("version").toString("1.0.0");
		manifest.description = obj.value("description").toString();
		manifest.author = obj.value("author").toString();
		manifest.entryPoint = obj.value("main").toString("index.ts");
		manifest.enabled = obj.value("enabled").toBool(true);

		auto plugin = std::make_unique<Plugin>(manifest, info.filePath());
		if (manifest.enabled) {
			plugin->load();
		}
		_plugins.push_back(std::move(plugin));
	}
}

void PluginManager::setPluginEnabled(const QString &id, bool enabled) {
	for (const auto &plugin : _plugins) {
		if (plugin->manifest().id == id) {
			if (enabled) {
				plugin->load();
			} else {
				plugin->unload();
			}
			break;
		}
	}
}

void PluginManager::reloadPlugin(const QString &id) {
	for (const auto &plugin : _plugins) {
		if (plugin->manifest().id == id) {
			plugin->unload();
			plugin->load();
			break;
		}
	}
}

} // namespace Omnigram::Plugins

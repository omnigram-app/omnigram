/*
This file is part of Omnigram,
an advanced fork of Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/omnigram-app/omnigram/blob/main/LICENSE
*/
#include "omnigram/plugins/plugin_manager.h"
#include "core/application.h"

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QSettings>

namespace Omnigram::Plugins {
namespace {

constexpr auto kPluginStateGroup = "OmnigramPluginsState";

[[nodiscard]] bool ReadSavedPluginEnabled(const QString &id, bool fallback) {
	QSettings s;
	s.beginGroup(kPluginStateGroup);
	const auto val = s.value(id, fallback).toBool();
	s.endGroup();
	return val;
}

void SavePluginEnabled(const QString &id, bool enabled) {
	QSettings s;
	s.beginGroup(kPluginStateGroup);
	s.setValue(id, enabled);
	s.endGroup();
}

} // namespace

PluginManager &PluginManager::Instance() {
	static PluginManager instance;
	return instance;
}

void PluginManager::init(const QString &customPluginsPath) {
	if (_initialized && customPluginsPath.isEmpty()) {
		return;
	}
	if (!customPluginsPath.isEmpty()) {
		_pluginsDirectory = customPluginsPath;
	} else {
		_pluginsDirectory = cWorkingDir() + "plugins";
	}
	QDir().mkpath(_pluginsDirectory);
	_initialized = true;
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
		manifest.enabled = ReadSavedPluginEnabled(
			manifest.id,
			obj.value("enabled").toBool(true));
		manifest.standaloneSettingsTab = obj.value("standaloneSettingsTab").toBool(false);

		if (obj.contains("settings") && obj.value("settings").isArray()) {
			const auto arr = obj.value("settings").toArray();
			for (const auto &item : arr) {
				if (!item.isObject()) {
					continue;
				}
				const auto sObj = item.toObject();
				PluginSettingSchema schema;
				schema.key = sObj.value("key").toString();
				schema.type = sObj.value("type").toString("boolean");
				schema.label = sObj.value("label").toString(schema.key);
				schema.description = sObj.value("description").toString();
				schema.defaultValue = sObj.value("default");
				if (sObj.contains("options") && sObj.value("options").isArray()) {
					for (const auto &opt : sObj.value("options").toArray()) {
						schema.options.append(opt.toString());
					}
				}
				if (!schema.key.isEmpty()) {
					manifest.settings.push_back(std::move(schema));
				}
			}
		}

		if (obj.contains("rules") && obj.value("rules").isObject()) {
			const auto rulesObj = obj.value("rules").toObject();
			if (rulesObj.contains("beforeSend") && rulesObj.value("beforeSend").isArray()) {
				for (const auto &rItem : rulesObj.value("beforeSend").toArray()) {
					if (!rItem.isObject()) {
						continue;
					}
					const auto rObj = rItem.toObject();
					PluginTextRule rule;
					rule.from = rObj.value("from").toString();
					rule.to = rObj.value("to").toString();
					if (!rule.from.isEmpty()) {
						manifest.beforeSendRules.push_back(std::move(rule));
					}
				}
			}
		}

		auto plugin = std::make_unique<Plugin>(manifest, info.filePath());
		if (manifest.enabled) {
			plugin->load();
		}
		_plugins.push_back(std::move(plugin));
	}
}

Plugin *PluginManager::findPlugin(const QString &id) const {
	for (const auto &plugin : _plugins) {
		if (plugin->manifest().id == id) {
			return plugin.get();
		}
	}
	return nullptr;
}

void PluginManager::setPluginEnabled(const QString &id, bool enabled) {
	SavePluginEnabled(id, enabled);
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

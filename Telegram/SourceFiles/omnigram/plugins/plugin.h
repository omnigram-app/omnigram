/*
This file is part of Omnigram,
an advanced fork of Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/omnigram-app/omnigram/blob/main/LICENSE
*/
#pragma once

#include "base/basic_types.h"
#include <QtCore/QJsonObject>
#include <QtCore/QJsonValue>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>
#include <memory>
#include <vector>

namespace Omnigram::Plugins {

class PluginRuntime;

struct PluginSettingSchema {
	QString key;
	QString type; // "boolean", "string", "select", "button"
	QString label;
	QString description;
	QJsonValue defaultValue;
	QStringList options; // for "select"
};

struct PluginTextRule {
	QString from;
	QString to;
};

struct PluginManifest {
	QString id;
	QString name;
	QString version;
	QString description;
	QString author;
	QString entryPoint; // e.g. "index.ts" or "index.js"
	bool enabled = true;
	bool standaloneSettingsTab = false;
	std::vector<PluginSettingSchema> settings;
	std::vector<PluginTextRule> beforeSendRules;
};

class Plugin {
public:
	explicit Plugin(PluginManifest manifest, QString directory);
	~Plugin();

	[[nodiscard]] const PluginManifest &manifest() const { return _manifest; }
	[[nodiscard]] const QString &directory() const { return _directory; }
	[[nodiscard]] bool isRunning() const { return _running; }
	[[nodiscard]] const QString &lastError() const { return _lastError; }
	[[nodiscard]] bool hasSettings() const {
		return !_manifest.settings.empty();
	}

	[[nodiscard]] QVariant settingValue(const QString &key) const;
	void setSettingValue(const QString &key, const QVariant &value);
	void resetSettings();

	void setDynamicSettings(
		bool standaloneTab,
		std::vector<PluginSettingSchema> items);
	void invokeButton(const QString &key);

	bool load();
	void unload();

private:
	PluginManifest _manifest;
	QString _directory;
	QString _lastError;
	std::unique_ptr<PluginRuntime> _runtime;
	bool _running = false;
};

} // namespace Omnigram::Plugins

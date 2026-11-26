/*
This file is part of Omnigram,
an advanced fork of Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/omnigram-app/omnigram/blob/main/LICENSE
*/
#include "omnigram/plugins/plugin.h"
#include "omnigram/plugins/plugin_events.h"

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QSettings>

namespace Omnigram::Plugins {
namespace {

constexpr auto kPluginSettingsPrefix = "OmnigramPluginSettings/";

} // namespace

Plugin::Plugin(PluginManifest manifest, QString directory)
: _manifest(std::move(manifest))
, _directory(std::move(directory)) {
}

Plugin::~Plugin() {
	if (_running) {
		unload();
	}
}

QVariant Plugin::settingValue(const QString &key) const {
	QSettings s;
	s.beginGroup(QString::fromLatin1(kPluginSettingsPrefix) + _manifest.id);
	if (s.contains(key)) {
		const auto val = s.value(key);
		s.endGroup();
		return val;
	}
	s.endGroup();

	for (const auto &schema : _manifest.settings) {
		if (schema.key == key) {
			return schema.defaultValue.toVariant();
		}
	}
	return {};
}

void Plugin::setSettingValue(const QString &key, const QVariant &value) {
	QSettings s;
	s.beginGroup(QString::fromLatin1(kPluginSettingsPrefix) + _manifest.id);
	s.setValue(key, value);
	s.endGroup();
}

void Plugin::resetSettings() {
	QSettings s;
	s.beginGroup(QString::fromLatin1(kPluginSettingsPrefix) + _manifest.id);
	s.remove(QString());
	s.endGroup();
}

bool Plugin::load() {
	if (_running) {
		return true;
	}
	const auto entryPath = QDir(_directory).filePath(_manifest.entryPoint);
	if (!QFile::exists(entryPath) && _manifest.beforeSendRules.empty()) {
		return false;
	}
	if (!_manifest.beforeSendRules.empty()) {
		const auto rules = _manifest.beforeSendRules;
		PluginEvents::Instance().subscribeBeforeSend(
			_manifest.id,
			[rules](const QString &text) {
				auto result = text;
				for (const auto &rule : rules) {
					if (!rule.from.isEmpty()) {
						result.replace(rule.from, rule.to);
					}
				}
				return result;
			});
	}
	_running = true;
	return true;
}

void Plugin::unload() {
	if (!_running) {
		return;
	}
	PluginEvents::Instance().unsubscribePlugin(_manifest.id);
	_running = false;
}

} // namespace Omnigram::Plugins

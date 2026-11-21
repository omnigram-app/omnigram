/*
This file is part of Omnigram,
an advanced fork of Telegram Desktop porting macOS aesthetics.

For license and copyright information please follow this link:
https://github.com/omnigram-app/omnigram/blob/main/LICENSE
*/
#pragma once

#include "base/basic_types.h"
#include <QtCore/QString>
#include <QtCore/QJsonObject>
#include <vector>
#include <memory>

namespace Omnigram::Plugins {

struct PluginManifest {
	QString id;
	QString name;
	QString version;
	QString description;
	QString author;
	QString entryPoint; // e.g. "index.ts" or "index.js"
	bool enabled = true;
};

class Plugin {
public:
	explicit Plugin(PluginManifest manifest, QString directory);
	~Plugin();

	[[nodiscard]] const PluginManifest &manifest() const { return _manifest; }
	[[nodiscard]] const QString &directory() const { return _directory; }
	[[nodiscard]] bool isRunning() const { return _running; }

	bool load();
	void unload();

private:
	PluginManifest _manifest;
	QString _directory;
	bool _running = false;
};

} // namespace Omnigram::Plugins

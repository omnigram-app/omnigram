/*
This file is part of Omnigram,
an advanced fork of Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/omnigram-app/omnigram/blob/main/LICENSE
*/
#pragma once

#include "omnigram/plugins/plugin.h"
#include <QtCore/QString>
#include <vector>
#include <memory>

namespace Omnigram::Plugins {

class PluginManager {
public:
	static PluginManager &Instance();

	void init(const QString &customPluginsPath = QString());
	void discoverPlugins();

	[[nodiscard]] const std::vector<std::unique_ptr<Plugin>> &plugins() const {
		return _plugins;
	}

	[[nodiscard]] Plugin *findPlugin(const QString &id) const;

	[[nodiscard]] QString pluginsDirectory() const {
		return _pluginsDirectory;
	}

	void setPluginEnabled(const QString &id, bool enabled);
	void reloadPlugin(const QString &id);

private:
	PluginManager() = default;
	~PluginManager() = default;

	PluginManager(const PluginManager &) = delete;
	PluginManager &operator=(const PluginManager &) = delete;

	bool _initialized = false;
	QString _pluginsDirectory;
	std::vector<std::unique_ptr<Plugin>> _plugins;
};

} // namespace Omnigram::Plugins

/*
This file is part of Omnigram,
an advanced fork of Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/omnigram-app/omnigram/blob/main/LICENSE
*/
#pragma once

#include <QtCore/QString>
#include <memory>

namespace Omnigram::Plugins {

class Plugin;
struct PluginMessageEvent;

class PluginRuntime final {
public:
	explicit PluginRuntime(not_null<Plugin*> plugin);
	~PluginRuntime();

	PluginRuntime(const PluginRuntime &) = delete;
	PluginRuntime &operator=(const PluginRuntime &) = delete;

	[[nodiscard]] bool evaluateFile(const QString &filePath, QString &errorOut);
	void invokeButtonCallback(const QString &key);

	struct Impl;

private:
	std::unique_ptr<Impl> _impl;
};

} // namespace Omnigram::Plugins

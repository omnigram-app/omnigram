/*
This file is part of Omnigram,
an advanced fork of Telegram Desktop porting macOS aesthetics.

For license and copyright information please follow this link:
https://github.com/omnigram-app/omnigram/blob/main/LICENSE
*/
#include "omnigram/plugins/plugin.h"
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QDir>

namespace Omnigram::Plugins {

Plugin::Plugin(PluginManifest manifest, QString directory)
: _manifest(std::move(manifest))
, _directory(std::move(directory)) {
}

Plugin::~Plugin() {
	if (_running) {
		unload();
	}
}

bool Plugin::load() {
	if (_running) {
		return true;
	}
	const auto entryPath = QDir(_directory).filePath(_manifest.entryPoint);
	if (!QFile::exists(entryPath)) {
		return false;
	}
	_running = true;
	return true;
}

void Plugin::unload() {
	_running = false;
}

} // namespace Omnigram::Plugins

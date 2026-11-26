/*
This file is part of Omnigram,
an advanced fork of Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/omnigram-app/omnigram/blob/main/LICENSE
*/
#pragma once

#include "settings/settings_type.h"
#include <QtCore/QString>

namespace Ui {
class VerticalLayout;
} // namespace Ui

namespace Window {
class SessionController;
} // namespace Window

namespace Omnigram::Plugins {

class Plugin;

[[nodiscard]] ::Settings::Type PluginSectionId(const QString &pluginId);

void BuildPluginSettingsUI(
	not_null<Window::SessionController*> controller,
	not_null<Ui::VerticalLayout*> container,
	not_null<Plugin*> plugin);

void OpenPluginSettings(
	not_null<Window::SessionController*> controller,
	not_null<Plugin*> plugin);

} // namespace Omnigram::Plugins

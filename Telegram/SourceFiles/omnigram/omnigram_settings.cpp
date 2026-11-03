/*
This file is part of Omnigram,
an advanced fork of Telegram Desktop porting macOS aesthetics.

For license and copyright information please follow this link:
https://github.com/omnigram-app/omnigram/blob/main/LICENSE
*/
#include "omnigram/omnigram_settings.h"

#include <QtCore/QSettings>

namespace Omnigram {
namespace {

constexpr auto kSettingsGroup = "OmnigramSettings";
constexpr auto kKeyUIMode = "ui_mode";
constexpr auto kKeyBubbles = "macos_bubbles";
constexpr auto kKeyAnimations = "macos_animations";
constexpr auto kKeyInputBar = "macos_input_bar";
constexpr auto kKeyChatList = "macos_chat_list";
constexpr auto kKeySidebar = "macos_sidebar";
constexpr auto kKeyExactTime = "exact_time";
constexpr auto kKeyShowPeerId = "show_peer_id";
constexpr auto kKeyNoAds = "no_ads";
constexpr auto kKeyClientLimitsRemoved = "client_limits_removed";

} // namespace

Settings &Settings::Instance() {
	static Settings instance;
	return instance;
}

Settings::Settings()
: _uiMode(UIMode::MacOS)
, _macOSBubbles(true)
, _macOSAnimations(true)
, _macOSInputBar(true)
, _macOSChatList(true)
, _macOSSidebar(true)
, _exactTime(true)
, _showPeerId(true)
, _noAds(true)
, _clientLimitsRemoved(true) {
	readSettings();
}

UIMode Settings::uiMode() const {
	return _uiMode.current();
}

void Settings::setUIMode(UIMode mode) {
	if (_uiMode.current() == mode) {
		return;
	}
	_uiMode = mode;
	if (mode != UIMode::Custom) {
		updateFromPreset(mode);
	}
	writeSettings();
}

rpl::producer<UIMode> Settings::uiModeValue() const {
	return _uiMode.value();
}

bool Settings::macOSBubbles() const {
	return _macOSBubbles.current();
}

void Settings::setMacOSBubbles(bool enabled) {
	if (_macOSBubbles.current() == enabled) {
		return;
	}
	_macOSBubbles = enabled;
	if (_uiMode.current() != UIMode::Custom) {
		_uiMode = UIMode::Custom;
	}
	writeSettings();
}

rpl::producer<bool> Settings::macOSBubblesValue() const {
	return _macOSBubbles.value();
}

bool Settings::macOSAnimations() const {
	return _macOSAnimations.current();
}

void Settings::setMacOSAnimations(bool enabled) {
	if (_macOSAnimations.current() == enabled) {
		return;
	}
	_macOSAnimations = enabled;
	if (_uiMode.current() != UIMode::Custom) {
		_uiMode = UIMode::Custom;
	}
	writeSettings();
}

rpl::producer<bool> Settings::macOSAnimationsValue() const {
	return _macOSAnimations.value();
}

bool Settings::macOSInputBar() const {
	return _macOSInputBar.current();
}

void Settings::setMacOSInputBar(bool enabled) {
	if (_macOSInputBar.current() == enabled) {
		return;
	}
	_macOSInputBar = enabled;
	if (_uiMode.current() != UIMode::Custom) {
		_uiMode = UIMode::Custom;
	}
	writeSettings();
}

rpl::producer<bool> Settings::macOSInputBarValue() const {
	return _macOSInputBar.value();
}

bool Settings::macOSChatList() const {
	return _macOSChatList.current();
}

void Settings::setMacOSChatList(bool enabled) {
	if (_macOSChatList.current() == enabled) {
		return;
	}
	_macOSChatList = enabled;
	if (_uiMode.current() != UIMode::Custom) {
		_uiMode = UIMode::Custom;
	}
	writeSettings();
}

rpl::producer<bool> Settings::macOSChatListValue() const {
	return _macOSChatList.value();
}

bool Settings::macOSSidebar() const {
	return _macOSSidebar.current();
}

void Settings::setMacOSSidebar(bool enabled) {
	if (_macOSSidebar.current() == enabled) {
		return;
	}
	_macOSSidebar = enabled;
	if (_uiMode.current() != UIMode::Custom) {
		_uiMode = UIMode::Custom;
	}
	writeSettings();
}

rpl::producer<bool> Settings::macOSSidebarValue() const {
	return _macOSSidebar.value();
}

bool Settings::exactTime() const {
	return _exactTime.current();
}

void Settings::setExactTime(bool enabled) {
	if (_exactTime.current() == enabled) {
		return;
	}
	_exactTime = enabled;
	writeSettings();
}

rpl::producer<bool> Settings::exactTimeValue() const {
	return _exactTime.value();
}

bool Settings::showPeerId() const {
	return _showPeerId.current();
}

void Settings::setShowPeerId(bool enabled) {
	if (_showPeerId.current() == enabled) {
		return;
	}
	_showPeerId = enabled;
	writeSettings();
}

rpl::producer<bool> Settings::showPeerIdValue() const {
	return _showPeerId.value();
}

bool Settings::noAds() const {
	return _noAds.current();
}

void Settings::setNoAds(bool enabled) {
	if (_noAds.current() == enabled) {
		return;
	}
	_noAds = enabled;
	writeSettings();
}

rpl::producer<bool> Settings::noAdsValue() const {
	return _noAds.value();
}

bool Settings::clientLimitsRemoved() const {
	return _clientLimitsRemoved.current();
}

void Settings::setClientLimitsRemoved(bool enabled) {
	if (_clientLimitsRemoved.current() == enabled) {
		return;
	}
	_clientLimitsRemoved = enabled;
	writeSettings();
}

rpl::producer<bool> Settings::clientLimitsRemovedValue() const {
	return _clientLimitsRemoved.value();
}

void Settings::updateFromPreset(UIMode mode) {
	const auto enableAll = (mode == UIMode::MacOS);
	_macOSBubbles = enableAll;
	_macOSAnimations = enableAll;
	_macOSInputBar = enableAll;
	_macOSChatList = enableAll;
	_macOSSidebar = enableAll;
}

void Settings::resetToDefaults() {
	_uiMode = UIMode::MacOS;
	updateFromPreset(UIMode::MacOS);
	_exactTime = true;
	_showPeerId = true;
	_noAds = true;
	_clientLimitsRemoved = true;
	writeSettings();
}

void Settings::readSettings() {
	QSettings settings;
	settings.beginGroup(kSettingsGroup);

	const auto modeInt = settings.value(
		kKeyUIMode,
		static_cast<int>(UIMode::MacOS)).toInt();
	const auto mode = (modeInt >= 0 && modeInt <= 2)
		? static_cast<UIMode>(modeInt)
		: UIMode::MacOS;

	_uiMode = mode;
	if (mode == UIMode::Custom) {
		_macOSBubbles = settings.value(kKeyBubbles, true).toBool();
		_macOSAnimations = settings.value(kKeyAnimations, true).toBool();
		_macOSInputBar = settings.value(kKeyInputBar, true).toBool();
		_macOSChatList = settings.value(kKeyChatList, true).toBool();
		_macOSSidebar = settings.value(kKeySidebar, true).toBool();
	} else {
		updateFromPreset(mode);
	}
	_exactTime = settings.value(kKeyExactTime, true).toBool();
	_showPeerId = settings.value(kKeyShowPeerId, true).toBool();
	_noAds = settings.value(kKeyNoAds, true).toBool();
	_clientLimitsRemoved = settings.value(kKeyClientLimitsRemoved, true).toBool();

	settings.endGroup();
}

void Settings::writeSettings() {
	QSettings settings;
	settings.beginGroup(kSettingsGroup);

	settings.setValue(kKeyUIMode, static_cast<int>(_uiMode.current()));
	settings.setValue(kKeyBubbles, _macOSBubbles.current());
	settings.setValue(kKeyAnimations, _macOSAnimations.current());
	settings.setValue(kKeyInputBar, _macOSInputBar.current());
	settings.setValue(kKeyChatList, _macOSChatList.current());
	settings.setValue(kKeySidebar, _macOSSidebar.current());
	settings.setValue(kKeyExactTime, _exactTime.current());
	settings.setValue(kKeyShowPeerId, _showPeerId.current());
	settings.setValue(kKeyNoAds, _noAds.current());
	settings.setValue(kKeyClientLimitsRemoved, _clientLimitsRemoved.current());

	settings.endGroup();
}

} // namespace Omnigram

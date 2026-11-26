/*
This file is part of Omnigram,
an advanced fork of Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/omnigram-app/omnigram/blob/main/LICENSE
*/
#include "omnigram/omnigram_settings.h"

#include <QtCore/QSettings>

namespace Omnigram {
namespace {

constexpr auto kSettingsGroup = "OmnigramSettings";
constexpr auto kKeyExactTime = "exact_time";
constexpr auto kKeyShowPeerId = "show_peer_id";
constexpr auto kKeyNoAds = "no_ads";
constexpr auto kKeyClientLimitsRemoved = "client_limits_removed";
constexpr auto kKeyFastDownload = "fast_download";

} // namespace

Settings &Settings::Instance() {
	static Settings instance;
	return instance;
}

Settings::Settings()
: _exactTime(true)
, _showPeerId(true)
, _noAds(true)
, _clientLimitsRemoved(true)
, _fastDownload(true) {
	readSettings();
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

bool Settings::fastDownload() const {
	return _fastDownload.current();
}

void Settings::setFastDownload(bool enabled) {
	if (_fastDownload.current() == enabled) {
		return;
	}
	_fastDownload = enabled;
	writeSettings();
}

rpl::producer<bool> Settings::fastDownloadValue() const {
	return _fastDownload.value();
}

void Settings::resetToDefaults() {
	_exactTime = true;
	_showPeerId = true;
	_noAds = true;
	_clientLimitsRemoved = true;
	_fastDownload = true;
	writeSettings();
}

void Settings::readSettings() {
	QSettings s;
	s.beginGroup(kSettingsGroup);
	_exactTime = s.value(kKeyExactTime, true).toBool();
	_showPeerId = s.value(kKeyShowPeerId, true).toBool();
	_noAds = s.value(kKeyNoAds, true).toBool();
	_clientLimitsRemoved = s.value(kKeyClientLimitsRemoved, true).toBool();
	_fastDownload = s.value(kKeyFastDownload, true).toBool();
	s.endGroup();
}

void Settings::writeSettings() {
	QSettings s;
	s.beginGroup(kSettingsGroup);
	s.setValue(kKeyExactTime, _exactTime.current());
	s.setValue(kKeyShowPeerId, _showPeerId.current());
	s.setValue(kKeyNoAds, _noAds.current());
	s.setValue(kKeyClientLimitsRemoved, _clientLimitsRemoved.current());
	s.setValue(kKeyFastDownload, _fastDownload.current());
	s.endGroup();
}

} // namespace Omnigram

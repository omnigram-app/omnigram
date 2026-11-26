/*
This file is part of Omnigram,
an advanced fork of Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/omnigram-app/omnigram/blob/main/LICENSE
*/
#pragma once

#include <rpl/variable.h>
#include <QtCore/QString>

namespace Omnigram {

class Settings final {
public:
	static Settings &Instance();

	// Omnigram Power-User Features
	[[nodiscard]] bool exactTime() const;
	void setExactTime(bool enabled);
	[[nodiscard]] rpl::producer<bool> exactTimeValue() const;

	[[nodiscard]] bool showPeerId() const;
	void setShowPeerId(bool enabled);
	[[nodiscard]] rpl::producer<bool> showPeerIdValue() const;

	[[nodiscard]] bool noAds() const;
	void setNoAds(bool enabled);
	[[nodiscard]] rpl::producer<bool> noAdsValue() const;

	[[nodiscard]] bool clientLimitsRemoved() const;
	void setClientLimitsRemoved(bool enabled);
	[[nodiscard]] rpl::producer<bool> clientLimitsRemovedValue() const;

	[[nodiscard]] bool fastDownload() const;
	void setFastDownload(bool enabled);
	[[nodiscard]] rpl::producer<bool> fastDownloadValue() const;

	// Reset all settings to defaults
	void resetToDefaults();

private:
	Settings();

	void readSettings();
	void writeSettings();

	rpl::variable<bool> _exactTime;
	rpl::variable<bool> _showPeerId;
	rpl::variable<bool> _noAds;
	rpl::variable<bool> _clientLimitsRemoved;
	rpl::variable<bool> _fastDownload;
};

// Convenient direct helpers
[[nodiscard]] inline bool ExactTimeEnabled() {
	return Settings::Instance().exactTime();
}

[[nodiscard]] inline bool ShowPeerIdEnabled() {
	return Settings::Instance().showPeerId();
}

[[nodiscard]] inline bool NoAdsEnabled() {
	return Settings::Instance().noAds();
}

[[nodiscard]] inline bool ClientLimitsRemoved() {
	return Settings::Instance().clientLimitsRemoved();
}

[[nodiscard]] inline bool FastDownloadEnabled() {
	return Settings::Instance().fastDownload();
}

} // namespace Omnigram

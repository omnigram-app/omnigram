/*
This file is part of Omnigram,
an advanced fork of Telegram Desktop porting macOS aesthetics.

For license and copyright information please follow this link:
https://github.com/omnigram-app/omnigram/blob/main/LICENSE
*/
#pragma once

#include <rpl/variable.h>
#include <QtCore/QString>

namespace Omnigram {

enum class UIMode {
	Classic = 0,
	MacOS = 1,
	Custom = 2,
};

enum class SidebarMode {
	Classic = 0,
	MacOSDock = 1,
};

enum class DialogsLayout {
	Classic = 0,
	MacOS = 1,
};

enum class BubbleStyle {
	Classic = 0,
	MacOS = 1,
};

enum class InputBarStyle {
	Classic = 0,
	MacOSCapsule = 1,
};

enum class AnimationStyle {
	Classic = 0,
	MacOSSpring = 1,
};

class Settings final {
public:
	static Settings &Instance();

	// UIMode Preset
	[[nodiscard]] UIMode uiMode() const;
	void setUIMode(UIMode mode);
	[[nodiscard]] rpl::producer<UIMode> uiModeValue() const;

	// Granular Toggles (respect active preset or custom overrides)
	[[nodiscard]] bool macOSBubbles() const;
	void setMacOSBubbles(bool enabled);
	[[nodiscard]] rpl::producer<bool> macOSBubblesValue() const;

	[[nodiscard]] bool macOSAnimations() const;
	void setMacOSAnimations(bool enabled);
	[[nodiscard]] rpl::producer<bool> macOSAnimationsValue() const;

	[[nodiscard]] bool macOSInputBar() const;
	void setMacOSInputBar(bool enabled);
	[[nodiscard]] rpl::producer<bool> macOSInputBarValue() const;

	[[nodiscard]] bool macOSChatList() const;
	void setMacOSChatList(bool enabled);
	[[nodiscard]] rpl::producer<bool> macOSChatListValue() const;

	[[nodiscard]] bool macOSSidebar() const;
	void setMacOSSidebar(bool enabled);
	[[nodiscard]] rpl::producer<bool> macOSSidebarValue() const;

	// AyuGram / Kotatogram Features
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

	// Reset all settings to defaults
	void resetToDefaults();

private:
	Settings();

	void updateFromPreset(UIMode mode);
	void readSettings();
	void writeSettings();

	rpl::variable<UIMode> _uiMode;
	rpl::variable<bool> _macOSBubbles;
	rpl::variable<bool> _macOSAnimations;
	rpl::variable<bool> _macOSInputBar;
	rpl::variable<bool> _macOSChatList;
	rpl::variable<bool> _macOSSidebar;
	rpl::variable<bool> _exactTime;
	rpl::variable<bool> _showPeerId;
	rpl::variable<bool> _noAds;
	rpl::variable<bool> _clientLimitsRemoved;
};

// Convenient direct helpers
[[nodiscard]] inline bool UseMacOSBubbles() {
	return Settings::Instance().macOSBubbles();
}

[[nodiscard]] inline bool UseMacOSAnimations() {
	return Settings::Instance().macOSAnimations();
}

[[nodiscard]] inline bool UseMacOSInputBar() {
	return Settings::Instance().macOSInputBar();
}

[[nodiscard]] inline bool UseMacOSChatList() {
	return Settings::Instance().macOSChatList();
}

[[nodiscard]] inline bool UseMacOSSidebar() {
	return Settings::Instance().macOSSidebar();
}

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

} // namespace Omnigram

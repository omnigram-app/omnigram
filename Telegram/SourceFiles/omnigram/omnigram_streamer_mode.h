/*
This file is part of Omnigram,
an advanced fork of Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/omnigram-app/omnigram/blob/main/LICENSE
*/
#pragma once

#include <QtCore/QString>
#include <QtCore/QSet>
#include <QtGui/QImage>
#include <memory>
#include <vector>

class PeerData;
class UserData;

namespace Omnigram {

enum class StreamerTarget {
	Everyone = 0,
	Self = 1,
	Contacts = 2,
	Selected = 3,
};

struct StreamerRule {
	bool hideUsername = false;
	bool hidePeerId = false;
	bool hidePhone = false;
	bool hideName = false;
	bool hideAvatar = false;

	QString customUsername = u"*"_q;
	QString customPeerId = u"*"_q;
	QString customPhone = u"*"_q;
	QString customName = u"*"_q;
	QString customAvatarPath; // path to custom image file

	QSet<uint64> exceptions; // PeerIds that are exempt from this rule
	QSet<uint64> selectedPeers; // Only used for StreamerTarget::Selected
};

class StreamerMode final {
public:
	static StreamerMode &Instance();

	[[nodiscard]] bool enabled() const;
	void setEnabled(bool enabled);

	[[nodiscard]] StreamerRule &rule(StreamerTarget target);
	[[nodiscard]] const StreamerRule &rule(StreamerTarget target) const;

	// Query streamer mode for a specific peer
	[[nodiscard]] bool shouldHideUsername(not_null<const PeerData*> peer) const;
	[[nodiscard]] bool shouldHidePeerId(not_null<const PeerData*> peer) const;
	[[nodiscard]] bool shouldHidePhone(not_null<const PeerData*> peer) const;
	[[nodiscard]] bool shouldHideName(not_null<const PeerData*> peer) const;
	[[nodiscard]] bool shouldHideAvatar(not_null<const PeerData*> peer) const;

	[[nodiscard]] QString maskedUsername(not_null<const PeerData*> peer) const;
	[[nodiscard]] QString maskedPeerId(not_null<const PeerData*> peer) const;
	[[nodiscard]] QString maskedPhone(not_null<const PeerData*> peer) const;
	[[nodiscard]] QString maskedName(not_null<const PeerData*> peer) const;
	[[nodiscard]] QImage customAvatar(not_null<const PeerData*> peer) const;

	void save();
	void load();

private:
	StreamerMode();

	[[nodiscard]] const StreamerRule *matchingRule(not_null<const PeerData*> peer) const;

	bool _enabled = false;
	StreamerRule _everyone;
	StreamerRule _self;
	StreamerRule _contacts;
	StreamerRule _selected;

	mutable QImage _cachedAvatar;
	mutable QString _cachedAvatarPath;
};

[[nodiscard]] inline bool StreamerModeEnabled() {
	return StreamerMode::Instance().enabled();
}

} // namespace Omnigram

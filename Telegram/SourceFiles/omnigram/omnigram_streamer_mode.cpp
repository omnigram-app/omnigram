/*
This file is part of Omnigram,
an advanced fork of Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/omnigram-app/omnigram/blob/main/LICENSE
*/
#include "omnigram/omnigram_streamer_mode.h"
#include "data/data_peer.h"
#include "data/data_user.h"

#include <QtCore/QSettings>
#include <QtCore/QStringList>

namespace Omnigram {
namespace {

constexpr auto kGroup = "OmnigramStreamerMode";
constexpr auto kBadge = " [SM]";

QString FormatMasked(const QString &customText) {
	const auto text = customText.isEmpty() ? u"*"_q : customText;
	return text + QString::fromUtf8(kBadge);
}

QString TargetPrefix(StreamerTarget target) {
	switch (target) {
	case StreamerTarget::Everyone: return u"everyone_"_q;
	case StreamerTarget::Self: return u"self_"_q;
	case StreamerTarget::Contacts: return u"contacts_"_q;
	case StreamerTarget::Selected: return u"selected_"_q;
	}
	return QString();
}

void SaveRule(QSettings &s, StreamerTarget target, const StreamerRule &rule) {
	const auto prefix = TargetPrefix(target);
	s.setValue(prefix + u"hide_username"_q, rule.hideUsername);
	s.setValue(prefix + u"hide_peer_id"_q, rule.hidePeerId);
	s.setValue(prefix + u"hide_phone"_q, rule.hidePhone);
	s.setValue(prefix + u"hide_name"_q, rule.hideName);
	s.setValue(prefix + u"hide_avatar"_q, rule.hideAvatar);

	s.setValue(prefix + u"custom_username"_q, rule.customUsername);
	s.setValue(prefix + u"custom_peer_id"_q, rule.customPeerId);
	s.setValue(prefix + u"custom_phone"_q, rule.customPhone);
	s.setValue(prefix + u"custom_name"_q, rule.customName);
	s.setValue(prefix + u"custom_avatar_path"_q, rule.customAvatarPath);

	QStringList excList;
	for (const auto id : rule.exceptions) {
		excList.append(QString::number(id));
	}
	s.setValue(prefix + u"exceptions"_q, excList.join(','));

	if (target == StreamerTarget::Selected) {
		QStringList selList;
		for (const auto id : rule.selectedPeers) {
			selList.append(QString::number(id));
		}
		s.setValue(prefix + u"selected_peers"_q, selList.join(','));
	}
}

void LoadRule(QSettings &s, StreamerTarget target, StreamerRule &rule) {
	const auto prefix = TargetPrefix(target);
	rule.hideUsername = s.value(prefix + u"hide_username"_q, false).toBool();
	rule.hidePeerId = s.value(prefix + u"hide_peer_id"_q, false).toBool();
	rule.hidePhone = s.value(prefix + u"hide_phone"_q, false).toBool();
	rule.hideName = s.value(prefix + u"hide_name"_q, false).toBool();
	rule.hideAvatar = s.value(prefix + u"hide_avatar"_q, false).toBool();

	rule.customUsername = s.value(prefix + u"custom_username"_q, u"*"_q).toString();
	rule.customPeerId = s.value(prefix + u"custom_peer_id"_q, u"*"_q).toString();
	rule.customPhone = s.value(prefix + u"custom_phone"_q, u"*"_q).toString();
	rule.customName = s.value(prefix + u"custom_name"_q, u"*"_q).toString();
	rule.customAvatarPath = s.value(prefix + u"custom_avatar_path"_q, QString()).toString();

	rule.exceptions.clear();
	const auto excStr = s.value(prefix + u"exceptions"_q, QString()).toString();
	if (!excStr.isEmpty()) {
		for (const auto &part : excStr.split(',', Qt::SkipEmptyParts)) {
			rule.exceptions.insert(part.toULongLong());
		}
	}

	if (target == StreamerTarget::Selected) {
		rule.selectedPeers.clear();
		const auto selStr = s.value(prefix + u"selected_peers"_q, QString()).toString();
		if (!selStr.isEmpty()) {
			for (const auto &part : selStr.split(',', Qt::SkipEmptyParts)) {
				rule.selectedPeers.insert(part.toULongLong());
			}
		}
	}
}

} // namespace

StreamerMode &StreamerMode::Instance() {
	static StreamerMode instance;
	return instance;
}

StreamerMode::StreamerMode() {
	load();
}

bool StreamerMode::enabled() const {
	return _enabled;
}

void StreamerMode::setEnabled(bool enabled) {
	if (_enabled == enabled) {
		return;
	}
	_enabled = enabled;
	save();
}

StreamerRule &StreamerMode::rule(StreamerTarget target) {
	switch (target) {
	case StreamerTarget::Everyone: return _everyone;
	case StreamerTarget::Self: return _self;
	case StreamerTarget::Contacts: return _contacts;
	case StreamerTarget::Selected: return _selected;
	}
	return _everyone;
}

const StreamerRule &StreamerMode::rule(StreamerTarget target) const {
	switch (target) {
	case StreamerTarget::Everyone: return _everyone;
	case StreamerTarget::Self: return _self;
	case StreamerTarget::Contacts: return _contacts;
	case StreamerTarget::Selected: return _selected;
	}
	return _everyone;
}

const StreamerRule *StreamerMode::matchingRule(not_null<const PeerData*> peer) const {
	if (!_enabled) {
		return nullptr;
	}
	const auto peerIdVal = peer->id.value;

	// 1. Check Selected target
	if (_selected.selectedPeers.contains(peerIdVal)) {
		if (!_selected.exceptions.contains(peerIdVal)) {
			return &_selected;
		}
	}

	// 2. Check Self target
	if (peer->isSelf()) {
		if (!_self.exceptions.contains(peerIdVal)) {
			return &_self;
		}
		return nullptr;
	}

	// 3. Check Contacts target
	const auto user = peer->asUser();
	if (user && user->isContact()) {
		if (!_contacts.exceptions.contains(peerIdVal)) {
			return &_contacts;
		}
		return nullptr;
	}

	// 4. Fallback to Everyone target
	if (!_everyone.exceptions.contains(peerIdVal)) {
		return &_everyone;
	}

	return nullptr;
}

bool StreamerMode::shouldHideUsername(not_null<const PeerData*> peer) const {
	const auto r = matchingRule(peer);
	return r ? r->hideUsername : false;
}

bool StreamerMode::shouldHidePeerId(not_null<const PeerData*> peer) const {
	const auto r = matchingRule(peer);
	return r ? r->hidePeerId : false;
}

bool StreamerMode::shouldHidePhone(not_null<const PeerData*> peer) const {
	const auto r = matchingRule(peer);
	return r ? r->hidePhone : false;
}

bool StreamerMode::shouldHideName(not_null<const PeerData*> peer) const {
	const auto r = matchingRule(peer);
	return r ? r->hideName : false;
}

bool StreamerMode::shouldHideAvatar(not_null<const PeerData*> peer) const {
	const auto r = matchingRule(peer);
	return r ? r->hideAvatar : false;
}

QString StreamerMode::maskedUsername(not_null<const PeerData*> peer) const {
	const auto r = matchingRule(peer);
	if (!r || !r->hideUsername) {
		return QString();
	}
	return FormatMasked(r->customUsername);
}

QString StreamerMode::maskedPeerId(not_null<const PeerData*> peer) const {
	const auto r = matchingRule(peer);
	if (!r || !r->hidePeerId) {
		return QString();
	}
	return FormatMasked(r->customPeerId);
}

QString StreamerMode::maskedPhone(not_null<const PeerData*> peer) const {
	const auto r = matchingRule(peer);
	if (!r || !r->hidePhone) {
		return QString();
	}
	return FormatMasked(r->customPhone);
}

QString StreamerMode::maskedName(not_null<const PeerData*> peer) const {
	const auto r = matchingRule(peer);
	if (!r || !r->hideName) {
		return QString();
	}
	return FormatMasked(r->customName);
}

QImage StreamerMode::customAvatar(not_null<const PeerData*> peer) const {
	const auto r = matchingRule(peer);
	if (!r || !r->hideAvatar || r->customAvatarPath.isEmpty()) {
		return QImage();
	}
	if (_cachedAvatarPath != r->customAvatarPath) {
		_cachedAvatarPath = r->customAvatarPath;
		_cachedAvatar = QImage(r->customAvatarPath);
	}
	return _cachedAvatar;
}

void StreamerMode::save() {
	QSettings s;
	s.beginGroup(kGroup);
	s.setValue(u"enabled"_q, _enabled);
	SaveRule(s, StreamerTarget::Everyone, _everyone);
	SaveRule(s, StreamerTarget::Self, _self);
	SaveRule(s, StreamerTarget::Contacts, _contacts);
	SaveRule(s, StreamerTarget::Selected, _selected);
	s.endGroup();
}

void StreamerMode::load() {
	QSettings s;
	s.beginGroup(kGroup);
	_enabled = s.value(u"enabled"_q, false).toBool();
	LoadRule(s, StreamerTarget::Everyone, _everyone);
	LoadRule(s, StreamerTarget::Self, _self);
	LoadRule(s, StreamerTarget::Contacts, _contacts);
	LoadRule(s, StreamerTarget::Selected, _selected);
	s.endGroup();
}

} // namespace Omnigram

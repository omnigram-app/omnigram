/*
This file is part of Omnigram,
an advanced fork of Telegram Desktop porting macOS aesthetics.

For license and copyright information please follow this link:
https://github.com/omnigram-app/omnigram/blob/main/LICENSE
*/
#pragma once

#include "base/basic_types.h"
#include <QtCore/QString>
#include <QtCore/QJsonObject>
#include <functional>

namespace Omnigram::Plugins {

struct PluginMessageEvent {
	int64 id = 0;
	int64 peerId = 0;
	int64 senderId = 0;
	QString text;
	int32 date = 0;
	bool isOutgoing = false;
};

class PluginEvents {
public:
	static PluginEvents &Instance();

	using MessageHandler = std::function<void(const PluginMessageEvent &)>;
	using BeforeSendHandler = std::function<QString(const QString &)>;

	void subscribeMessage(const QString &pluginId, MessageHandler handler);
	void subscribeBeforeSend(const QString &pluginId, BeforeSendHandler handler);
	void unsubscribePlugin(const QString &pluginId);

	void dispatchMessage(const PluginMessageEvent &event);
	[[nodiscard]] QString processBeforeSend(QString text);

private:
	PluginEvents() = default;

	struct Subscription {
		QString pluginId;
		MessageHandler messageHandler;
		BeforeSendHandler beforeSendHandler;
	};
	std::vector<Subscription> _subscriptions;
};

} // namespace Omnigram::Plugins

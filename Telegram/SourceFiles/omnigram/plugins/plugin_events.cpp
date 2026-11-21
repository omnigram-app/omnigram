/*
This file is part of Omnigram,
an advanced fork of Telegram Desktop porting macOS aesthetics.

For license and copyright information please follow this link:
https://github.com/omnigram-app/omnigram/blob/main/LICENSE
*/
#include "omnigram/plugins/plugin_events.h"
#include <algorithm>

namespace Omnigram::Plugins {

PluginEvents &PluginEvents::Instance() {
	static PluginEvents instance;
	return instance;
}

void PluginEvents::subscribeMessage(const QString &pluginId, MessageHandler handler) {
	_subscriptions.push_back({ pluginId, std::move(handler), nullptr });
}

void PluginEvents::subscribeBeforeSend(const QString &pluginId, BeforeSendHandler handler) {
	_subscriptions.push_back({ pluginId, nullptr, std::move(handler) });
}

void PluginEvents::unsubscribePlugin(const QString &pluginId) {
	_subscriptions.erase(
		std::remove_if(
			_subscriptions.begin(),
			_subscriptions.end(),
			[&](const Subscription &sub) { return sub.pluginId == pluginId; }),
		_subscriptions.end());
}

void PluginEvents::dispatchMessage(const PluginMessageEvent &event) {
	for (const auto &sub : _subscriptions) {
		if (sub.messageHandler) {
			sub.messageHandler(event);
		}
	}
}

QString PluginEvents::processBeforeSend(QString text) {
	for (const auto &sub : _subscriptions) {
		if (sub.beforeSendHandler) {
			text = sub.beforeSendHandler(text);
		}
	}
	return text;
}

} // namespace Omnigram::Plugins

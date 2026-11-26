/*
This file is part of Omnigram,
an advanced fork of Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/omnigram-app/omnigram/blob/main/LICENSE
*/
#include "omnigram/plugins/plugin_runtime.h"
#include "omnigram/plugins/plugin.h"
#include "omnigram/plugins/plugin_events.h"

#include "base/flat_map.h"
#include "crl/crl.h"
#include "ui/toast/toast.h"
#include "logs.h"

#include <QtCore/QFile>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QRegularExpression>
#include <QtCore/QSettings>

extern "C" {
#include "quickjs.h"
}

namespace Omnigram::Plugins {
namespace {

constexpr auto kPluginStoragePrefix = "OmnigramPluginStorage/";

[[nodiscard]] QString ToQString(JSContext *ctx, JSValueConst val) {
	const auto str = JS_ToCString(ctx, val);
	if (!str) {
		return {};
	}
	const auto result = QString::fromUtf8(str);
	JS_FreeCString(ctx, str);
	return result;
}

[[nodiscard]] QString DumpException(JSContext *ctx) {
	const auto exc = JS_GetException(ctx);
	const auto msg = ToQString(ctx, exc);
	QString stack;
	if (JS_IsObject(exc)) {
		const auto stackVal = JS_GetPropertyStr(ctx, exc, "stack");
		if (!JS_IsUndefined(stackVal)) {
			stack = ToQString(ctx, stackVal);
		}
		JS_FreeValue(ctx, stackVal);
	}
	JS_FreeValue(ctx, exc);
	return stack.isEmpty() ? msg : (msg + u"\n"_q + stack);
}

[[nodiscard]] QString PreprocessScript(const QString &source, bool isTypeScript) {
	if (!isTypeScript) {
		return source;
	}
	auto lines = source.split('\n');
	for (auto &line : lines) {
		const auto trimmed = line.trimmed();
		if (trimmed.startsWith(u"/// <reference"_q)
			|| trimmed == u"export {};"_q
			|| trimmed == u"export {}"_q) {
			line = u"// "_q + line;
		}
	}
	return lines.join('\n');
}

} // namespace

struct PluginRuntime::Impl {
	explicit Impl(not_null<Plugin*> p)
	: plugin(p) {
	}

	~Impl() {
		if (ctx) {
			for (auto &[key, fn] : buttonCallbacks) {
				JS_FreeValue(ctx, fn);
			}
			buttonCallbacks.clear();
			for (auto &fn : messageListeners) {
				JS_FreeValue(ctx, fn);
			}
			messageListeners.clear();
			for (auto &fn : beforeSendListeners) {
				JS_FreeValue(ctx, fn);
			}
			beforeSendListeners.clear();
			JS_FreeContext(ctx);
		}
		if (rt) {
			JS_FreeRuntime(rt);
		}
	}

	not_null<Plugin*> plugin;
	JSRuntime *rt = nullptr;
	JSContext *ctx = nullptr;
	std::vector<JSValue> messageListeners;
	std::vector<JSValue> beforeSendListeners;
	base::flat_map<QString, JSValue> buttonCallbacks;
};

namespace {

JSValue JsConsoleLog(
		JSContext *ctx,
		JSValueConst thisVal,
		int argc,
		JSValueConst *argv) {
	QStringList parts;
	for (int i = 0; i < argc; ++i) {
		parts.append(ToQString(ctx, argv[i]));
	}
	LOG(("Omnigram Plugin: %1").arg(parts.join(' ')));
	return JS_UNDEFINED;
}

JSValue JsShowToast(
		JSContext *ctx,
		JSValueConst thisVal,
		int argc,
		JSValueConst *argv) {
	if (argc < 1) {
		return JS_UNDEFINED;
	}
	const auto text = ToQString(ctx, argv[0]);
	if (!text.isEmpty()) {
		crl::on_main([text] {
			Ui::Toast::Show(text);
		});
	}
	return JS_UNDEFINED;
}

JSValue JsStorageGet(
		JSContext *ctx,
		JSValueConst thisVal,
		int argc,
		JSValueConst *argv,
		int magic,
		JSValueConst *funcData) {
	const auto impl = static_cast<PluginRuntime::Impl*>(
		JS_GetContextOpaque(ctx));
	if (!impl || argc < 1) {
		return JS_UNDEFINED;
	}
	const auto key = ToQString(ctx, argv[0]);
	QSettings s;
	s.beginGroup(
		QString::fromLatin1(kPluginStoragePrefix)
			+ impl->plugin->manifest().id);
	if (!s.contains(key)) {
		s.endGroup();
		return (argc >= 2) ? JS_DupValue(ctx, argv[1]) : JS_UNDEFINED;
	}
	const auto jsonStr = s.value(key).toString().toUtf8();
	s.endGroup();
	if (jsonStr.isEmpty()) {
		return (argc >= 2) ? JS_DupValue(ctx, argv[1]) : JS_UNDEFINED;
	}
	const auto val = JS_ParseJSON(ctx, jsonStr.constData(), jsonStr.size(), "<storage>");
	if (JS_IsException(val)) {
		JS_FreeValue(ctx, val);
		return (argc >= 2) ? JS_DupValue(ctx, argv[1]) : JS_UNDEFINED;
	}
	return val;
}

JSValue JsStorageSet(
		JSContext *ctx,
		JSValueConst thisVal,
		int argc,
		JSValueConst *argv) {
	const auto impl = static_cast<PluginRuntime::Impl*>(
		JS_GetContextOpaque(ctx));
	if (!impl || argc < 2) {
		return JS_UNDEFINED;
	}
	const auto key = ToQString(ctx, argv[0]);
	const auto jsonVal = JS_JSONStringify(ctx, argv[1], JS_UNDEFINED, JS_UNDEFINED);
	if (JS_IsException(jsonVal)) {
		JS_FreeValue(ctx, jsonVal);
		return JS_UNDEFINED;
	}
	const auto serialized = ToQString(ctx, jsonVal);
	JS_FreeValue(ctx, jsonVal);

	QSettings s;
	s.beginGroup(
		QString::fromLatin1(kPluginStoragePrefix)
			+ impl->plugin->manifest().id);
	s.setValue(key, serialized);
	s.endGroup();
	return JS_UNDEFINED;
}

JSValue JsStorageDelete(
		JSContext *ctx,
		JSValueConst thisVal,
		int argc,
		JSValueConst *argv) {
	const auto impl = static_cast<PluginRuntime::Impl*>(
		JS_GetContextOpaque(ctx));
	if (!impl || argc < 1) {
		return JS_UNDEFINED;
	}
	const auto key = ToQString(ctx, argv[0]);
	QSettings s;
	s.beginGroup(
		QString::fromLatin1(kPluginStoragePrefix)
			+ impl->plugin->manifest().id);
	s.remove(key);
	s.endGroup();
	return JS_UNDEFINED;
}

JSValue JsRegisterSettings(
		JSContext *ctx,
		JSValueConst thisVal,
		int argc,
		JSValueConst *argv) {
	const auto impl = static_cast<PluginRuntime::Impl*>(
		JS_GetContextOpaque(ctx));
	if (!impl || argc < 1 || !JS_IsObject(argv[0])) {
		return JS_UNDEFINED;
	}
	const auto standaloneVal = JS_GetPropertyStr(ctx, argv[0], "standaloneTab");
	const auto standalone = JS_ToBool(ctx, standaloneVal) > 0;
	JS_FreeValue(ctx, standaloneVal);

	std::vector<PluginSettingSchema> items;
	const auto itemsVal = JS_GetPropertyStr(ctx, argv[0], "items");
	if (JS_IsArray(itemsVal)) {
		const auto lenVal = JS_GetPropertyStr(ctx, itemsVal, "length");
		uint32_t len = 0;
		JS_ToUint32(ctx, &len, lenVal);
		JS_FreeValue(ctx, lenVal);

		for (uint32_t i = 0; i < len; ++i) {
			const auto item = JS_GetPropertyUint32(ctx, itemsVal, i);
			if (!JS_IsObject(item)) {
				JS_FreeValue(ctx, item);
				continue;
			}
			PluginSettingSchema schema;
			const auto idVal = JS_GetPropertyStr(ctx, item, "id");
			const auto keyVal = JS_GetPropertyStr(ctx, item, "key");
			schema.key = !JS_IsUndefined(idVal)
				? ToQString(ctx, idVal)
				: ToQString(ctx, keyVal);
			JS_FreeValue(ctx, idVal);
			JS_FreeValue(ctx, keyVal);

			const auto typeVal = JS_GetPropertyStr(ctx, item, "type");
			schema.type = ToQString(ctx, typeVal);
			JS_FreeValue(ctx, typeVal);

			const auto titleVal = JS_GetPropertyStr(ctx, item, "title");
			const auto labelVal = JS_GetPropertyStr(ctx, item, "label");
			schema.label = !JS_IsUndefined(titleVal)
				? ToQString(ctx, titleVal)
				: ToQString(ctx, labelVal);
			JS_FreeValue(ctx, titleVal);
			JS_FreeValue(ctx, labelVal);

			const auto descVal = JS_GetPropertyStr(ctx, item, "description");
			if (!JS_IsUndefined(descVal)) {
				schema.description = ToQString(ctx, descVal);
			}
			JS_FreeValue(ctx, descVal);

			const auto defVal = JS_GetPropertyStr(ctx, item, "default");
			if (!JS_IsUndefined(defVal)) {
				if (JS_IsBool(defVal)) {
					schema.defaultValue = QJsonValue(JS_ToBool(ctx, defVal) > 0);
				} else if (JS_IsNumber(defVal)) {
					double d = 0;
					JS_ToFloat64(ctx, &d, defVal);
					schema.defaultValue = QJsonValue(d);
				} else {
					schema.defaultValue = QJsonValue(ToQString(ctx, defVal));
				}
			}
			JS_FreeValue(ctx, defVal);

			const auto optsVal = JS_GetPropertyStr(ctx, item, "options");
			if (JS_IsArray(optsVal)) {
				const auto olenVal = JS_GetPropertyStr(ctx, optsVal, "length");
				uint32_t olen = 0;
				JS_ToUint32(ctx, &olen, olenVal);
				JS_FreeValue(ctx, olenVal);
				for (uint32_t j = 0; j < olen; ++j) {
					const auto opt = JS_GetPropertyUint32(ctx, optsVal, j);
					if (JS_IsObject(opt)) {
						const auto v = JS_GetPropertyStr(ctx, opt, "value");
						schema.options.append(ToQString(ctx, v));
						JS_FreeValue(ctx, v);
					} else {
						schema.options.append(ToQString(ctx, opt));
					}
					JS_FreeValue(ctx, opt);
				}
			}
			JS_FreeValue(ctx, optsVal);

			const auto onClickVal = JS_GetPropertyStr(ctx, item, "onClick");
			if (JS_IsFunction(ctx, onClickVal) && !schema.key.isEmpty()) {
				const auto prev = impl->buttonCallbacks.find(schema.key);
				if (prev != impl->buttonCallbacks.end()) {
					JS_FreeValue(ctx, prev->second);
					prev->second = JS_DupValue(ctx, onClickVal);
				} else {
					impl->buttonCallbacks.emplace(
						schema.key,
						JS_DupValue(ctx, onClickVal));
				}
			}
			JS_FreeValue(ctx, onClickVal);

			if (!schema.key.isEmpty()) {
				items.push_back(std::move(schema));
			}
			JS_FreeValue(ctx, item);
		}
	}
	JS_FreeValue(ctx, itemsVal);

	impl->plugin->setDynamicSettings(standalone, std::move(items));
	return JS_UNDEFINED;
}

JSValue JsMessagesOn(
		JSContext *ctx,
		JSValueConst thisVal,
		int argc,
		JSValueConst *argv) {
	const auto impl = static_cast<PluginRuntime::Impl*>(
		JS_GetContextOpaque(ctx));
	if (!impl || argc < 2 || !JS_IsFunction(ctx, argv[1])) {
		return JS_UNDEFINED;
	}
	const auto eventName = ToQString(ctx, argv[0]);
	if (eventName != u"message"_q) {
		return JS_UNDEFINED;
	}
	const auto listener = JS_DupValue(ctx, argv[1]);
	impl->messageListeners.push_back(listener);

	if (impl->messageListeners.size() == 1) {
		PluginEvents::Instance().subscribeMessage(
			impl->plugin->manifest().id,
			[impl](const PluginMessageEvent &ev) {
				if (!impl->ctx) {
					return;
				}
				const auto obj = JS_NewObject(impl->ctx);
				JS_SetPropertyStr(impl->ctx, obj, "id", JS_NewInt64(impl->ctx, ev.id));
				JS_SetPropertyStr(impl->ctx, obj, "peerId", JS_NewInt64(impl->ctx, ev.peerId));
				JS_SetPropertyStr(impl->ctx, obj, "senderId", JS_NewInt64(impl->ctx, ev.senderId));
				const auto utf8 = ev.text.toUtf8();
				JS_SetPropertyStr(
					impl->ctx,
					obj,
					"text",
					JS_NewStringLen(impl->ctx, utf8.constData(), utf8.size()));
				JS_SetPropertyStr(impl->ctx, obj, "date", JS_NewInt32(impl->ctx, ev.date));
				JS_SetPropertyStr(impl->ctx, obj, "isOutgoing", JS_NewBool(impl->ctx, ev.isOutgoing));

				for (const auto &fn : impl->messageListeners) {
					JSValue arg = JS_DupValue(impl->ctx, obj);
					const auto ret = JS_Call(impl->ctx, fn, JS_UNDEFINED, 1, &arg);
					JS_FreeValue(impl->ctx, arg);
					if (JS_IsException(ret)) {
						LOG(("Omnigram Plugin Error: %1").arg(DumpException(impl->ctx)));
					}
					JS_FreeValue(impl->ctx, ret);
				}
				JS_FreeValue(impl->ctx, obj);
			});
	}
	return JS_UNDEFINED;
}

JSValue JsMessagesOnBeforeSend(
		JSContext *ctx,
		JSValueConst thisVal,
		int argc,
		JSValueConst *argv) {
	const auto impl = static_cast<PluginRuntime::Impl*>(
		JS_GetContextOpaque(ctx));
	if (!impl || argc < 1 || !JS_IsFunction(ctx, argv[0])) {
		return JS_UNDEFINED;
	}
	const auto listener = JS_DupValue(ctx, argv[0]);
	impl->beforeSendListeners.push_back(listener);

	if (impl->beforeSendListeners.size() == 1) {
		PluginEvents::Instance().subscribeBeforeSend(
			impl->plugin->manifest().id,
			[impl](const QString &text) {
				if (!impl->ctx) {
					return text;
				}
				auto current = text;
				for (const auto &fn : impl->beforeSendListeners) {
					const auto utf8 = current.toUtf8();
					JSValue arg = JS_NewStringLen(impl->ctx, utf8.constData(), utf8.size());
					const auto ret = JS_Call(impl->ctx, fn, JS_UNDEFINED, 1, &arg);
					JS_FreeValue(impl->ctx, arg);
					if (JS_IsException(ret)) {
						LOG(("Omnigram Plugin Error: %1").arg(DumpException(impl->ctx)));
					} else if (JS_IsString(ret)) {
						current = ToQString(impl->ctx, ret);
					}
					JS_FreeValue(impl->ctx, ret);
				}
				return current;
			});
	}
	return JS_UNDEFINED;
}

} // namespace

PluginRuntime::PluginRuntime(not_null<Plugin*> plugin)
: _impl(std::make_unique<Impl>(plugin)) {
	_impl->rt = JS_NewRuntime();
	_impl->ctx = JS_NewContext(_impl->rt);
	JS_SetContextOpaque(_impl->ctx, _impl.get());

	const auto global = JS_GetGlobalObject(_impl->ctx);

	// console
	const auto consoleObj = JS_NewObject(_impl->ctx);
	JS_SetPropertyStr(
		_impl->ctx,
		consoleObj,
		"log",
		JS_NewCFunction(_impl->ctx, JsConsoleLog, "log", 1));
	JS_SetPropertyStr(
		_impl->ctx,
		consoleObj,
		"warn",
		JS_NewCFunction(_impl->ctx, JsConsoleLog, "warn", 1));
	JS_SetPropertyStr(
		_impl->ctx,
		consoleObj,
		"error",
		JS_NewCFunction(_impl->ctx, JsConsoleLog, "error", 1));
	JS_SetPropertyStr(_impl->ctx, global, "console", consoleObj);

	// omnigram
	const auto omniObj = JS_NewObject(_impl->ctx);
	JS_SetPropertyStr(_impl->ctx, omniObj, "version", JS_NewString(_impl->ctx, "1.0.0"));

	// omnigram.manifest
	const auto &m = plugin->manifest();
	const auto manifestObj = JS_NewObject(_impl->ctx);
	const auto setStr = [&](const char *prop, const QString &val) {
		const auto utf8 = val.toUtf8();
		JS_SetPropertyStr(
			_impl->ctx,
			manifestObj,
			prop,
			JS_NewStringLen(_impl->ctx, utf8.constData(), utf8.size()));
	};
	setStr("id", m.id);
	setStr("name", m.name);
	setStr("version", m.version);
	setStr("description", m.description);
	setStr("author", m.author);
	setStr("main", m.entryPoint);
	JS_SetPropertyStr(_impl->ctx, manifestObj, "enabled", JS_NewBool(_impl->ctx, m.enabled));
	JS_SetPropertyStr(
		_impl->ctx,
		manifestObj,
		"standaloneSettingsTab",
		JS_NewBool(_impl->ctx, m.standaloneSettingsTab));
	JS_SetPropertyStr(_impl->ctx, omniObj, "manifest", manifestObj);

	// omnigram.ui
	const auto uiObj = JS_NewObject(_impl->ctx);
	JS_SetPropertyStr(
		_impl->ctx,
		uiObj,
		"showToast",
		JS_NewCFunction(_impl->ctx, JsShowToast, "showToast", 2));
	JS_SetPropertyStr(
		_impl->ctx,
		uiObj,
		"registerSettings",
		JS_NewCFunction(_impl->ctx, JsRegisterSettings, "registerSettings", 1));
	JS_SetPropertyStr(_impl->ctx, omniObj, "ui", uiObj);

	// omnigram.messages
	const auto msgObj = JS_NewObject(_impl->ctx);
	JS_SetPropertyStr(
		_impl->ctx,
		msgObj,
		"on",
		JS_NewCFunction(_impl->ctx, JsMessagesOn, "on", 2));
	JS_SetPropertyStr(
		_impl->ctx,
		msgObj,
		"onBeforeSend",
		JS_NewCFunction(_impl->ctx, JsMessagesOnBeforeSend, "onBeforeSend", 1));
	JS_SetPropertyStr(_impl->ctx, omniObj, "messages", msgObj);

	// omnigram.storage
	const auto storageObj = JS_NewObject(_impl->ctx);
	JS_SetPropertyStr(
		_impl->ctx,
		storageObj,
		"get",
		JS_NewCFunctionData(_impl->ctx, JsStorageGet, 2, 0, 0, nullptr));
	JS_SetPropertyStr(
		_impl->ctx,
		storageObj,
		"set",
		JS_NewCFunction(_impl->ctx, JsStorageSet, "set", 2));
	JS_SetPropertyStr(
		_impl->ctx,
		storageObj,
		"delete",
		JS_NewCFunction(_impl->ctx, JsStorageDelete, "delete", 1));
	JS_SetPropertyStr(_impl->ctx, omniObj, "storage", storageObj);

	JS_SetPropertyStr(_impl->ctx, global, "omnigram", omniObj);
	JS_FreeValue(_impl->ctx, global);
}

PluginRuntime::~PluginRuntime() = default;

bool PluginRuntime::evaluateFile(const QString &filePath, QString &errorOut) {
	QFile file(filePath);
	if (!file.open(QIODevice::ReadOnly)) {
		errorOut = u"Cannot open entry file: "_q + filePath;
		return false;
	}
	const auto isTs = filePath.endsWith(u".ts"_q, Qt::CaseInsensitive);
	const auto code = PreprocessScript(QString::fromUtf8(file.readAll()), isTs);
	const auto utf8 = code.toUtf8();
	const auto filenameUtf8 = filePath.toUtf8();

	const auto res = JS_Eval(
		_impl->ctx,
		utf8.constData(),
		utf8.size(),
		filenameUtf8.constData(),
		JS_EVAL_TYPE_GLOBAL);
	if (JS_IsException(res)) {
		errorOut = DumpException(_impl->ctx);
		LOG(("Omnigram Plugin Eval Error (%1): %2").arg(filePath, errorOut));
		JS_FreeValue(_impl->ctx, res);
		return false;
	}
	JS_FreeValue(_impl->ctx, res);
	return true;
}

void PluginRuntime::invokeButtonCallback(const QString &key) {
	const auto it = _impl->buttonCallbacks.find(key);
	if (it == _impl->buttonCallbacks.end() || !_impl->ctx) {
		return;
	}
	const auto ret = JS_Call(_impl->ctx, it->second, JS_UNDEFINED, 0, nullptr);
	if (JS_IsException(ret)) {
		LOG(("Omnigram Plugin Button Error: %1").arg(DumpException(_impl->ctx)));
	}
	JS_FreeValue(_impl->ctx, ret);
}

} // namespace Omnigram::Plugins

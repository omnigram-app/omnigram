/*
This file is part of Omnigram,
an advanced fork of Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/omnigram-app/omnigram/blob/main/LICENSE
*/
#include "omnigram/plugins/plugin_settings_ui.h"
#include "omnigram/plugins/plugin.h"
#include "omnigram/plugins/plugin_manager.h"

#include "base/flat_map.h"
#include "settings/settings_common.h"
#include "settings/settings_common_session.h"
#include "ui/layers/generic_box.h"
#include "ui/vertical_list.h"
#include "ui/widgets/buttons.h"
#include "ui/widgets/checkbox.h"
#include "ui/widgets/fields/input_field.h"
#include "ui/widgets/labels.h"
#include "ui/wrap/slide_wrap.h"
#include "ui/wrap/vertical_layout.h"
#include "window/window_session_controller.h"
#include "lang/lang_keys.h"
#include "styles/style_boxes.h"
#include "styles/style_layers.h"
#include "styles/style_settings.h"

namespace Omnigram::Plugins {
namespace {

class PluginSection final : public ::Settings::AbstractSection {
public:
	PluginSection(
		QWidget *parent,
		not_null<Window::SessionController*> controller,
		QString pluginId);

	[[nodiscard]] ::Settings::Type id() const override;
	[[nodiscard]] rpl::producer<QString> title() override;

private:
	void setupContent();

	QString _pluginId;
};

class PluginSectionFactory final : public ::Settings::AbstractSectionFactory {
public:
	explicit PluginSectionFactory(QString pluginId)
	: _pluginId(std::move(pluginId)) {
	}

	[[nodiscard]] object_ptr<::Settings::AbstractSection> create(
		not_null<QWidget*> parent,
		not_null<Window::SessionController*> controller,
		not_null<Ui::ScrollArea*> scroll,
		rpl::producer<::Settings::Container> containerValue
	) const override {
		return object_ptr<PluginSection>(parent, controller, _pluginId);
	}

private:
	QString _pluginId;
};

PluginSection::PluginSection(
	QWidget *parent,
	not_null<Window::SessionController*> controller,
	QString pluginId)
: AbstractSection(parent, controller)
, _pluginId(std::move(pluginId)) {
	setupContent();
}

::Settings::Type PluginSection::id() const {
	return PluginSectionId(_pluginId);
}

rpl::producer<QString> PluginSection::title() {
	if (const auto plugin = PluginManager::Instance().findPlugin(_pluginId)) {
		return rpl::single(plugin->manifest().name);
	}
	return rpl::single(_pluginId);
}

void PluginSection::setupContent() {
	const auto content = Ui::CreateChild<Ui::VerticalLayout>(this);
	if (const auto plugin = PluginManager::Instance().findPlugin(_pluginId)) {
		BuildPluginSettingsUI(controller(), content, plugin);
	} else {
		content->add(
			object_ptr<Ui::FlatLabel>(
				content,
				u"Plugin not found."_q,
				st::settingsScaleLabel),
			st::settingsSendTypePadding);
	}
	Ui::ResizeFitChild(this, content);
}

void OpenSelectBox(
		not_null<Window::SessionController*> controller,
		const PluginSettingSchema &schema,
		const QString &currentValue,
		Fn<void(const QString&)> onSelect) {
	controller->show(Box([=](not_null<Ui::GenericBox*> box) {
		box->setTitle(rpl::single(schema.label));

		auto initialIndex = 0;
		for (auto i = 0; i < schema.options.size(); ++i) {
			if (schema.options[i] == currentValue) {
				initialIndex = i;
				break;
			}
		}

		const auto group = std::make_shared<Ui::RadiobuttonGroup>(initialIndex);
		const auto container = box->verticalLayout();
		for (auto i = 0; i < schema.options.size(); ++i) {
			container->add(
				object_ptr<Ui::Radiobutton>(
					container,
					group,
					i,
					schema.options[i],
					st::defaultBoxCheckbox),
				st::boxOptionListPadding);
		}

		group->setChangedCallback([=](int value) {
			if (value >= 0 && value < schema.options.size()) {
				onSelect(schema.options[value]);
			}
			box->closeBox();
		});

		box->addButton(tr::lng_cancel(), [=] { box->closeBox(); });
	}));
}

} // namespace

::Settings::Type PluginSectionId(const QString &pluginId) {
	static base::flat_map<QString, ::Settings::Type> kFactories;
	const auto i = kFactories.find(pluginId);
	if (i != end(kFactories)) {
		return i->second;
	}
	auto factory = std::make_shared<PluginSectionFactory>(pluginId);
	kFactories.emplace(pluginId, factory);
	return factory;
}

void BuildPluginSettingsUI(
		not_null<Window::SessionController*> controller,
		not_null<Ui::VerticalLayout*> container,
		not_null<Plugin*> plugin) {
	const auto &m = plugin->manifest();

	Ui::AddSubsectionTitle(container, rpl::single(m.name + u" v"_q + m.version));

	if (!m.description.isEmpty() || !m.author.isEmpty()) {
		auto infoText = m.description;
		if (!m.author.isEmpty()) {
			if (!infoText.isEmpty()) {
				infoText += u"\n"_q;
			}
			infoText += u"Author: "_q + m.author;
		}
		container->add(
			object_ptr<Ui::FlatLabel>(
				container,
				infoText,
				st::settingsScaleLabel),
			st::settingsSendTypePadding);
		Ui::AddSkip(container, st::settingsCheckboxesSkip);
	}

	if (m.settings.empty()) {
		container->add(
			object_ptr<Ui::FlatLabel>(
				container,
				u"This plugin does not declare any configurable settings."_q,
				st::settingsScaleLabel),
			st::settingsSendTypePadding);
		return;
	}

	for (const auto &schema : m.settings) {
		if (schema.type == u"boolean"_q) {
			const auto current = plugin->settingValue(schema.key).toBool();
			const auto wrap = container->add(
				object_ptr<Ui::SlideWrap<Ui::Checkbox>>(
					container,
					object_ptr<Ui::Checkbox>(
						container,
						schema.label,
						current,
						st::settingsCheckbox),
					st::settingsSendTypePadding));
			wrap->entity()->checkedChanges(
			) | rpl::on_next([=](bool checked) {
				plugin->setSettingValue(schema.key, checked);
			}, wrap->lifetime());
		} else if (schema.type == u"string"_q) {
			container->add(
				object_ptr<Ui::FlatLabel>(
					container,
					schema.label,
					st::settingsScaleLabel),
				st::settingsSendTypePadding);
			const auto current = plugin->settingValue(schema.key).toString();
			const auto field = container->add(
				object_ptr<Ui::InputField>(
					container,
					st::defaultInputField,
					Ui::InputField::Mode::SingleLine,
					rpl::single(schema.description.isEmpty()
						? schema.label
						: schema.description),
					current),
				st::settingsSendTypePadding);
			field->changes() | rpl::on_next([=] {
				plugin->setSettingValue(schema.key, field->getLastText());
			}, field->lifetime());
		} else if (schema.type == u"select"_q) {
			const auto current = plugin->settingValue(schema.key).toString();
			const auto labelText = container->lifetime().make_state<rpl::variable<QString>>(
				schema.label + u": "_q + (current.isEmpty() && !schema.options.isEmpty()
					? schema.options.front()
					: current));
			const auto btn = container->add(
				object_ptr<Ui::SettingsButton>(
					container,
					labelText->value(),
					st::settingsButton));
			btn->setClickedCallback([=] {
				const auto val = plugin->settingValue(schema.key).toString();
				OpenSelectBox(controller, schema, val, [=](const QString &chosen) {
					plugin->setSettingValue(schema.key, chosen);
					*labelText = schema.label + u": "_q + chosen;
				});
			});
		} else if (schema.type == u"button"_q) {
			const auto btn = container->add(
				object_ptr<Ui::SettingsButton>(
					container,
					rpl::single(schema.label),
					st::settingsButton));
			btn->setClickedCallback([=] {
				plugin->setSettingValue(schema.key, QVariant(true));
			});
		}

		if (!schema.description.isEmpty() && schema.type != u"string"_q) {
			container->add(
				object_ptr<Ui::FlatLabel>(
					container,
					schema.description,
					st::settingsScaleLabel),
				st::settingsSendTypePadding);
		}
	}

	Ui::AddSkip(container, st::settingsCheckboxesSkip);
	Ui::AddDivider(container);
	Ui::AddSkip(container, st::settingsCheckboxesSkip);

	const auto resetBtn = container->add(
		object_ptr<Ui::SettingsButton>(
			container,
			rpl::single(u"Reset plugin settings"_q),
			st::settingsAttentionButton));
	resetBtn->setClickedCallback([=] {
		plugin->resetSettings();
	});

	Ui::AddSkip(container, st::settingsCheckboxesSkip);
}

void OpenPluginSettings(
		not_null<Window::SessionController*> controller,
		not_null<Plugin*> plugin) {
	if (plugin->manifest().standaloneSettingsTab) {
		controller->showSettings(PluginSectionId(plugin->manifest().id));
		return;
	}
	controller->show(Box([=](not_null<Ui::GenericBox*> box) {
		box->setTitle(rpl::single(plugin->manifest().name));
		BuildPluginSettingsUI(controller, box->verticalLayout(), plugin);
		box->addButton(tr::lng_close(), [=] { box->closeBox(); });
	}));
}

} // namespace Omnigram::Plugins

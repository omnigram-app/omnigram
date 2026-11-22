/*
This file is part of Omnigram,
an advanced fork of Telegram Desktop porting macOS aesthetics.

For license and copyright information please follow this link:
https://github.com/omnigram-app/omnigram/blob/main/LICENSE
*/
#include "omnigram/omnigram_settings_section.h"
#include "omnigram/omnigram_settings.h"
#include "omnigram/plugins/plugin_manager.h"
#include "chat_helpers/tabbed_panel.h"
#include "core/file_utilities.h"
#include "settings/settings_common.h"
#include "settings/settings_common_session.h"
#include "ui/vertical_list.h"
#include "ui/widgets/buttons.h"
#include "ui/widgets/checkbox.h"
#include "ui/wrap/slide_wrap.h"
#include "ui/wrap/vertical_layout.h"
#include "window/window_session_controller.h"
#include "lang/lang_keys.h"
#include "styles/style_settings.h"
#include "styles/style_menu_icons.h"

namespace Omnigram {
namespace {

void AddToggle(
		not_null<Ui::VerticalLayout*> container,
		const QString &title,
		bool initial,
		Fn<void(bool)> onChange) {
	const auto wrap = container->add(
		object_ptr<Ui::SlideWrap<Ui::Checkbox>>(
			container,
			object_ptr<Ui::Checkbox>(
				container,
				title,
				initial,
				st::settingsCheckbox),
			st::settingsSendTypePadding));

	wrap->entity()->checkedChanges(
	) | rpl::on_next([=](bool checked) {
		onChange(checked);
	}, wrap->lifetime());
}

class OmnigramSection : public ::Settings::Section<OmnigramSection> {
public:
	OmnigramSection(
		QWidget *parent,
		not_null<Window::SessionController*> controller);

	[[nodiscard]] rpl::producer<QString> title() override;

private:
	void setupContent();
};

OmnigramSection::OmnigramSection(
	QWidget *parent,
	not_null<Window::SessionController*> controller)
: Section(parent, controller) {
	setupContent();
}

rpl::producer<QString> OmnigramSection::title() {
	return rpl::single(u"Omnigram Settings"_q);
}

void OmnigramSection::setupContent() {
	const auto content = Ui::CreateChild<Ui::VerticalLayout>(this);
	SetupOmnigramSettings(controller(), content);
	Ui::ResizeFitChild(this, content);
}

} // namespace

::Settings::Type OmnigramSectionId() {
	return OmnigramSection::Id();
}

void SetupOmnigramSettings(
		not_null<Window::SessionController*> controller,
		not_null<Ui::VerticalLayout*> container) {
	auto &settings = Settings::Instance();

	Ui::AddSubsectionTitle(container, rpl::single(u"macOS Interface & Physics"_q));

	// Master "Enable All" toggle
	const auto masterWrap = container->add(
		object_ptr<Ui::SlideWrap<Ui::Checkbox>>(
			container,
			object_ptr<Ui::Checkbox>(
				container,
				u"Всё (Enable All macOS Features)"_q,
				settings.macOSBubbles()
					&& settings.macOSAnimations()
					&& settings.macOSInputBar()
					&& settings.macOSChatList()
					&& settings.macOSSidebar(),
				st::settingsCheckbox),
			st::settingsSendTypePadding));
	const auto masterCheckbox = masterWrap->entity();

	struct ChildToggles {
		Ui::Checkbox *bubbles = nullptr;
		Ui::Checkbox *animations = nullptr;
		Ui::Checkbox *input = nullptr;
		Ui::Checkbox *chatList = nullptr;
		Ui::Checkbox *sidebar = nullptr;
		bool updatingFromMaster = false;
	};
	const auto toggles = container->lifetime().make_state<ChildToggles>();

	const auto updateMasterState = [=] {
		if (toggles->updatingFromMaster) {
			return;
		}
		const auto all = (toggles->bubbles && toggles->bubbles->checked())
			&& (toggles->animations && toggles->animations->checked())
			&& (toggles->input && toggles->input->checked())
			&& (toggles->chatList && toggles->chatList->checked())
			&& (toggles->sidebar && toggles->sidebar->checked());
		if (masterCheckbox->checked() != all) {
			masterCheckbox->setChecked(all);
		}
	};

	masterCheckbox->checkedChanges(
	) | rpl::on_next([=, &settings](bool checked) {
		toggles->updatingFromMaster = true;
		if (toggles->bubbles) toggles->bubbles->setChecked(checked);
		if (toggles->animations) toggles->animations->setChecked(checked);
		if (toggles->input) toggles->input->setChecked(checked);
		if (toggles->chatList) toggles->chatList->setChecked(checked);
		if (toggles->sidebar) toggles->sidebar->setChecked(checked);
		settings.setMacOSBubbles(checked);
		settings.setMacOSAnimations(checked);
		settings.setMacOSInputBar(checked);
		settings.setMacOSChatList(checked);
		settings.setMacOSSidebar(checked);
		toggles->updatingFromMaster = false;
	}, masterWrap->lifetime());

	Ui::AddSkip(container, st::settingsCheckboxesSkip);

	// 1. Bubbles
	const auto bWrap = container->add(
		object_ptr<Ui::SlideWrap<Ui::Checkbox>>(
			container,
			object_ptr<Ui::Checkbox>(
				container,
				u"Message Bubbles (16pt/8pt radii, carved tail)"_q,
				settings.macOSBubbles(),
				st::settingsCheckbox),
			st::settingsSendTypePadding));
	toggles->bubbles = bWrap->entity();
	toggles->bubbles->checkedChanges() | rpl::on_next([=, &settings](bool checked) {
		settings.setMacOSBubbles(checked);
		updateMasterState();
	}, bWrap->lifetime());

	// 2. Animations & Smooth Physics
	const auto aWrap = container->add(
		object_ptr<Ui::SlideWrap<Ui::Checkbox>>(
			container,
			object_ptr<Ui::Checkbox>(
				container,
				u"Spring Physics & Kinetic Smooth Scroll (120fps)"_q,
				settings.macOSAnimations(),
				st::settingsCheckbox),
			st::settingsSendTypePadding));
	toggles->animations = aWrap->entity();
	toggles->animations->checkedChanges() | rpl::on_next([=, &settings](bool checked) {
		settings.setMacOSAnimations(checked);
		updateMasterState();
	}, aWrap->lifetime());

	// 3. Input bar
	const auto iWrap = container->add(
		object_ptr<Ui::SlideWrap<Ui::Checkbox>>(
			container,
			object_ptr<Ui::Checkbox>(
				container,
				u"Liquid Glass Capsule Input Bar (20pt radius)"_q,
				settings.macOSInputBar(),
				st::settingsCheckbox),
			st::settingsSendTypePadding));
	toggles->input = iWrap->entity();
	toggles->input->checkedChanges() | rpl::on_next([=, &settings](bool checked) {
		settings.setMacOSInputBar(checked);
		updateMasterState();
	}, iWrap->lifetime());

	// 4. Chat list
	const auto cWrap = container->add(
		object_ptr<Ui::SlideWrap<Ui::Checkbox>>(
			container,
			object_ptr<Ui::Checkbox>(
				container,
				u"macOS Dialogs List (70pt rows, 50pt avatars, pill badges)"_q,
				settings.macOSChatList(),
				st::settingsCheckbox),
			st::settingsSendTypePadding));
	toggles->chatList = cWrap->entity();
	toggles->chatList->checkedChanges() | rpl::on_next([=, &settings](bool checked) {
		settings.setMacOSChatList(checked);
		updateMasterState();
	}, cWrap->lifetime());

	// 5. Sidebar dock
	const auto sWrap = container->add(
		object_ptr<Ui::SlideWrap<Ui::Checkbox>>(
			container,
			object_ptr<Ui::Checkbox>(
				container,
				u"Floating Folders Dock (28pt glass dock on left)"_q,
				settings.macOSSidebar(),
				st::settingsCheckbox),
			st::settingsSendTypePadding));
	toggles->sidebar = sWrap->entity();
	toggles->sidebar->checkedChanges() | rpl::on_next([=, &settings](bool checked) {
		settings.setMacOSSidebar(checked);
		updateMasterState();
	}, sWrap->lifetime());

	Ui::AddSkip(container, st::settingsCheckboxesSkip);
	Ui::AddDivider(container);
	Ui::AddSkip(container, st::settingsCheckboxesSkip);

	Ui::AddSubsectionTitle(container, rpl::single(u"Omnigram Features"_q));

	// 6. Exact Time with Seconds
	AddToggle(
		container,
		u"Show Exact Message Time (hh:mm:ss)"_q,
		settings.exactTime(),
		[&](bool checked) { settings.setExactTime(checked); });

	// 7. Profile Peer ID & DC
	AddToggle(
		container,
		u"Show Peer ID & Datacenter in Profile"_q,
		settings.showPeerId(),
		[&](bool checked) { settings.setShowPeerId(checked); });

	// 8. No Ads / Remove Sponsored
	AddToggle(
		container,
		u"Block Sponsored Messages (No Ads)"_q,
		settings.noAds(),
		[&](bool checked) { settings.setNoAds(checked); });

	// 9. Remove Client-Side Limits
	AddToggle(
		container,
		u"Unlock Client-Side Limits (Pinned chats, filters, etc.)"_q,
		settings.clientLimitsRemoved(),
		[&](bool checked) { settings.setClientLimitsRemoved(checked); });

	// 10. Open Emoji & Stickers Panel only on click (not hover)
	AddToggle(
		container,
		u"Open Emoji Panel only on click (disable hover pop-up)"_q,
		ChatHelpers::ShowPanelOnClick(),
		[&](bool checked) { ChatHelpers::SetShowPanelOnClick(checked); });

	Ui::AddSkip(container, st::settingsCheckboxesSkip);
	Ui::AddDivider(container);
	Ui::AddSkip(container, st::settingsCheckboxesSkip);

	Ui::AddSubsectionTitle(container, rpl::single(u"Plugins (TypeScript / JS)"_q));

	auto &pluginManager = Plugins::PluginManager::Instance();
	pluginManager.init();

	// Open Plugins Folder button
	const auto openFolderBtn = container->add(
		object_ptr<Ui::SettingsButton>(
			container,
			rpl::single(u"Open Plugins Folder"_q),
			st::settingsButton));

	openFolderBtn->setClickedCallback([&] {
		File::ShowInFolder(pluginManager.pluginsDirectory());
	});

	// Reload Plugins button
	const auto reloadPluginsBtn = container->add(
		object_ptr<Ui::SettingsButton>(
			container,
			rpl::single(u"Reload Plugins"_q),
			st::settingsButton));

	reloadPluginsBtn->setClickedCallback([&] {
		pluginManager.discoverPlugins();
	});

	// List discovered plugins with toggles
	const auto &plugins = pluginManager.plugins();
	if (plugins.empty()) {
		container->add(
			object_ptr<Ui::FlatLabel>(
				container,
				u"No plugins installed yet. Put .ts/.js plugin folders into the plugins directory."_q,
				st::settingsScaleLabel));
	} else {
		for (const auto &plugin : plugins) {
			const auto &m = plugin->manifest();
			AddToggle(
				container,
				m.name + u" (v"_q + m.version + u")"_q,
				plugin->isRunning(),
				[&pluginManager, id = m.id](bool checked) {
					pluginManager.setPluginEnabled(id, checked);
				});
		}
	}

	Ui::AddSkip(container, st::settingsCheckboxesSkip);
	Ui::AddDivider(container);
	Ui::AddSkip(container, st::settingsCheckboxesSkip);

	// Reset button
	const auto resetBtn = container->add(
		object_ptr<Ui::SettingsButton>(
			container,
			rpl::single(u"Reset Omnigram settings to defaults"_q),
			st::settingsAttentionButton));

	resetBtn->setClickedCallback([=, &settings] {
		settings.resetToDefaults();
	});

	Ui::AddSkip(container, st::settingsCheckboxesSkip);
}

} // namespace Omnigram

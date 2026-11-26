/*
This file is part of Omnigram,
an advanced fork of Telegram Desktop porting macOS aesthetics.

For license and copyright information please follow this link:
https://github.com/omnigram-app/omnigram/blob/main/LICENSE
*/
#include "omnigram/omnigram_settings_section.h"
#include "omnigram/omnigram_settings.h"
#include "omnigram/omnigram_streamer_mode.h"
#include "omnigram/plugins/plugin_manager.h"
#include "omnigram/plugins/plugin_settings_ui.h"
#include "chat_helpers/tabbed_panel.h"
#include "core/application.h"
#include "core/file_utilities.h"
#include "data/data_peer.h"
#include "data/data_session.h"
#include "history/history.h"
#include "main/main_session.h"
#include "boxes/peer_list_box.h"
#include "boxes/peer_list_controllers.h"
#include "settings/settings_common.h"
#include "settings/settings_common_session.h"
#include "ui/vertical_list.h"
#include "ui/widgets/buttons.h"
#include "ui/widgets/checkbox.h"
#include "ui/widgets/fields/input_field.h"
#include "ui/wrap/slide_wrap.h"
#include "ui/wrap/vertical_layout.h"
#include "ui/toast/toast.h"
#include "window/main_window.h"
#include "window/window_session_controller.h"
#include "lang/lang_keys.h"
#include "styles/style_boxes.h"
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

void AddInputFieldRow(
		not_null<Ui::VerticalLayout*> container,
		const QString &placeholder,
		const QString &initial,
		Fn<void(const QString&)> onChange) {
	const auto field = container->add(
		object_ptr<Ui::InputField>(
			container,
			st::defaultInputField,
			Ui::InputField::Mode::SingleLine,
			rpl::single(placeholder),
			initial),
		st::settingsSendTypePadding);

	field->changes() | rpl::on_next([=] {
		onChange(field->getLastText());
	}, field->lifetime());
}

void OpenPeerSelector(
		not_null<Window::SessionController*> controller,
		const QString &title,
		const QSet<uint64> &initialSelected,
		Fn<void(QSet<uint64>)> onSave) {
	class CustomChatsController final : public ChatsListBoxController {
	public:
		CustomChatsController(
			not_null<Main::Session*> session,
			const QSet<uint64> &selected)
		: ChatsListBoxController(session)
		, _session(session)
		, _selected(selected) {
		}

		Main::Session &session() const override {
			return *_session;
		}

		void prepareViewHook() override {
			delegate()->peerListSetSearchMode(PeerListSearchMode::Enabled);
			auto peers = std::vector<not_null<PeerData*>>();
			for (const auto id : _selected) {
				if (const auto peer = session().data().peerLoaded(PeerId(id))) {
					peers.push_back(peer);
				}
			}
			if (!peers.empty()) {
				delegate()->peerListAddSelectedPeers(std::move(peers));
			}
		}

		void rowClicked(not_null<PeerListRow*> row) override {
			delegate()->peerListSetRowChecked(row, !row->checked());
		}

	protected:
		std::unique_ptr<Row> createRow(not_null<History*> history) override {
			return std::make_unique<Row>(history);
		}

	private:
		not_null<Main::Session*> _session;
		QSet<uint64> _selected;
	};

	auto listController = std::make_unique<CustomChatsController>(
		&controller->session(),
		initialSelected);
	auto initBox = [=](not_null<PeerListBox*> box) {
		box->setTitle(rpl::single(title));
		box->addButton(tr::lng_settings_save(), [=] {
			const auto peers = box->collectSelectedRows();
			QSet<uint64> result;
			for (const auto &peer : peers) {
				result.insert(peer->id.value);
			}
			onSave(result);
			box->closeBox();
		});
		box->addButton(tr::lng_cancel(), [box] { box->closeBox(); });
	};

	controller->show(
		Box<PeerListBox>(std::move(listController), std::move(initBox)));
}

void SetupStreamerTargetBlock(
		not_null<Window::SessionController*> controller,
		not_null<Ui::VerticalLayout*> container,
		StreamerTarget target,
		const QString &title) {
	auto &streamer = StreamerMode::Instance();
	auto &rule = streamer.rule(target);

	Ui::AddSubsectionTitle(container, rpl::single(title));

	AddToggle(
		container,
		u"Hide Username"_q,
		rule.hideUsername,
		[&](bool checked) {
			rule.hideUsername = checked;
			streamer.save();
		});
	AddInputFieldRow(
		container,
		u"Custom Username replacement (default: *)"_q,
		rule.customUsername,
		[&](const QString &text) {
			rule.customUsername = text.trimmed().isEmpty() ? u"*"_q : text.trimmed();
			streamer.save();
		});

	AddToggle(
		container,
		u"Hide Peer ID"_q,
		rule.hidePeerId,
		[&](bool checked) {
			rule.hidePeerId = checked;
			streamer.save();
		});
	AddInputFieldRow(
		container,
		u"Custom Peer ID replacement (default: *)"_q,
		rule.customPeerId,
		[&](const QString &text) {
			rule.customPeerId = text.trimmed().isEmpty() ? u"*"_q : text.trimmed();
			streamer.save();
		});

	AddToggle(
		container,
		u"Hide Phone Number"_q,
		rule.hidePhone,
		[&](bool checked) {
			rule.hidePhone = checked;
			streamer.save();
		});
	AddInputFieldRow(
		container,
		u"Custom Phone replacement (default: *)"_q,
		rule.customPhone,
		[&](const QString &text) {
			rule.customPhone = text.trimmed().isEmpty() ? u"*"_q : text.trimmed();
			streamer.save();
		});

	AddToggle(
		container,
		u"Hide Display Name"_q,
		rule.hideName,
		[&](bool checked) {
			rule.hideName = checked;
			streamer.save();
		});
	AddInputFieldRow(
		container,
		u"Custom Display Name replacement (default: *)"_q,
		rule.customName,
		[&](const QString &text) {
			rule.customName = text.trimmed().isEmpty() ? u"*"_q : text.trimmed();
			streamer.save();
		});

	AddToggle(
		container,
		u"Hide Avatar (Replace with custom photo)"_q,
		rule.hideAvatar,
		[&](bool checked) {
			rule.hideAvatar = checked;
			streamer.save();
		});

	const auto pickAvatarBtn = container->add(
		object_ptr<Ui::SettingsButton>(
			container,
			rpl::single(rule.customAvatarPath.isEmpty()
				? u"Select Custom Avatar Image..."_q
				: u"Change Custom Avatar ("_q + QFileInfo(rule.customAvatarPath).fileName() + u")"_q),
			st::settingsButton));

	pickAvatarBtn->setClickedCallback([=, &streamer, &rule] {
		FileDialog::GetOpenPath(
			Core::App().getFileDialogParent(),
			u"Choose Custom Avatar Image"_q,
			FileDialog::ImagesFilter(),
			[=, &streamer, &rule](FileDialog::OpenResult &&result) {
				if (!result.paths.isEmpty()) {
					rule.customAvatarPath = result.paths.front();
					streamer.save();
				}
			});
	});

	if (target == StreamerTarget::Selected) {
		const auto selectedCount = rule.selectedPeers.size();
		const auto selectPeersBtn = container->add(
			object_ptr<Ui::SettingsButton>(
				container,
				rpl::single(u"Selected Peers ("_q + QString::number(selectedCount) + u")"_q),
				st::settingsButton));

		selectPeersBtn->setClickedCallback([=, &streamer, &rule] {
			OpenPeerSelector(
				controller,
				u"Select Peers for Streamer Mode"_q,
				rule.selectedPeers,
				[=, &streamer, &rule](QSet<uint64> peers) {
					rule.selectedPeers = peers;
					streamer.save();
				});
		});
	}

	const auto excCount = rule.exceptions.size();
	const auto excBtn = container->add(
		object_ptr<Ui::SettingsButton>(
			container,
			rpl::single(u"Exceptions ("_q + QString::number(excCount) + u")"_q),
			st::settingsButton));

	excBtn->setClickedCallback([=, &streamer, &rule] {
		OpenPeerSelector(
			controller,
			u"Select Exceptions"_q,
			rule.exceptions,
			[=, &streamer, &rule](QSet<uint64> exceptions) {
				rule.exceptions = exceptions;
				streamer.save();
			});
	});

	Ui::AddSkip(container, st::settingsCheckboxesSkip);
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
	auto &streamer = StreamerMode::Instance();

	// --- STREAMER MODE SECTION ---
	Ui::AddSubsectionTitle(container, rpl::single(u"Streamer Mode (Privacy & Anti-Doxx)"_q));

	AddToggle(
		container,
		u"Enable Streamer Mode"_q,
		streamer.enabled(),
		[&](bool checked) {
			streamer.setEnabled(checked);
		});

	SetupStreamerTargetBlock(controller, container, StreamerTarget::Everyone, u"Target: Everyone"_q);
	SetupStreamerTargetBlock(controller, container, StreamerTarget::Self, u"Target: Self (Current Account)"_q);
	SetupStreamerTargetBlock(controller, container, StreamerTarget::Contacts, u"Target: Contacts"_q);
	SetupStreamerTargetBlock(controller, container, StreamerTarget::Selected, u"Target: Selected Specific Peers"_q);

	Ui::AddSkip(container, st::settingsCheckboxesSkip);
	Ui::AddDivider(container);
	Ui::AddSkip(container, st::settingsCheckboxesSkip);

	Ui::AddSubsectionTitle(container, rpl::single(u"Omnigram Features"_q));

	// 1. Exact Time with Seconds
	AddToggle(
		container,
		u"Show Exact Message Time (hh:mm:ss)"_q,
		settings.exactTime(),
		[&](bool checked) { settings.setExactTime(checked); });

	// 2. Profile Peer ID & DC
	AddToggle(
		container,
		u"Show Peer ID & Datacenter in Profile"_q,
		settings.showPeerId(),
		[&](bool checked) { settings.setShowPeerId(checked); });

	// 3. No Ads / Remove Sponsored
	AddToggle(
		container,
		u"Block Sponsored Messages (No Ads)"_q,
		settings.noAds(),
		[&](bool checked) { settings.setNoAds(checked); });

	// 4. Remove Client-Side Limits
	AddToggle(
		container,
		u"Unlock Client-Side Limits (Pinned chats, filters, etc.)"_q,
		settings.clientLimitsRemoved(),
		[&](bool checked) { settings.setClientLimitsRemoved(checked); });

	// 5. Fast Download & Upload Boost
	AddToggle(
		container,
		u"Fast Download & Upload (up to 16 parallel connections)"_q,
		settings.fastDownload(),
		[&](bool checked) { settings.setFastDownload(checked); });

	// 6. Open Emoji & Stickers Panel only on click (not hover)
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
		Ui::Toast::Show(
			u"Plugins reloaded ("_q
				+ QString::number(pluginManager.plugins().size())
				+ u" found)"_q);
	});

	// List discovered plugins with toggles and gear settings button
	const auto &plugins = pluginManager.plugins();
	if (plugins.empty()) {
		container->add(
			object_ptr<Ui::FlatLabel>(
				container,
				u"No plugins installed yet. Put .ts/.js plugin folders into the plugins directory."_q,
				st::settingsScaleLabel),
			st::settingsSendTypePadding);
	} else {
		for (const auto &plugin : plugins) {
			const auto &m = plugin->manifest();
			const auto rawPlugin = plugin.get();
			const auto hasGear = rawPlugin->hasSettings() || m.standaloneSettingsTab;
			if (!hasGear) {
				AddToggle(
					container,
					m.name + u" (v"_q + m.version + u")"_q,
					plugin->isRunning(),
					[&pluginManager, id = m.id](bool checked) {
						pluginManager.setPluginEnabled(id, checked);
					});
				continue;
			}
			const auto row = container->add(
				object_ptr<Ui::RpWidget>(container));
			const auto check = Ui::CreateChild<Ui::Checkbox>(
				row,
				m.name + u" (v"_q + m.version + u")"_q,
				plugin->isRunning(),
				st::settingsCheckbox);
			check->checkedChanges() | rpl::on_next([&pluginManager, id = m.id](bool checked) {
				pluginManager.setPluginEnabled(id, checked);
			}, check->lifetime());

			const auto gear = Ui::CreateChild<Ui::IconButton>(
				row,
				st::sessionTerminate);
			gear->setIconOverride(&st::menuIconSettings, &st::menuIconSettings);
			gear->setClickedCallback([=] {
				Plugins::OpenPluginSettings(controller, rawPlugin);
			});

			row->sizeValue() | rpl::on_next([=](QSize size) {
				const auto pad = st::settingsSendTypePadding;
				const auto gearW = gear->width();
				const auto checkMaxW = std::max(
					0,
					size.width() - pad.left() - pad.right() - gearW - st::settingsCheckboxesSkip);
				check->resizeToWidth(checkMaxW);
				check->moveToLeft(pad.left(), pad.top());
				gear->moveToRight(pad.right(), (size.height() - gear->height()) / 2);
			}, row->lifetime());
			row->resize(
				row->width(),
				std::max(check->height(), gear->height())
					+ st::settingsSendTypePadding.top()
					+ st::settingsSendTypePadding.bottom());
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

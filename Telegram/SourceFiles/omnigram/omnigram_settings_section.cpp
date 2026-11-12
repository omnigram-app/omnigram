/*
This file is part of Omnigram,
an advanced fork of Telegram Desktop porting macOS aesthetics.

For license and copyright information please follow this link:
https://github.com/omnigram-app/omnigram/blob/main/LICENSE
*/
#include "omnigram/omnigram_settings_section.h"
#include "omnigram/omnigram_settings.h"
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
	return rpl::single(u"Omnigram macOS Mode"_q);
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

	Ui::AddSubsectionTitle(container, rpl::single(u"Interface Preset"_q));

	// Master Preset Buttons
	const auto modeLabel = container->lifetime().make_state<rpl::variable<QString>>(
		settings.uiMode() == UIMode::MacOS
			? u"macOS"_q
			: (settings.uiMode() == UIMode::Classic
				? u"Classic"_q
				: u"Custom"_q));

	AddToggle(
		container,
		u"Preset: macOS Interface & Physics"_q,
		settings.uiMode() == UIMode::MacOS,
		[&settings, modeLabel](bool checked) {
			if (checked) {
				settings.setUIMode(UIMode::MacOS);
				*modeLabel = u"macOS"_q;
			}
		});

	AddToggle(
		container,
		u"Preset: Classic Telegram Desktop"_q,
		settings.uiMode() == UIMode::Classic,
		[&settings, modeLabel](bool checked) {
			if (checked) {
				settings.setUIMode(UIMode::Classic);
				*modeLabel = u"Classic"_q;
			}
		});

	AddToggle(
		container,
		u"Preset: Custom Granular Mode"_q,
		settings.uiMode() == UIMode::Custom,
		[&settings, modeLabel](bool checked) {
			if (checked) {
				settings.setUIMode(UIMode::Custom);
				*modeLabel = u"Custom"_q;
			}
		});

	Ui::AddSkip(container, st::settingsCheckboxesSkip);
	Ui::AddDivider(container);
	Ui::AddSkip(container, st::settingsCheckboxesSkip);

	Ui::AddSubsectionTitle(container, rpl::single(u"macOS Elements (Granular)"_q));

	// 1. Bubbles
	AddToggle(
		container,
		u"macOS Message Bubbles (16pt/8pt radii, carved tail)"_q,
		settings.macOSBubbles(),
		[&](bool checked) { settings.setMacOSBubbles(checked); });

	// 2. Animations
	AddToggle(
		container,
		u"macOS Spring Physics & Smooth Slide Transitions"_q,
		settings.macOSAnimations(),
		[&](bool checked) { settings.setMacOSAnimations(checked); });

	// 3. Input bar
	AddToggle(
		container,
		u"macOS Liquid Glass Capsule Input Bar (20pt radius)"_q,
		settings.macOSInputBar(),
		[&](bool checked) { settings.setMacOSInputBar(checked); });

	// 4. Chat list
	AddToggle(
		container,
		u"macOS Dialogs List (70pt rows, 50pt avatars, pill badges)"_q,
		settings.macOSChatList(),
		[&](bool checked) { settings.setMacOSChatList(checked); });

	// 5. Sidebar dock
	AddToggle(
		container,
		u"macOS Floating Folders Dock (28pt glass dock on left)"_q,
		settings.macOSSidebar(),
		[&](bool checked) { settings.setMacOSSidebar(checked); });

	Ui::AddSkip(container, st::settingsCheckboxesSkip);
	Ui::AddDivider(container);
	Ui::AddSkip(container, st::settingsCheckboxesSkip);

	Ui::AddSubsectionTitle(container, rpl::single(u"Omnigram / AyuGram Features"_q));

	// 6. Exact Time with Seconds
	AddToggle(
		container,
		u"Show Exact Message Time (hh:mm:ss)"_q,
		settings.exactTime(),
		[&](bool checked) { settings.setExactTime(checked); });

	// 7. AyuGram Profile Peer ID & DC
	AddToggle(
		container,
		u"Show Peer ID & Datacenter in Profile (AyuGram style)"_q,
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
		*modeLabel = u"macOS"_q;
	});

	Ui::AddSkip(container, st::settingsCheckboxesSkip);
}

} // namespace Omnigram

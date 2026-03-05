// geode header
#include <Geode/Geode.hpp>

#include "ProgressSettingsPopup.hpp"
#include "../menus/RouletteSafeLayer.hpp"
#include "../DPLevels.hpp"
#include "../menus/DPListLayer.hpp"

ProgressSettingsPopup* ProgressSettingsPopup::create(int difficultyIndex) {
	auto popup = new ProgressSettingsPopup();
	popup->difficultyIndex = difficultyIndex;
	if (popup && popup->init()) {
		popup->autorelease();
		return popup;
	}
	CC_SAFE_DELETE(popup);
	return nullptr;
}

bool ProgressSettingsPopup::init() {
	if (!Popup::init(250, 145.f)) return false;

	this->setTitle("Set Required Level Count");

	auto data = Mod::get()->getSavedValue<matjson::Value>("cached-data");
	int defaultLevelRequirement = data["main"][this->difficultyIndex]["reqLevels"].asInt().unwrap();
	this->valueInput = TextInput::create(100, std::to_string(defaultLevelRequirement));
	this->valueInput->setID("required-level-count");
	this->valueInput->setPosition(this->getPosition() + ccp(this->m_mainLayer->getContentWidth() / 2, this->m_mainLayer->getContentHeight() - 50));
	this->m_mainLayer->addChild(this->valueInput);

	auto padding = 100.f;
	auto menu = CCMenu::create();
	menu->setID("require-all-menu"_spr);
	menu->setAnchorPoint({ 0.5, 0.5 });
	menu->setContentSize(this->m_mainLayer->getContentSize() - ccp(padding, 0));
	menu->setPosition(this->m_mainLayer->getContentSize() - menu->getContentSize() / 2 - ccp(padding / 2, -27));

	auto requireAllButton = CCMenuItemSpriteExtra::create(
		ButtonSprite::create(
			"Require All", 
			0, 
			false, 
			"bigFont.fnt", 
			"GJ_button_02.png", 
			20.0f, 
			0.25f
		),
		this,
		menu_selector(ProgressSettingsPopup::requireAll)
	);
	requireAllButton->setAnchorPoint({ 0, 0});
	requireAllButton->setID("require-all-button"_spr);
	menu->addChildAtPosition(requireAllButton, Anchor::BottomLeft, ccp(0, 25));
	
	auto resetButton = CCMenuItemSpriteExtra::create(
		ButtonSprite::create(
			"  Default  ", 
			0, 
			false, 
			"bigFont.fnt", 
			"GJ_button_06.png", 
			20.0f, 
			0.25f
		),
		this,
		menu_selector(ProgressSettingsPopup::reset)
	);
	resetButton->setID("reset-button"_spr);
	resetButton->setAnchorPoint({ 1, 0});
	menu->addChildAtPosition(resetButton, Anchor::BottomRight, ccp(0, 25));

	this->m_mainLayer->addChild(menu);

	auto okButton = CCMenuItemSpriteExtra::create(
		ButtonSprite::create("Ok"),
		this,
		menu_selector(ProgressSettingsPopup::confirm)
	);
	okButton->setID("ok-button"_spr);
	okButton->setAnchorPoint({ 0.5, 1 });
	menu->addChildAtPosition(okButton, Anchor::Bottom, ccp(0, 13));

	return true;
}

void ProgressSettingsPopup::requireAll(CCObject* sender) {
	this->valueInput->setString("All");
}

void ProgressSettingsPopup::reset(CCObject* sender) {
	auto data = Mod::get()->getSavedValue<matjson::Value>("cached-data");
	int defaultLevelRequirement = data["main"][this->difficultyIndex]["reqLevels"].asInt().unwrap();
	this->valueInput->setString(std::to_string(defaultLevelRequirement));
}

void ProgressSettingsPopup::confirm(CCObject* sender) {
	std::string inputText = this->valueInput->getString();
	std::transform(inputText.begin(), inputText.end(), inputText.begin(), [](unsigned char c){ return std::tolower(c); });
	auto data = Mod::get()->getSavedValue<matjson::Value>("cached-data");
	int defaultLevelRequirement = data["main"][this->difficultyIndex]["reqLevels"].asInt().unwrap();
	int amount = inputText == "all" ? -2 : numFromString<int>(this->valueInput->getString()).unwrapOr(defaultLevelRequirement);
	DPLevels::setRequiredLevels(this->difficultyIndex, amount);

	auto dplist = static_cast<DPListLayer*>(this->getParent()->getChildByIndex(0));
	dplist->updateProgressBar();

	this->m_closeBtn->activate();
}
//geode header
#include <Geode/Geode.hpp>

//other headers
#include <Geode/utils/web.hpp>
#include <Geode/utils/JsonValidation.hpp>
#include <Geode/loader/Event.hpp>

#include "DPRemovedLayer.hpp"
#include "../MainListEditor.hpp"
#include "../CustomText.hpp"

DPRemovedLayer* DPRemovedLayer::create(int difficultyIndex) {
	auto removedLayer = new DPRemovedLayer();
	if (removedLayer && removedLayer->init(difficultyIndex)) {
		removedLayer->autorelease();
		return removedLayer;
	}
	CC_SAFE_DELETE(removedLayer); //don't crash if it fails
	return nullptr;
}

bool DPRemovedLayer::init(int difficultyIndex) {
	if (!CCLayer::init()) return false;
	this->difficultyIndex = difficultyIndex;
	Mod::get()->setSavedValue("in-removed-menu", true);

	auto removedLevels = MainListEditor::getRemovedMainListLevels(difficultyIndex);

	auto director = CCDirector::sharedDirector();
	auto size = director->getWinSize();

	auto bg = createLayerBG();
	if (!Mod::get()->getSettingValue<bool>("restore-bg-color")) {
		bg->setColor({ 18, 18, 86 });
	}
	bg->setZOrder(-10);
	bg->setID("bg");
	this->addChild(bg);

	auto lCornerSprite = CCSprite::createWithSpriteFrameName("GJ_sideArt_001.png");
	lCornerSprite->setAnchorPoint({ 0, 0 });
	lCornerSprite->setID("left-corner-sprite");
	this->addChild(lCornerSprite);

	auto rCornerSprite = CCSprite::createWithSpriteFrameName("GJ_sideArt_001.png");
	rCornerSprite->setAnchorPoint({ 1, 0 });
	rCornerSprite->setPosition({ size.width, 0 });
	rCornerSprite->setFlipX(true);
	rCornerSprite->setID("right-corner-sprite");
	this->addChild(rCornerSprite);

	//back button
	auto backSprite = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
	auto backButton = CCMenuItemSpriteExtra::create(backSprite, this, menu_selector(DPRemovedLayer::backButton));
	auto backMenu = CCMenu::create();
	backMenu->addChild(backButton);
	backMenu->setPosition({ 25, size.height - 25 });
	backMenu->setZOrder(2);
	backMenu->setID("back-menu");
	this->addChild(backMenu);

	//info button
	auto infoMenu = CCMenu::create();
	auto infoButton = InfoAlertButton::create("Removed Info", "Here are all default GDDP levels you've removed from this difficulty.", 1.0f);
	infoMenu->setPosition({ 25, 25 });
	infoMenu->setZOrder(2);
	infoMenu->addChild(infoButton);
	infoMenu->setID("info-menu");
	this->addChild(infoMenu);

	//pages menu
	m_pagesMenu = CCMenu::create();
	m_pagesMenu->setPosition({ 0, 0 });
	m_pagesMenu->setID("pages-menu");

	m_left = CCMenuItemSpriteExtra::create(CCSprite::createWithSpriteFrameName("GJ_arrow_03_001.png"), this, menu_selector(DPRemovedLayer::pageLeft));
	m_left->setPosition(24.f, size.height / 2);
	m_left->setVisible(false);
	m_pagesMenu->addChild(m_left);

	auto rightBtnSpr = CCSprite::createWithSpriteFrameName("GJ_arrow_03_001.png");
	rightBtnSpr->setFlipX(true);
	m_right = CCMenuItemSpriteExtra::create(rightBtnSpr, this, menu_selector(DPRemovedLayer::pageRight));
	m_right->setPosition(size.width - 24.0f, size.height / 2);
	m_right->setVisible(false);
	m_pagesMenu->addChild(m_right);

	this->addChild(m_pagesMenu);

	m_list = GJListLayer::create(CustomListView::create(CCArray::create(), BoomListType::Level, 220.0f, 358.0f), "", {194, 114, 62, 255}, 358.0f, 220.0f, 0);
	m_list->setZOrder(2);
	m_list->setPosition(size / 2 - m_list->getContentSize() / 2);
	this->addChild(m_list);

	DPRemovedLayer::loadLevels(0);

	this->setKeyboardEnabled(true);
	this->setKeypadEnabled(true);

	return true;
}

void DPRemovedLayer::loadLevels(int page) {
	this->page = page;
	m_pagesMenu->setVisible(false);

	m_list->m_listView->setVisible(false);
	
	auto data = Mod::get()->getSavedValue<matjson::Value>("cached-data");
	this->levelIDs = MainListEditor::getRemovedMainListLevels(this->difficultyIndex);
	auto begin = levelIDs.begin() + page * 10;
	auto end = levelIDs.begin() + std::min((int) (page + 1) * 10, (int) levelIDs.size() - 1);

	auto pageLevels = std::vector<int>();
	for (int levelIndex = page * 10; levelIndex < levelIDs.size() && levelIndex < (page + 1) * 10; levelIndex++) {
		pageLevels.push_back(levelIDs[levelIndex]);
	}

	auto pageLevelStringIDs = std::vector<std::string>();
	for (auto id : pageLevels) {
		pageLevelStringIDs.push_back(std::to_string(id));
	}

	auto levelManager = GameLevelManager::sharedState();
	levelManager->m_levelManagerDelegate = this;
	auto searchObject = GJSearchObject::create(SearchType::Type19, string::join(pageLevelStringIDs, ","));
	auto storedLevels = levelManager->getStoredOnlineLevels(searchObject->getKey());

	this->updateNoLevelsText(storedLevels);
	if (storedLevels) {
		DPRemovedLayer::loadLevelsFinished(storedLevels, "");
	} else {
		levelManager->getOnlineLevels(searchObject);
	}
}

void DPRemovedLayer::updateNoLevelsText(CCArray* levels) {
	if (!levels || levels->count() == 0) {
		this->noLevelsText = CCLabelBMFont::create("No removed levels", "bigFont.fnt");
		noLevelsText->setID("no-levels-text"_spr);
		noLevelsText->setPosition(this->getContentSize() / 2 + ccp(0, 8));
		noLevelsText->setZOrder(20);
		this->addChild(noLevelsText);
		return;
	}

	if (this->noLevelsText) {
		this->noLevelsText->removeFromParentAndCleanup(true);
		this->noLevelsText = nullptr;
	}
}

void DPRemovedLayer::loadLevelsFinished(CCArray* levels, const char*) {
	m_left->setVisible(this->page > 0);
	m_right->setVisible(levelIDs.size() > (page + 1) * 10);
	m_pagesMenu->setVisible(true);
	m_list->m_listView->setVisible(true);

	auto director = CCDirector::sharedDirector();
	auto size = director->getWinSize();

	if (m_list->getParent() == this) { 
		this->removeChild(m_list); 
	}

	auto data = Mod::get()->getSavedValue<matjson::Value>("cached-data");
	std::string fullName = data["main"][this->difficultyIndex]["name"].asString().unwrapOr("null") + " Demons";

	m_list = GJListLayer::create(
		CustomListView::create(levels, BoomListType::Level, 220.0f, 358.0f), 
		fullName.c_str(), 
		{ 194, 114, 62, 255 }, 
		358.0f, 
		220.0f, 
		0
	);
	m_list->setZOrder(2);
	m_list->setPosition(size / 2 - m_list->getContentSize() / 2);
	this->addChild(m_list);

	//custom pack label
	if (
		(
			Mod::get()->getSettingValue<bool>("custom-pack-text") && 
			DPTextEffects.contains(data["main"][this->difficultyIndex]["saveID"].asString().unwrapOr("null"))
		) && !(
			Mod::get()->getSettingValue<bool>("disable-fancy-bonus-text")
		)
	) {
		auto label = typeinfo_cast<CCLabelBMFont*>(m_list->getChildByID("title"));

		auto customText = CustomText::create(label->getString());
		customText->addEffectsFromProperties(DPTextEffects[data["main"][this->difficultyIndex]["saveID"].asString().unwrapOr("null")].as<matjson::Value>().unwrapOrDefault());
		customText->setPosition(label->getPosition());
		customText->setAnchorPoint(label->getAnchorPoint());
		customText->setScale(label->getScale());
		customText->setZOrder(12);
		customText->setID("custom-pack-title"_spr);

		label->setOpacity(0);
		label->setVisible(false);
				
		m_list->addChild(customText);
	}

	this->updateNoLevelsText(levels);
}

void DPRemovedLayer::pageRight(CCObject*) {
	page += 1;
	loadLevels(page);
}

void DPRemovedLayer::pageLeft(CCObject*) {
	page -= 1;
	loadLevels(page);
}

void DPRemovedLayer::backButton(CCObject*) {
	DPRemovedLayer::keyBackClicked();
}

void DPRemovedLayer::keyBackClicked() {
	CCDirector::sharedDirector()->popSceneWithTransition(0.5f, PopTransition::kPopTransitionFade);
}

DPRemovedLayer::~DPRemovedLayer() {
	Mod::get()->setSavedValue("in-removed-menu", false);
    this->removeAllChildrenWithCleanup(true);
}
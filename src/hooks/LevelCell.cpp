//geode header
#include <Geode/Geode.hpp>

#include <Geode/utils/JsonValidation.hpp>
#include <Geode/modify/LevelCell.hpp>
#include "../menus/DPLayer.hpp"
#include "../CustomText.hpp"
#include "../MainListEditor.hpp"
#include "../menus/DPListLayer.hpp"
#include "../DPUtils.hpp"
#include "../LevelDownloader.hpp"

//geode namespace
using namespace geode::prelude;

struct MainListData : public CCObject {
	std::vector<int> defaultLevelIDs;

    MainListData(std::vector<int> defaultIDs) : defaultLevelIDs(defaultIDs) {
        // Always remember to call autorelease on your classes!
        this->autorelease();
    }
};

//modify level cells
class $modify(DemonProgression, LevelCell) {

	static void onModify(auto & self) {
		static_cast<void>(self.setHookPriority("LevelCell::loadCustomLevelCell", -42));
	}

	void skillInfoPopup(CCObject* target) {
		auto btn = static_cast<CCMenuItemSpriteExtra*>(target);
		auto skillID = btn->getID();
		
		auto skillsetData = Mod::get()->getSavedValue<matjson::Value>("skillset-info");

		FLAlertLayer::create(
			skillsetData[skillID]["display-name"].asString().unwrapOr("null").c_str(),
			skillsetData[skillID]["description"].asString().unwrapOr("erm that\'s awkward").c_str(),
			"OK"
		)->show();

		return;
	}

	void loadCustomLevelCell() {
		LevelCell::loadCustomLevelCell();

		// Get data
		auto data = Mod::get()->getSavedValue<matjson::Value>("cached-data");
		auto jsonCheck = checkJson(data, "");
		if (!jsonCheck.ok()) {
			log::info("Something went wrong validating the GDDP list data.");
			return;
		}

		bool inGDDP = Mod::get()->getSavedValue<bool>("in-gddp");
		bool showOutsideGDDP = Mod::get()->getSettingValue<bool>("show-outside-menus");
		bool showOnlyMainList = Mod::get()->getSettingValue<bool>("show-only-main-list-faces");
		auto difficultyIndex = Mod::get()->getSavedValue<int>("current-difficulty-index", 0);

		// Check if this is a progression level
		auto difficulties = MainListEditor::getDifficultyPacks(this->m_level->m_levelID);
		auto inMainList = !difficulties.empty();
		auto inDefaultGDDP = data["level-data"].contains(std::to_string(this->m_level->m_levelID.value()));
		if (showOnlyMainList) inDefaultGDDP = MainListEditor::isInDefaultGDDPMainList(this->m_level->m_levelID);
		auto renderCustomGDDPLevel = (inGDDP || showOutsideGDDP) && ((inDefaultGDDP && inGDDP) || inMainList);
		if (!renderCustomGDDPLevel) return;

		// If not in a GDDP menu, the difficulty pack should just be the hardest one that level belongs to.
		if (!inGDDP && inMainList) difficultyIndex = difficulties[difficulties.size() - 1];

		if (this->isOnlyInMonthlyPack() && !inMainList) return;
		if (this->isOnlyInBonus() && !inMainList) return;

		// More definitions
		auto type = Mod::get()->getSavedValue<std::string>("current-pack-type", "main");
		auto id = Mod::get()->getSavedValue<int>("current-pack-index", 0);
		std::string saveID = type == "main" && !data["main"][id]["saveID"].isNull() ? data["main"][id]["saveID"].asString().unwrapOr("null") : "null";
		auto hasRank = Mod::get()->getSavedValue<ListSaveFormat>(saveID).hasRank;

		this->renderSkillsetBadges();
		this->renderEditControls(difficultyIndex, type, id, inGDDP);
		this->renderFancyLevelName(difficultyIndex);
		this->renderCustomDifficultyFace(difficultyIndex, type, hasRank);
	}

	void renderEditControls(int difficultyIndex, std::string type, int id, bool inGDDP) {
		if (type != "main" || !Mod::get()->getSettingValue<bool>("enable-main-list-editing") || !inGDDP) return;

		auto data = Mod::get()->getSavedValue<matjson::Value>("cached-data");

		auto viewButtonNode = typeinfo_cast<CCNode*>(this
			->getChildByID("main-layer")
			->getChildByID("main-menu")
			->getChildByID("view-button")
		);

		auto inRemovedList = Mod::get()->getSavedValue<bool>("in-removed-menu", false);
		
		auto removeMenu = CCMenu::create();
		removeMenu->setID("remove-menu"_spr);
		removeMenu->setContentSize(ccp(41, 24));
		if (viewButtonNode) {
			removeMenu->setPosition(viewButtonNode->getPosition() + ccp(12, viewButtonNode->getContentHeight() - 60));
			removeMenu->setAnchorPoint(viewButtonNode->getAnchorPoint());
		}

		auto removeButton = CCMenuItemSpriteExtra::create(
			ButtonSprite::create(
				inRemovedList ? "Add Back" : "Remove", 
				0, 
				false, 
				"bigFont.fnt", 
				inRemovedList ? "GJ_button_02.png" : "GJ_button_06.png", 
				23.0f, 
				0.2f
			),
			this,
			inRemovedList ? menu_selector(DemonProgression::openUnremoveLevelPopup) : menu_selector(DemonProgression::openRemoveMainListLevelPopup)
		);

		auto levelIDs = std::vector<int>();
		for (auto id : data["main"][id]["levelIDs"].asArray().unwrap()) {
			levelIDs.push_back(id.asInt().unwrap());
		}

		removeButton->setTag(difficultyIndex);
		removeButton->setID("remove-button"_spr);
		removeMenu->addChild(removeButton);

		this->getChildByID("main-layer")->getChildByID("main-menu")->addChild(removeMenu);
	}

	void renderSkillsetBadges() {
		auto data = Mod::get()->getSavedValue<matjson::Value>("cached-data");
		auto skillsetData = Mod::get()->getSavedValue<matjson::Value>("skillset-info", matjson::makeObject({
			{"unknown", matjson::makeObject({
					{ "display-name", "Unknown" },
					{ "description", "This skill does not have a description." },
					{ "sprite", "DP_Skill_Unknown" }
				})
			}
		}));

		//check for errors
		auto jsonCheck2 = checkJson(skillsetData, "");

		if (!jsonCheck2.ok()) {
			log::info("Something went wrong validating the skillset data.");

			return;
		}

		std::vector<std::string> skillsets = {};
		auto levelID = std::to_string(this->m_level->m_levelID.value());
		if (data["level-data"].contains(levelID)) {
			if (!data["level-data"][levelID]["skillsets"].isNull()) { skillsets = data["level-data"][levelID]["skillsets"].as<std::vector<std::string>>().unwrapOrDefault(); }

			if (this->m_level->m_normalPercent.value() == 100) {
				auto completedLvls = Mod::get()->getSavedValue<std::vector<int>>("completed-levels");

				if (std::find(completedLvls.begin(), completedLvls.end(), this->m_level->m_levelID.value()) == completedLvls.end()) {
					completedLvls.insert(completedLvls.begin(), this->m_level->m_levelID.value());
					Mod::get()->setSavedValue<std::vector<int>>("completed-levels", completedLvls);
				}
			}
		}

		if (Mod::get()->getSettingValue<bool>("skillset-badges") && skillsets.size() > 0 && Mod::get()->getSettingValue<bool>("show-skills-in-list")) {
			auto layer = typeinfo_cast<CCNode*>(this->getChildByID("main-layer"));

			GJDifficultySprite* diffSpr;
			if (this->getChildByID("main-layer")->getChildByID("grd-demon-icon-layer")) {
				diffSpr = typeinfo_cast<GJDifficultySprite*>(layer->getChildByID("grd-demon-icon-layer")->getChildByID("difficulty-sprite"));
			}
			else if (this->getChildByID("main-layer")->getChildByID("difficulty-container")) {
				diffSpr = typeinfo_cast<GJDifficultySprite*>(layer->getChildByID("difficulty-container")->getChildByID("difficulty-sprite"));
			}

			//create the skillset menu
			auto skillMenu = CCMenu::create();
			auto skillLayout = AxisLayout::create();
			skillLayout->setAxis(Axis::Column);
			skillLayout->setAxisReverse(true);
			skillMenu->setLayout(skillLayout, true, false);
			skillMenu->setID("skillset-menu"_spr);
			if (layer->getChildByID("level-place")) {
				if (!layer->getChildByID("level-place")->isVisible()) {
					skillMenu->setPosition({ 25.f, -12.f });
				}
				else {
					skillMenu->setPosition({ 45.f, -12.f });
				}
				skillMenu->setScale(0.4f);
			}
			else {
				skillMenu->setPosition({ diffSpr->getPositionX() + 32, diffSpr->getPositionY() + 6 });
				skillMenu->setScale(0.5f);
			}
			skillMenu->setZOrder(42);
			skillMenu->setContentSize({ 31.5f, 90.0f });
			if (skillsets.size() >= 3) {
				skillMenu->setAnchorPoint({ 0.7f, 0.5f });
			}
			else {
				skillMenu->setAnchorPoint({ 0.5f, 0.5f });
			}
			
			//add skillset buttons
			for (int i = 0; i < skillsets.size(); i++) {

				std::string skillID = skillsets[i];

				//check data entry
				if (!skillsetData.contains(skillID)) {
					skillID = "unknown";
				}

				//get data
				auto name = skillsetData[skillID]["display-name"].asString().unwrapOr("null");
				auto desc = skillsetData[skillID]["description"].asString().unwrapOr("erm that\'s awkward");
				auto spriteName = fmt::format("{}.png", skillsetData[skillID]["sprite"].asString().unwrapOr("DP_Skill_Unknown"));
				
				CCSprite* sprite;
				if (CCSprite::createWithSpriteFrameName(Mod::get()->expandSpriteName(spriteName).data()) == nullptr) {
					spriteName = fmt::format("{}.png", skillsetData["unknown"]["sprite"].asString().unwrapOr("DP_Skill_Unknown"));
					sprite = CCSprite::createWithSpriteFrameName(Mod::get()->expandSpriteName(spriteName).data());
				}
				else {
					sprite = CCSprite::createWithSpriteFrameName(Mod::get()->expandSpriteName(spriteName).data());
				}

				auto skillsetBtn = CCMenuItemSpriteExtra::create(sprite, this, menu_selector(DemonProgression::skillInfoPopup));
				skillsetBtn->setID(skillID);
				skillMenu->addChild(skillsetBtn);
			}

			skillMenu->updateLayout(false);
			layer->addChild(skillMenu);
		}
	}

	bool isOnlyInMonthlyPack() {
		if (!Mod::get()->getSettingValue<bool>("hide-monthly-outside") || Mod::get()->getSavedValue<bool>("in-gddp")) return false;
		auto data = Mod::get()->getSavedValue<matjson::Value>("cached-data");

		//check monthly, if check returns with nothing, skip the rest
		auto isMonthly = false;
		for (int i = 0; i < data["monthly"].as<std::vector<matjson::Value>>().unwrapOr(std::vector<matjson::Value>()).size(); i++) {
			auto monthlyPack = data["monthly"][i]["levelIDs"].as<std::vector<int>>().unwrapOrDefault();
			if (std::find(monthlyPack.begin(), monthlyPack.end(), this->m_level->m_levelID.value()) != monthlyPack.end()) {
				isMonthly = true;
				break;
			}	
		}
		
		if (isMonthly) {
			auto uniqueMonthly = true; //false = level is in main/legacy/bonus, so don't return if false

			//check main
			for (int i = 0; i < data["main"].as<std::vector<matjson::Value>>().unwrapOr(std::vector<matjson::Value>()).size(); i++) {
				auto pack = data["main"][i]["levelIDs"].as<std::vector<int>>().unwrapOrDefault();
				if (std::find(pack.begin(), pack.end(), this->m_level->m_levelID.value()) != pack.end()) {
					uniqueMonthly = false;
					break;
				}
			}

			//check legacy
			if (uniqueMonthly) {
				for (int i = 0; i < data["legacy"].as<std::vector<matjson::Value>>().unwrapOr(std::vector<matjson::Value>()).size(); i++) {
					auto pack = data["legacy"][i]["levelIDs"].as<std::vector<int>>().unwrapOrDefault();
					if (std::find(pack.begin(), pack.end(), this->m_level->m_levelID.value()) != pack.end()) {
						uniqueMonthly = false;
						break;
					}
				}
			}

			//check bonus
			if (uniqueMonthly) {
				for (int i = 0; i < data["bonus"].as<std::vector<matjson::Value>>().unwrapOr(std::vector<matjson::Value>()).size(); i++) {
					auto pack = data["bonus"][i]["levelIDs"].as<std::vector<int>>().unwrapOrDefault();
					if (std::find(pack.begin(), pack.end(), this->m_level->m_levelID.value()) != pack.end()) {
						uniqueMonthly = false;
						break;
					}
				}
			}

			if (uniqueMonthly) { 
				return true;
			}
		}

		return false;
	}

	bool isOnlyInBonus() {
		if (!Mod::get()->getSettingValue<bool>("hide-bonus-outside") || Mod::get()->getSavedValue<bool>("in-gddp")) return false;
		auto data = Mod::get()->getSavedValue<matjson::Value>("cached-data");

		//check bonus, if check returns with nothing, skip the rest
		auto isBonus = false;
		for (int i = 0; i < data["bonus"].as<std::vector<matjson::Value>>().unwrapOr(std::vector<matjson::Value>()).size(); i++) {
			auto bonusPack = data["bonus"][i]["levelIDs"].as<std::vector<int>>().unwrapOrDefault();
			if (std::find(bonusPack.begin(), bonusPack.end(), this->m_level->m_levelID.value()) != bonusPack.end()) {
				isBonus = true;
				break;
			}	
		}
		
		if (isBonus) {
			auto uniqueBonus = true; //false = level is in main/legacy, so don't return if false

			//check main
			for (int i = 0; i < data["main"].as<std::vector<matjson::Value>>().unwrapOr(std::vector<matjson::Value>()).size(); i++) {
				auto pack = data["main"][i]["levelIDs"].as<std::vector<int>>().unwrapOrDefault();
				if (std::find(pack.begin(), pack.end(), this->m_level->m_levelID.value()) != pack.end()) {
					uniqueBonus = false;
					break;
				}
			}

			//check legacy
			if (uniqueBonus) {
				for (int i = 0; i < data["legacy"].as<std::vector<matjson::Value>>().unwrapOr(std::vector<matjson::Value>()).size(); i++) {
					auto pack = data["legacy"][i]["levelIDs"].as<std::vector<int>>().unwrapOrDefault();
					if (std::find(pack.begin(), pack.end(), this->m_level->m_levelID.value()) != pack.end()) {
						uniqueBonus = false;
						break;
					}
				}
			}

			if (uniqueBonus) {
				return true;
			}
		}

		return false;
	}

	void renderFancyLevelName(int difficultyIndex) {
		if (!Mod::get()->getSettingValue<bool>("custom-level-name") || !this->getChildByID("main-layer")->getChildByID("level-name")) return;

		auto data = Mod::get()->getSavedValue<matjson::Value>("cached-data");

		auto lvlName = typeinfo_cast<CCLabelBMFont*>(this->getChildByID("main-layer")->getChildByID("level-name"));
		auto customLvlName = CustomText::create(lvlName->getString());
		customLvlName->addEffectsFromProperties(DPTextEffects[data["main"][difficultyIndex]["saveID"].asString().unwrapOr("none")].as<matjson::Value>().unwrapOrDefault());
		customLvlName->setPosition(lvlName->getPosition());
		customLvlName->setAnchorPoint(lvlName->getAnchorPoint());
		customLvlName->setScale(lvlName->getScale());
		customLvlName->setZOrder(5);
		customLvlName->setID("custom-level-name"_spr);

		lvlName->setOpacity(0);
		lvlName->setVisible(false);
		
		this->getChildByID("main-layer")->addChild(customLvlName);
	}

	void renderCustomDifficultyFace(int difficultyIndex, std::string type, bool hasRank) {
		if (!Mod::get()->getSettingValue<bool>("custom-difficulty-faces")) return;

		auto data = Mod::get()->getSavedValue<matjson::Value>("cached-data");

		// Grandpa Demon
		if (this->getChildByID("main-layer")->getChildByID("grd-demon-icon-layer") && Mod::get()->getSettingValue<bool>("override-grandpa-demon")) {
			auto diffIcon = typeinfo_cast<GJDifficultySprite*>(this->getChildByID("main-layer")->getChildByID("grd-demon-icon-layer")->getChildByID("difficulty-sprite"));
			auto layer = typeinfo_cast<CCNode*>(this->getChildByID("main-layer")->getChildByID("grd-demon-icon-layer"));

			diffIcon->setVisible(false);

			//find and identify the grandpa demon icons
			for (int i = 0; i < layer->getChildrenCount(); i++) {
				if (layer->getChildByType<CCSprite>(i)) {
					if (!(layer->getChildByType<CCSprite>(i)->getID() != "") && layer->getChildByType<CCSprite>(i)->getTag() != 69420) {
						layer->getChildByType<CCSprite>(i)->setID("grd-difficulty-face");
					}
				}
			}

			if (layer->getChildByID("grd-infinity")) { layer->getChildByID("grd-infinity")->setVisible(false); }
			
			if (layer->getChildByTag(69420)) {
				if ((this->m_level->m_isEpic == 1) && Mod::get()->getSettingValue<bool>("replace-epic")) { layer->getChildByTag(69420)->setVisible(false); }
			}

			if (layer->getChildByID("grd-difficulty-face")) {
				layer->getChildByID("grd-difficulty-face")->removeMeAndCleanup();
			}

			std::string sprite = "DP_Unknown";
			std::string plusSprite = "DP_Unknown";

			/*if (Mod::get()->getSettingValue<bool>("all-demons-rated") && !data["level-data"].contains(std::to_string(this->m_level->m_levelID.value()))) {
				sprite = ListManager::getSpriteName(this->m_level);
				plusSprite = fmt::format("{}Plus", sprite);
			}*/
			sprite = data["main"][difficultyIndex]["sprite"].asString().unwrapOr("DP_Unknown");
			plusSprite = data["main"][difficultyIndex]["plusSprite"].asString().unwrapOr("DP_Unknown");
			
			//fallbacks
			if (CCSprite::createWithSpriteFrameName(Mod::get()->expandSpriteName(fmt::format("{}.png", sprite)).data()) == nullptr) {
				sprite = "DP_Unknown";
			}

			if (CCSprite::createWithSpriteFrameName(Mod::get()->expandSpriteName(fmt::format("{}.png", plusSprite)).data()) == nullptr) {
				plusSprite = "DP_Unknown";
			}

			std::string fullSpr = fmt::format("{}.png", sprite);
			std::string fullPlusSpr = fmt::format("{}.png", plusSprite);

			if (sprite != "DP_Invisible") {
				auto customSpr = CCSprite::createWithSpriteFrameName(Mod::get()->expandSpriteName(fullSpr).data());

				if (this->m_level->m_isEpic == 1 && Mod::get()->getSettingValue<bool>("replace-epic") && plusSprite != "DP_Invisible") {
					typeinfo_cast<CCSprite*>(diffIcon->getChildren()->objectAtIndex(0))->setVisible(false);
					customSpr = CCSprite::createWithSpriteFrameName(Mod::get()->expandSpriteName(fullPlusSpr).data());
				}

				if (Mod::get()->getSettingValue<bool>("override-ratings") && type == "main" && hasRank && plusSprite != "DP_Invisible") {
					customSpr = CCSprite::createWithSpriteFrameName(Mod::get()->expandSpriteName(fullPlusSpr).data());
				}

				customSpr->setID("gddp-difficulty");
				customSpr->setAnchorPoint({ 0.5f, 0.5f });
				customSpr->setPosition({ diffIcon->getPositionX(), diffIcon->getPositionY() + 6.f });
				customSpr->setZOrder(5);

				layer->addChild(customSpr);
				
				//label
				auto demonLabel = CCLabelBMFont::create("Demon", "bigFont.fnt");
				auto customDemonLabel = CustomText::create("Demon");

				demonLabel->setID("demon-label"_spr);
				demonLabel->setAnchorPoint({ 0.5f, 0.5f });
				demonLabel->setPosition({ diffIcon->getPositionX(), diffIcon->getPositionY() - 16.f });
				demonLabel->setScale(0.35f);
				demonLabel->setZOrder(5);

				customDemonLabel->setID("custom-demon-label"_spr);
				customDemonLabel->setAnchorPoint({ 0.5f, 0.5f });
				customDemonLabel->setPosition({ diffIcon->getPositionX(), diffIcon->getPositionY() - 16.f });
				customDemonLabel->setScale(0.35f);
				customDemonLabel->setZOrder(5);

				customDemonLabel->addEffectsFromProperties(DPTextEffects[data["main"][difficultyIndex]["saveID"].asString().unwrapOr("none")].as<matjson::Value>().unwrapOrDefault());

				if (Mod::get()->getSettingValue<bool>("custom-demon-labels")) {
					layer->addChild(customDemonLabel);
				} 
				else {
					layer->addChild(demonLabel);
				}

				//check if the level is recommended and the effect is enabled
				auto recommendations = Mod::get()->getSavedValue<std::vector<int>>("recommended-levels");
				if ((!Mod::get()->getSettingValue<bool>("disable-recommended-effect")) && Mod::get()->getSettingValue<bool>("enable-recommendations") && std::find(recommendations.begin(), recommendations.end(), this->m_level->m_levelID.value()) != recommendations.end()) {
					auto recommendedSpr = CCSprite::createWithSpriteFrameName("DP_RecommendGlow.png"_spr);
					recommendedSpr->setAnchorPoint({ 0.f, 0.f });
					recommendedSpr->setZOrder(6);
					customSpr->addChild(recommendedSpr);
				}
			}
		}

		//typical list layer
		else if (this->getChildByID("main-layer")->getChildByID("difficulty-container")) {
			auto diffIcon = typeinfo_cast<GJDifficultySprite*>(this->getChildByID("main-layer")->getChildByID("difficulty-container")->getChildByID("difficulty-sprite"));
			auto layer = typeinfo_cast<CCNode*>(this->getChildByID("main-layer")->getChildByID("difficulty-container"));
			typeinfo_cast<GJDifficultySprite*>(layer->getChildByID("difficulty-sprite"))->setOpacity(0);

			std::string sprite = "DP_Unknown";
			std::string plusSprite = "DP_Unknown";

			/*if (Mod::get()->getSettingValue<bool>("all-demons-rated") && !data["level-data"].contains(std::to_string(this->m_level->m_levelID.value()))) {
				sprite = ListManager::getSpriteName(this->m_level);
				plusSprite = fmt::format("{}Plus", sprite);
			}*/
			sprite = data["main"][difficultyIndex]["sprite"].asString().unwrapOr("DP_Unknown");
			plusSprite = data["main"][difficultyIndex]["plusSprite"].asString().unwrapOr("DP_Unknown");

			//fallbacks
			if (CCSprite::createWithSpriteFrameName(Mod::get()->expandSpriteName(fmt::format("{}.png", sprite)).data()) == nullptr) {
				sprite = "DP_Unknown";
			}

			if (CCSprite::createWithSpriteFrameName(Mod::get()->expandSpriteName(fmt::format("{}.png", plusSprite)).data()) == nullptr) {
				plusSprite = "DP_Unknown";
			}

			std::string fullSpr = fmt::format("{}.png", sprite);
			std::string fullPlusSpr = fmt::format("{}.png", plusSprite);

			if (sprite != "DP_Invisible") {
				auto customSpr = CCSprite::createWithSpriteFrameName(Mod::get()->expandSpriteName(fullSpr).data());

				if (this->m_level->m_isEpic == 1 && Mod::get()->getSettingValue<bool>("replace-epic") && plusSprite != "DP_Invisible") {
					typeinfo_cast<CCSprite*>(diffIcon->getChildren()->objectAtIndex(0))->setVisible(false);
					customSpr = CCSprite::createWithSpriteFrameName(Mod::get()->expandSpriteName(fullPlusSpr).data());
				}

				if (Mod::get()->getSettingValue<bool>("override-ratings") && type == "main" && hasRank && plusSprite != "DP_Invisible") {
					customSpr = CCSprite::createWithSpriteFrameName(Mod::get()->expandSpriteName(fullPlusSpr).data());
				}

				customSpr->setID("gddp-difficulty");
				customSpr->setAnchorPoint({ 0.5f, 0.5f });
				customSpr->setPosition({ diffIcon->getPositionX(), diffIcon->getPositionY() + 6.f });
				customSpr->setZOrder(5);

				layer->addChild(customSpr);
				
				//label
				auto demonLabel = CCLabelBMFont::create("Demon", "bigFont.fnt");
				auto customDemonLabel = CustomText::create("Demon");

				demonLabel->setID("demon-label"_spr);
				demonLabel->setAnchorPoint({ 0.5f, 0.5f });
				demonLabel->setPosition({ diffIcon->getPositionX(), diffIcon->getPositionY() - 16.f });
				demonLabel->setScale(0.35f);
				demonLabel->setZOrder(5);

				customDemonLabel->setID("custom-demon-label"_spr);
				customDemonLabel->setAnchorPoint({ 0.5f, 0.5f });
				customDemonLabel->setPosition({ diffIcon->getPositionX(), diffIcon->getPositionY() - 16.f });
				customDemonLabel->setScale(0.35f);
				customDemonLabel->setZOrder(5);

				customDemonLabel->addEffectsFromProperties(DPTextEffects[data["main"][difficultyIndex]["saveID"].asString().unwrapOr("none")].as<matjson::Value>().unwrapOrDefault());

				if (Mod::get()->getSettingValue<bool>("custom-demon-labels")) {
					layer->addChild(customDemonLabel);
				} 
				else {
					layer->addChild(demonLabel);
				}

				//check if the level is recommended and the effect is enabled
				auto recommendations = Mod::get()->getSavedValue<std::vector<int>>("recommended-levels");
				if ((!Mod::get()->getSettingValue<bool>("disable-recommended-effect")) && Mod::get()->getSettingValue<bool>("enable-recommendations") && std::find(recommendations.begin(), recommendations.end(), this->m_level->m_levelID.value()) != recommendations.end()) {
					auto recommendedSpr = CCSprite::createWithSpriteFrameName("DP_RecommendGlow.png"_spr);
					recommendedSpr->setAnchorPoint({ 0.f, 0.f });
					recommendedSpr->setZOrder(6);
					customSpr->addChild(recommendedSpr);
				}
			}
		}
	}

	/**
	 * Removes this level cell, while adjusting the position of surrounding cells to make up for the empty
	 * space left behind. All children of this cell are cleaned up. This is called when the level is removed
	 * from the list under the `enable-main-list-editing` setting.
	 */
	void removeSelf() {
		auto children = this->getParent()->getChildren();
		int thisIndex = children->indexOfObject(this);

		auto dpListLayer = static_cast<DPListLayer*>(
			this
			->getParent()
			->getParent()
			->getParent()
			->getParent()
			->getParent()
		);
		auto levels = dpListLayer->getLevels();
		auto index = DPUtils::vectorIndexOf<int>(levels, this->m_level->m_levelID);
		auto indexToAdd = (index + 10) - (index % 10);

		// because we're about to remove a level, we need to *add* the first level from the next page
		// (if there is one), so that the page still has 10 levels
		if (indexToAdd < levels.size()) {
			auto levelToAdd = levels[indexToAdd];
			getLevel(levelToAdd, [this, &levels, dpListLayer](GJGameLevel* level) {
				auto parent = this->getParent();
				auto cellHeight = this->getContentHeight();

				// create the new level cell
				auto cell = LevelCell::create(this->m_width, this->m_height);
				cell->loadFromLevel(level);
				cell->setContentSize(this->getContentSize());
				parent->addChild(cell);

				// remove this level cell
				this->removeFromParentAndCleanup(true);
				dpListLayer->removeLevel(this->m_level->m_levelID);
				this->updateParent(parent, cellHeight);
			});
		} 
		
		// no more levels to fill the space - page just gets shorter
		else {
			auto parent = this->getParent();
			auto cellHeight = this->getContentHeight();
			this->removeFromParentAndCleanup(true);
			dpListLayer->removeLevel(this->m_level->m_levelID);
			this->updateParent(parent, cellHeight);
			parent->setContentHeight(parent->getContentHeight() - cellHeight);
			parent->setPositionY(parent->getPositionY() + cellHeight);
		}
	}

	void updateParent(CCNode* parent, int cellHeight) {
		// reposition all level cells to account for the removed cell
		auto siblings = parent->getChildren();
		int indexFromBottom = 0;
		for (int index = siblings->count() - 1; index >= 0; index--, indexFromBottom++) {
			auto levelCell = static_cast<LevelCell*>(siblings->objectAtIndex(index));
			levelCell->setPosition({ levelCell->getPositionX(), (float) indexFromBottom * cellHeight });
			levelCell->updateBGColor(index);
		}
	}

	/**
	 * Called when the "remove" button is pressed, to open a popup to remove a level
	 * from the default GDDP under the `enable-main-list-editing` setting. This expects 
	 * the tag of the button (sender) to be the difficulty index to remove the level from.
	 * 
	 * @param sender The "remove" button.
	 */
	void openRemoveMainListLevelPopup(CCObject* sender) {
		geode::createQuickPopup(
			"Remove level?",
			"Are you sure you want to remove this level from your main list progression?",
			"Cancel", "Ok",
			[this, sender](auto, bool confirmed) {
				if (confirmed) {
					MainListEditor::removeMainListLevel(sender->getTag(), this->m_level->m_levelID);
					this->removeSelf();
				}
			}
		);
	}

	/**
	 * Called when the "add back" button is pressed, to open a popup to "unremove" a level
	 * from the default GDDP that was removed from the main list under the `enable-main-list-editing`
	 * setting. This expects the tag of the button (sender) to be the difficulty index to remove the
	 * level from.
	 * 
	 * @param sender The "add back" button.
	 */
	void openUnremoveLevelPopup(CCObject* sender) {
		geode::createQuickPopup(
			"Add level back?",
			"Are you sure you want to add this level back into your main list progression?",
			"Cancel", "Ok",
			[this, sender](auto, bool confirmed) {
				if (confirmed) {
					log::info("unremoving level...");
					MainListEditor::unremoveMainListLevel(sender->getTag(), this->m_level->m_levelID);
					this->removeSelf();
				}
			}
		);
	}
};
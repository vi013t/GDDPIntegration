//geode header
#include <Geode/Geode.hpp>

#include <Geode/loader/Event.hpp>
#include <Geode/utils/web.hpp>

#include "MainListEditor.hpp"
#include "DPUtils.hpp"

//geode namespace
using namespace geode::prelude;

// define these! the mod will crash without these declarations
std::map<int, std::vector<int>> MainListEditor::mainListLevels;
std::map<int, int> MainListEditor::requiredLevels;

std::vector<int> MainListEditor::getMainListLevels(int difficultyIndex) {
    const auto data = Mod::get()->getSavedValue<matjson::Value>("cached-data");
	if (!Mod::get()->getSettingValue<bool>("enable-main-list-editing")) {
		auto values = data["main"][difficultyIndex]["levelIDs"].asArray().unwrap();
		auto levelIDs = std::vector<int>();
		for (auto id : values) {
			int levelID = id.asInt().unwrap();
			DPUtils::addToVectorIfAbsent(levelIDs, levelID);
		}
		return levelIDs;
	}

	// Check if the levels for this difficulty are already loaded
	if (MainListEditor::mainListLevels.contains(difficultyIndex)) {
		return MainListEditor::mainListLevels[difficultyIndex];
	}

	// No values loaded yet - check if any are saved to storage
	auto storageKey = fmt::format("main-list-{}", difficultyIndex);
	if (Mod::get()->hasSavedValue(storageKey)) {
		auto levels = Mod::get()->getSavedValue<std::vector<int>>(storageKey);
		MainListEditor::mainListLevels[difficultyIndex] = levels;
		return levels;
	}

	// None saved to storage - get them normally
	auto levels = data["main"][difficultyIndex]["levelIDs"].as<std::vector<int>>().unwrapOrDefault();
	MainListEditor::mainListLevels[difficultyIndex] = levels;
	MainListEditor::saveMainListDifficulty(difficultyIndex);
	return levels;
}

std::vector<int> MainListEditor::getAllMainListLevels() {
	auto data = Mod::get()->getSavedValue<matjson::Value>("cached-data");
	auto mainList = data["main"].asArray().unwrap();
	auto levels = std::vector<int>();
	for (int difficulty = 0; difficulty < mainList.size(); difficulty++)	 {
		auto difficultyLevels = MainListEditor::getMainListLevels(difficulty);
		for (auto levelID : difficultyLevels) {
			DPUtils::addToVectorIfAbsent(levels, levelID);
		}
	}

	return levels;
}

bool MainListEditor::isLevelInDifficulty(int levelID, int difficulty) {
	auto data = Mod::get()->getSavedValue<matjson::Value>("cached-data");
	auto levels = getMainListLevels(difficulty);
	return DPUtils::isInVector(levels, levelID);
}

void MainListEditor::addMainListLevel(int difficultyIndex, int levelID) {
	if (!Mod::get()->getSettingValue<bool>("enable-main-list-editing")) {
		return;
	}

	if (!MainListEditor::mainListLevels.contains(difficultyIndex)) {
		MainListEditor::mainListLevels[difficultyIndex] = std::vector<int>();
	}

	// Check if level is already present for this difficulty
	auto& levels = MainListEditor::mainListLevels[difficultyIndex];
	if (DPUtils::isInVector(levels, levelID)) {
		return;
	}

	// If we are trying to "add" a default level (that was removed), use the unremove() function instead
	auto defaultIDs = Mod::get()->getSavedValue<matjson::Value>("cached-data")["main"][difficultyIndex]["levelIDs"];
	for (auto id : defaultIDs.asArray().unwrap()) {
		if (id.asInt().unwrap() == levelID) {
			unremoveMainListLevel(difficultyIndex, levelID);
			return;
		}
	}

	// otherwise, just add it
	levels.push_back(levelID);
	saveMainListDifficulty(difficultyIndex);
}

void MainListEditor::saveMainListDifficulty(int difficultyIndex) {
	Mod::get()->setSavedValue<std::vector<int>>(fmt::format("main-list-{}", difficultyIndex), MainListEditor::mainListLevels[difficultyIndex]);
}

void MainListEditor::removeMainListLevel(int difficultyIndex, int levelID) {
	if (!Mod::get()->getSettingValue<bool>("enable-main-list-editing")) {
		return;
	}

	if (!MainListEditor::mainListLevels.contains(difficultyIndex)) {
		getMainListLevels(difficultyIndex);
	}

	auto ids = Mod::get()->getSavedValue<matjson::Value>("cached-data")["main"][difficultyIndex]["levelIDs"];
	auto defaultIDs = std::vector<int>();
	for (auto id : ids.asArray().unwrap()) {
		defaultIDs.push_back(id.asInt().unwrap());
	}

	auto& difficultyList = MainListEditor::mainListLevels[difficultyIndex];
	auto levelIndex = std::find(difficultyList.begin(), difficultyList.end(), levelID);
	if (levelIndex != difficultyList.end()) {
		MainListEditor::mainListLevels[difficultyIndex].erase(levelIndex);
		saveMainListDifficulty(difficultyIndex);

		// store this level as one of the "removed" ones if its part of the default GDDP
		if (std::find(defaultIDs.begin(), defaultIDs.end(), levelID) != defaultIDs.end()) {
			auto removedKey = fmt::format("removed-{}", difficultyIndex);
			auto removedLevels = std::vector<int>();
			if (Mod::get()->hasSavedValue(removedKey)) {
				removedLevels = Mod::get()->getSavedValue<std::vector<int>>(removedKey);
			}
			if (!DPUtils::isInVector(removedLevels, levelID)) {
				removedLevels.push_back(levelID);
				Mod::get()->setSavedValue(removedKey, removedLevels);
			}
		}
	}
}

std::vector<int> MainListEditor::getRemovedMainListLevels(int difficultyIndex) {
	auto removedKey = fmt::format("removed-{}", difficultyIndex);
	if (Mod::get()->hasSavedValue(removedKey)) {
		return Mod::get()->getSavedValue<std::vector<int>>(removedKey);
	}
	return std::vector<int>();
}

void MainListEditor::unremoveMainListLevel(int difficultyIndex, int levelID) {
	auto removedLevels = MainListEditor::getRemovedMainListLevels(difficultyIndex);

	auto index = std::find(removedLevels.begin(), removedLevels.end(), levelID);
	if (index == removedLevels.end()) {
		return;
	}

	removedLevels.erase(index);
	auto removedKey = fmt::format("removed-{}", difficultyIndex);
	Mod::get()->setSavedValue(removedKey, removedLevels);

	auto savedIndex = std::find(mainListLevels[difficultyIndex].begin(), mainListLevels[difficultyIndex].end(), levelID);
	if (savedIndex != mainListLevels[difficultyIndex].end()) {
		return;
	}

	mainListLevels[difficultyIndex].push_back(levelID);
	saveMainListDifficulty(difficultyIndex);
}

void MainListEditor::setRequiredLevels(int difficultyIndex, int amount) {
	MainListEditor::requiredLevels[difficultyIndex] = amount;
	Mod::get()->setSavedValue(fmt::format("required-{}", difficultyIndex), amount);
}

int MainListEditor::getRequiredLevels(int difficultyIndex) {
	auto data = Mod::get()->getSavedValue<matjson::Value>("cached-data");
	int amount = data["main"][difficultyIndex]["reqLevels"].asInt().unwrap();
	if (!Mod::get()->getSettingValue<bool>("enable-main-list-editing")) {
		return amount;
	} 

	if (MainListEditor::requiredLevels.contains(difficultyIndex)) {
		return MainListEditor::requiredLevels[difficultyIndex];
	}

	std::string storageKey = fmt::format("required-{}", difficultyIndex);
	if (Mod::get()->hasSavedValue(storageKey)) {
		return Mod::get()->getSavedValue<int>(storageKey);
	}

	setRequiredLevels(difficultyIndex, amount);
	return amount;
}

std::vector<int> MainListEditor::getDifficultyPacks(int levelID) {
	auto data = Mod::get()->getSavedValue<matjson::Value>("cached-data");
	auto difficulties = data["main"].asArray().unwrap().size();
	auto levelDifficulties = std::vector<int>();
	for (int difficulty = 0; difficulty < difficulties; difficulty++) {
		if (MainListEditor::isLevelInDifficulty(levelID, difficulty)) {
			levelDifficulties.push_back(difficulty);
		}
	}
	return levelDifficulties;
}

bool MainListEditor::isInDefaultGDDPMainList(int levelID) { 
	auto data = Mod::get()->getSavedValue<matjson::Value>("cached-data");
	for (auto object : data["main"].asArray().unwrap()) {
		auto levels = DPUtils::vectorMap<int>(object["levelIDs"].asArray().unwrap(), [](matjson::Value id) { return (int) id.asInt().unwrap(); });
		if (DPUtils::isInVector(levels, levelID)) {
			return true;
		}
	}
	return false;
}
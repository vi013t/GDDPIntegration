//geode header
#include <Geode/Geode.hpp>

#include <Geode/loader/Event.hpp>
#include <Geode/utils/web.hpp>

#include "DPLevels.hpp"
#include "DPUtils.hpp"

//geode namespace
using namespace geode::prelude;

std::map<int, std::vector<int>> DPLevels::mainListLevels;
std::map<int, int> DPLevels::requiredLevels;

/**
 * Returns a list of level IDs saved for a given difficulty.
 * 
 * @param difficultyIndex The index of the difficulty
 */
std::vector<int> DPLevels::getMainListLevels(int difficultyIndex) {
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
	if (DPLevels::mainListLevels.contains(difficultyIndex)) {
		return DPLevels::mainListLevels[difficultyIndex];
	}

	// No values loaded yet - check if any are saved to storage
	auto storageKey = fmt::format("main-list-{}", difficultyIndex);
	if (Mod::get()->hasSavedValue(storageKey)) {
		auto levels = Mod::get()->getSavedValue<std::vector<int>>(storageKey);
		DPLevels::mainListLevels[difficultyIndex] = levels;
		return levels;
	}

	// None saved to storage - get them normally
	auto levels = data["main"][difficultyIndex]["levelIDs"].as<std::vector<int>>().unwrapOrDefault();
	DPLevels::mainListLevels[difficultyIndex] = levels;
	DPLevels::saveMainListDifficulty(difficultyIndex);
	return levels;
}

std::vector<int> DPLevels::getAllMainListLevels() {
	auto data = Mod::get()->getSavedValue<matjson::Value>("cached-data");
	auto mainList = data["main"].asArray().unwrap();
	auto levels = std::vector<int>();
	for (int difficulty = 0; difficulty < mainList.size(); difficulty++)	 {
		auto difficultyLevels = DPLevels::getMainListLevels(difficulty);
		for (auto levelID : difficultyLevels) {
			DPUtils::addToVectorIfAbsent(levels, levelID);
		}
	}

	return levels;
}

bool DPLevels::isLevelInDifficulty(int levelID, int difficulty) {
	auto data = Mod::get()->getSavedValue<matjson::Value>("cached-data");
	auto levels = getMainListLevels(difficulty);
	return std::find(levels.begin(), levels.end(), levelID) != levels.end();
}

/**
 * Adds a level to the given difficulty. If the level is already present for this difficulty,
 * a duplicate will not be added.
 * 
 * @param difficultyIndex The index of the difficulty
 * @param levelID The ID of the level to add
 */
void DPLevels::addMainListLevel(int difficultyIndex, int levelID) {
	if (!DPLevels::mainListLevels.contains(difficultyIndex)) {
		DPLevels::mainListLevels[difficultyIndex] = std::vector<int>();
	}

	// Check if level is already present for this difficulty
	auto& levels = DPLevels::mainListLevels[difficultyIndex];
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

/**
 * Saves a difficulty list to persistent storage.
 * 
 * @param difficultyIndex The index of the difficulty
 */
void DPLevels::saveMainListDifficulty(int difficultyIndex) {
	Mod::get()->setSavedValue<std::vector<int>>(fmt::format("main-list-{}", difficultyIndex), DPLevels::mainListLevels[difficultyIndex]);
}

/**
 * Removes a level from a difficulty list. The changes are saved to persistent storage.
 * 
 * @param difficultyIndex The index of the difficulty
 * @param levelID The ID of the level to remove
 */
void DPLevels::removeMainListLevel(int difficultyIndex, int levelID) {
	if (!DPLevels::mainListLevels.contains(difficultyIndex)) {
		getMainListLevels(difficultyIndex);
	}

	auto ids = Mod::get()->getSavedValue<matjson::Value>("cached-data")["main"][difficultyIndex]["levelIDs"];
	auto defaultIDs = std::vector<int>();
	for (auto id : ids.asArray().unwrap()) {
		defaultIDs.push_back(id.asInt().unwrap());
	}

	auto& difficultyList = DPLevels::mainListLevels[difficultyIndex];
	auto levelIndex = std::find(difficultyList.begin(), difficultyList.end(), levelID);
	if (levelIndex != difficultyList.end()) {
		DPLevels::mainListLevels[difficultyIndex].erase(levelIndex);
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

std::vector<int> DPLevels::getRemovedMainListLevels(int difficultyIndex) {
	auto removedKey = fmt::format("removed-{}", difficultyIndex);
	if (Mod::get()->hasSavedValue(removedKey)) {
		return Mod::get()->getSavedValue<std::vector<int>>(removedKey);
	}
	return std::vector<int>();
}

void DPLevels::unremoveMainListLevel(int difficultyIndex, int levelID) {
	auto removedLevels = DPLevels::getRemovedMainListLevels(difficultyIndex);

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

int DPLevels::difficultyIndexOfLevel(int levelID) {
	auto data = Mod::get()->getSavedValue<matjson::Value>("cached-data");
	int difficulties = data["main"].size();
	for (int difficulty = 0; difficulty < difficulties; difficulty++) {
		auto levels = getMainListLevels(difficulty);
		if (std::find(levels.begin(), levels.end(), levelID) != levels.end()) {
			return difficulty;
		}
	}

	return -1;
}

void DPLevels::setRequiredLevels(int difficultyIndex, int amount) {
	DPLevels::requiredLevels[difficultyIndex] = amount;
	Mod::get()->setSavedValue(fmt::format("required-{}", difficultyIndex), amount);
}

int DPLevels::getRequiredLevels(int difficultyIndex) {
	auto data = Mod::get()->getSavedValue<matjson::Value>("cached-data");
	int amount = data["main"][difficultyIndex]["reqLevels"].asInt().unwrap();
	if (!Mod::get()->getSettingValue<bool>("enable-main-list-editing")) {
		return amount;
	} 

	if (DPLevels::requiredLevels.contains(difficultyIndex)) {
		return DPLevels::requiredLevels[difficultyIndex];
	}

	std::string storageKey = fmt::format("required-{}", difficultyIndex);
	if (Mod::get()->hasSavedValue(storageKey)) {
		return Mod::get()->getSavedValue<int>(storageKey);
	}

	setRequiredLevels(difficultyIndex, amount);
	return amount;
}
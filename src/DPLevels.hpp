#pragma once

#include <Geode/Geode.hpp>

//geode namespace
using namespace geode::prelude;

class DPLevels {
private:
	static std::map<int, std::vector<int>> mainListLevels;
public:
	static std::vector<int> getMainListLevels(int difficultyIndex, matjson::Value fallbackData);
	static void addMainListLevel(int difficultyIndex, int levelID);
	static void saveMainListDifficulty(int difficultyIndex);
	static void removeMainListLevel(int difficultyIndex, int levelID);
	static std::vector<int> getRemovedMainListLevels(int difficultyIndex);
	static void unremoveMainListLevel(int difficultyIndex, int levelID);
	static int difficultyIndexOfLevel(int levelID, matjson::Value defaultData);
	static bool isLevelInDifficulty(int levelID, int difficulty);
};
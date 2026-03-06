#pragma once

#include <Geode/Geode.hpp>

//geode namespace
using namespace geode::prelude;

/**
 * Class for managing main list editing under the `enable-main-list-editing` flag.
 */
class MainListEditor {

private:

	/**
	 * A map where the keys are difficulty pack indices, and the values are lists of level IDs present in
	 * that difficulty pack. Note that packs that have not been changed/fetched may not have mappings here;
	 * Use MainListEditor::getMainListLevels() or MainListEditor::getAllMainListLevels() to access this safely.
	 */
	static std::map<int, std::vector<int>> mainListLevels;

	/**
	 * A map where the keys are difficulty pack indices, and the values are the number of required levels
	 * for that pack. Note that packs that have not had their required levels changed may not have present
	 * mappings here; Use MainListEditor::getRequiredLevels() to access this safely.
	 */
	static std::map<int, int> requiredLevels;

public:

	/**
	* Returns the level IDs for a difficulty pack. This factors in the default GDDP levels, as
	* well as any user changes like added/removed levels.
	* 
	* @param difficultyIndex The index of the difficulty
	*
	* @return A vector of level IDs that are in the difficulty pack.
	*/
	static std::vector<int> getMainListLevels(int difficultyIndex);

	/**
	 * Returns all level IDs in the main list. This factors in the default GDDP levels, as
	 * well as any user changes like added/removed levels.
	 *
	 * @return All level IDs in the main list.
	 */
	static std::vector<int> getAllMainListLevels();

	/**
	 * Adds a level to a main list difficulty pack. The change is stored in persistent mod
	 * storage, meaning it will persist between restarts. If the level is already in the pack,
	 * nothing will be done (no duplicate added). If the `enable-main-list-editing` setting is
	 * set to `false`, nothing will be done. If the given level is a default GDDP level that
	 * has been removed, unremoveMainListLevel() will be called.
	 *
	 * @param difficultyIndex The difficulty pack index
	 * @param levelID The ID of the level to add
	 */
	static void addMainListLevel(int difficultyIndex, int levelID);

	/**
	 * Saves a difficulty pack to persistent mod storage. This is called after a difficulty pack
	 * changes, such as adding/removing levels.
	 *
	 * @param difficultyIndex The index of the difficulty pack
	 */
	static void saveMainListDifficulty(int difficultyIndex);

	/**
	 * Removes a level from a main list difficulty pack. The change is stored in persistent mod
	 * storage, meaning it will persist between restarts. If the level is not in the pack,
	 * nothing will be done (no error). If the `enable-main-list-editing` setting is
	 * set to `false`, nothing will be done.
	 *
	 * @param difficultyIndex The difficulty pack index
	 * @param levelID The ID of the level to add
	 */
	static void removeMainListLevel(int difficultyIndex, int levelID);

	/**
	 * Gets a list of all levels that are in a difficulty pack in the default GDDP, but have been removed
	 * from that pack by the user. This is used to display the screen of removed levels that can be added
	 * back into the pack. If the level exists in another pack but not this one, it will still be present
	 * in the returned vector.
	 *
	 * @param difficultyIndex The difficulty pack index
	 *
	 * @return A vector of level IDs of all default GDDP levels removed from the pack
	 */
	static std::vector<int> getRemovedMainListLevels(int difficultyIndex);

	/**
	 * Restores a default GDDP level that was removed from a difficulty pack. This adds the level to the
	 * pack, similar to addMainListLevel(), but also removes the level from the stored "removed levels"
	 * cache, which is used to display the default GDDP levels that have been removed from a difficulty
	 * pack and can be re-added. If the level is not a default GDDP level or wasn't removed from this
	 * specific difficulty pack, nothing will be done (no error). The change is stored in persistent mod
	 * storage, meaning it will persist between restarts.
	 *
	 * @param difficultyIndex The difficulty pack index
	 * @param levelID The level ID to unremove
	 */
	static void unremoveMainListLevel(int difficultyIndex, int levelID);

	/**
	 * Returns whether a level is part of a given main list difficulty pack. This factors in the default 
	 * GDDP levels, as well as any user changes like added/removed levels. 
	 *
	 * @param difficultyIndex The difficulty pack index
	 * @param levelID The level ID to check for
	 *
	 * @return Whether the given level is present in the given difficulty pack
	 */
	static bool isLevelInDifficulty(int levelID, int difficulty);

	/**
	 * Sets the number of required levels for a main list difficulty pack. This is the number that appears
	 * on the progress bar. If the `enable-main-list-editing` setting is set to `false`, nothing will be done.
	 * The change is stored in persistent mod storage, meaning it will persist between restarts.
	 * 
	 * @param difficultyIndex The difficulty pack index
	 * @param amount The required level amount
	 */
	static void setRequiredLevels(int difficultyIndex, int amount);

	/**
	 * Returns the number of required levels for a main list difficulty pack. This factors in the default 
	 * pack requirements, as well as any overrides the user has made. 
	 *
	 * @param difficultyIndex The difficulty pack index
	 *
	 * @return The amount of required levels for that pack
	 */
	static int getRequiredLevels(int difficultyIndex);

	/**
	 * Returns a list of the main list difficulty packs that a level belongs to. Normally levels can only
	 * belong to one difficulty pack (or none), but with the `enable-main-list-editing` setting enabled,
	 * levels can belong to multiple difficulty packs.
	 *
	 * @param levelID The level ID to get the difficulty packs of. If this is not a valid level ID, an
	 * empty vector will be returned.
	 *
	 * @return A list of the main list difficulty packs that the given level is in.
	 */
	static std::vector<int> getDifficultyPacks(int levelID);
	static bool isInDefaultGDDPMainList(int levelID);
};
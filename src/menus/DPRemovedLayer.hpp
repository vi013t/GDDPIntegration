#pragma once

#include <Geode/Geode.hpp>
#include <Geode/utils/web.hpp>
#include <Geode/loader/Event.hpp>

//geode namespace
using namespace geode::prelude;

/**
 * The removed levels screen shown when clicking the trash can in the bottom right
 * of a main list difficulty pack. This screen shows all levels that are present
 * in the default GDDP but have been removed by the user under the `enable-main-list-editing`
 * setting.
 */
class DPRemovedLayer : public CCLayer, LevelManagerDelegate {
private:
	GJListLayer* m_list;
	LoadingCircle* m_loadCircle;
	CCMenu* m_pagesMenu;
	CCMenuItemSpriteExtra* m_left;
	CCMenuItemSpriteExtra* m_right;

	/**
	 * The difficulty pack index that this screen is for.
	 */
	int difficultyIndex;

	/**
	 * The zero-indexed page number that the screen is currently on.
	 */
	int page;

	/**
	 * The levels shown on this page that have been removed.
	 */
	std::vector<int> levelIDs;

	/**
	 * The text that says "no levels removed" if no levels have been removed. This is
	 * updated with DPRemovedLayer::updateNoLevelsText();
	 */
	CCLabelBMFont* noLevelsText = nullptr;

protected:
	virtual bool init(int difficultyIndex);

	/**
	 * Called when the user presses the back button.
	 *
	 * @param backButton the back button object
	 */
	void backButton(CCObject* backButton); //when you press back

	virtual void keyBackClicked() override; //when you press escape

	virtual ~DPRemovedLayer();

public:
	void loadLevelsFinished(CCArray*, const char*) override;
	void loadLevelsFinished(CCArray* levels, const char* key, int) override {
		loadLevelsFinished(levels, key);
	}

	/**
	 * Creates a new removed levels screen for the given difficulty pack.
	 *
	 * @param difficultyPack The difficulty pack index
	 *
	 * @return A pointer to the created layer
	 */
	static DPRemovedLayer* create(int difficultyIndex);

	void pageRight(CCObject*);

	void pageLeft(CCObject*);


	/**
	 * Loads the levels for the given page.
	 *
	 * @param page The page to load the levels on
	 */
	void loadLevels(int page);

	/**
	 * Updates the text that says "No levels removed." If there are levels removed to show,
	 * the text will be removed. If there are not, it will be shown.
	 *
	 * @param levels The removed levels
	 */
	void updateNoLevelsText(CCArray* levels);
};
#pragma once

#include <Geode/Geode.hpp>
#include <Geode/utils/web.hpp>
#include <Geode/loader/Event.hpp>

//geode namespace
using namespace geode::prelude;

class DPRemovedLayer : public CCLayer, LevelManagerDelegate {
protected:
	GJListLayer* m_list;
	LoadingCircle* m_loadCircle;
	CCMenu* m_pagesMenu;
	CCMenuItemSpriteExtra* m_left;
	CCMenuItemSpriteExtra* m_right;
	int difficultyIndex;
	int page;
	std::vector<int> levelIDs;

	virtual bool init(int difficultyIndex);
	void backButton(CCObject*); //when you press back
	virtual void keyBackClicked() override; //when you press escape
	virtual ~DPRemovedLayer();

public:
	static DPRemovedLayer* create(int difficultyIndex);
	void pageRight(CCObject*);
	void pageLeft(CCObject*);
	void loadLevels(int page);
	void loadLevelsFinished(CCArray*, const char*) override;
	void loadLevelsFinished(CCArray* levels, const char* key, int) override {
		loadLevelsFinished(levels, key);
	}
};
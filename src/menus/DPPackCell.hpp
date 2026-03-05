#pragma once

#include <Geode/Geode.hpp>

//geode namespace
using namespace geode::prelude;

class DPPackCell : public CCLayer {
protected:
    matjson::Value m_pack; // Pack Data
    std::string m_index; // Should be main/legacy/bonus/monthly
    int m_id;
	int levelID;
	bool isInDifficulty = false;

    virtual bool init() override;
    virtual void onEnter() override;

public:
    static DPPackCell* create(matjson::Value data, std::string index, int id);
    static DPPackCell* create(matjson::Value data, std::string index, int id, int levelID);
	void removeFromDifficulty(CCObject* sender);
	void addToDifficulty(CCObject* sender);
    virtual ~DPPackCell();
	CCMenuItemSpriteExtra* createViewButton();
	void updateDPLayer();

	/*
	 * Recreates this pack cell from scratch, ensuring all elements are up to date.
	 */
	bool recreate();
};
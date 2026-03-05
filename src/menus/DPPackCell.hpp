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

    virtual bool init();
public:
    static DPPackCell* create(matjson::Value data, std::string index, int id);
    static DPPackCell* create(matjson::Value data, std::string index, int id, int levelID);
	void removeFromDifficulty(CCObject* sender);
	void addToDifficulty(CCObject* sender);
    virtual ~DPPackCell();
	CCMenuItemSpriteExtra* createViewButton();
};
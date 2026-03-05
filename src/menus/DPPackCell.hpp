#pragma once

#include <Geode/Geode.hpp>

//geode namespace
using namespace geode::prelude;

class DPPackCell : public CCLayer {
private:
    matjson::Value m_pack; // Pack Data
    std::string m_index; // Should be main/legacy/bonus/monthly
    int m_id;

	/**
	 * The level ID that is currently being edited for this cell. This is present when the side button 
	 * to add/remove the level from a main list difficulty pack) is pressed, which will change the
	 * UI - including things like removing the tabs that aren't the main list, changing the buttons,
	 * etc. If not, and this cell is present on the normal GDDP screen, this will be -1.
	 */
	int levelID;

	/**
	 * Updates the `DPLayer` screen object that this pack cell is placed inside of. This is
	 * called when, for example, a completed level is added/removed from this pack cell,
	 * meaning the progress bars and other UI elements must update.
	 */
	void updateDPLayer();

protected:
    virtual bool init() override;
    virtual void onEnter() override;

public:
    static DPPackCell* create(matjson::Value data, std::string index, int id);

	/**
	 * Creates a new pack cell with a level ID. This is called when the side button 
	 * to add/remove the level from a main list difficulty pack) is pressed, which will change the
	 * UI - including things like removing the tabs that aren't the main list, changing the buttons,
	 * etc. To create a pack cell *normally*, use create(Value, string) with just two arguments.
	 */
    static DPPackCell* create(matjson::Value data, std::string index, int id, int levelID);

	/**
	 * Called when the "remove" button is pressed when the DP list is open on a level for the
	 * add/remove menu when the `enable-main-list-editing` setting is `true`.
	 */
	void removeFromDifficulty(CCObject* sender);

	/**
	 * Called when the "add" button is pressed when the DP list is open on a level for the
	 * add/remove menu when the `enable-main-list-editing` setting is `true`. Removes
	 * this objects `levelID` value from the pack indicated by `m_id`.
	 */
	void addToDifficulty(CCObject* sender);

	/**
	 * Recreates this pack cell from scratch, ensuring all elements are up to date.
	 */
	bool recreate();

    virtual ~DPPackCell();
};
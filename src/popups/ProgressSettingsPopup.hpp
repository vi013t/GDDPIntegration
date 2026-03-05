#pragma once

#include <Geode/Geode.hpp>

//geode namespace
using namespace geode::prelude;

class ProgressSettingsPopup : public Popup {
protected:
	int difficultyIndex;
	bool init() override;

	TextInput* valueInput;
public:
	static ProgressSettingsPopup* create(int difficultyIndex);
	void requireAll(CCObject* sender);
	void reset(CCObject* sender);
	void confirm(CCObject* sender);
};
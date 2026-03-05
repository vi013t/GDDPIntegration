#pragma once

#include <Geode/Geode.hpp>

//geode namespace
using namespace geode::prelude;

/**
 * The popup that appears when pressing the settings icon next to the progress bar, which allows
 * changing the amount of required levels for a pack.
 */
class ProgressSettingsPopup : public Popup {
private:
	int difficultyIndex;
	TextInput* valueInput;

	// Button press actions

	/**
	 * Called when the "Require all" button is pressed. All this does is fill the
	 * text input with "All". That's not actually interpreted, nor are changes made,
	 * until confirm() is called.
	 */
	void requireAll(CCObject* sender);

	/**
	 * Called when the "Reset" button is pressed. All this does is fill the
	 * text input with the default value from the GDDP. The changes are not saved until
	 * confirm() is called.
	 */
	void reset(CCObject* sender);

	/**
	 * Called when the "Ok" button is pressed. Gets the value stored in the value input and
	 * sets the required level count to that value. If it is "all" (case insensitive) the
	 * required level count is set to -2 (-1 is already used as a marker), which is then
	 * interpreted later to mean "all". If the value is less than -2, or is not a number,
	 * it will be set to the default value from the GDDP.
	 */
	void confirm(CCObject* sender);

protected:
	bool init() override;

public:

	/**
	 * Creates a new `ProgressSettingsPopup`.
	 *
	 * @param difficultyIndex The difficulty pack index that the required levels are being changed
	 * for.
	 *
	 * @return A pointer to the popup, which can be shown with `->show()`.
	 */
	static ProgressSettingsPopup* create(int difficultyIndex);
};
#pragma once

#include <Geode/Geode.hpp>
#include <Geode/modify/LevelDownloadDelegate.hpp>

using namespace geode::prelude;

template<typename F> 
requires std::invocable<F, GJGameLevel*> && std::same_as<std::invoke_result_t<F, GJGameLevel*>, void>
class LevelDownloader : public cocos2d::CCNode, public LevelDownloadDelegate {
private:
	F onFinished;
	LevelDownloadDelegate* oldDelegate;

    void levelDownloadFinished(GJGameLevel* level) override {
		this->onFinished(level);
		GameLevelManager::sharedState()->m_levelDownloadDelegate = this->oldDelegate;
		delete this;
	}
	
    void levelDownloadFailed(int levelID) override {
		GameLevelManager::sharedState()->m_levelDownloadDelegate = this->oldDelegate;
		delete this;
	}

public:
	LevelDownloader(int levelID, F onFinished_) : onFinished(onFinished_) {
		this->oldDelegate = GameLevelManager::sharedState()->m_levelDownloadDelegate;
		GameLevelManager::sharedState()->m_levelDownloadDelegate = this;
		GameLevelManager::sharedState()->downloadLevel(levelID, false, -1);
	}

	~LevelDownloader() {}
};

/**
 * Gets a level by ID, and runs the given callback with the level object.
 */
template<typename G> 
requires std::invocable<G, GJGameLevel*> && std::same_as<std::invoke_result_t<G, GJGameLevel*>, void>
void getLevel(int levelID, G callback) {
	new LevelDownloader<G>(levelID, callback);
}
#ifndef STATE_MANAGER_H
#define STATE_MANAGER_H

#ifdef __OBJC__
#include <objc/NSObjCRuntime.h>
#endif

#include "gpg/gpg.h"

class StateManager {
  
public:
    static StateManager* getInstance() {
        if(!_instance) {
            _instance = new StateManager();
        }
        return _instance;
    }
    void initServices(gpg::PlatformConfiguration &pc);
    gpg::GameServices *getGameServices();
    void beginUserInitiatedSignIn();
    void signOut();
    void unlockAchievement(const char *achievementId);
    void submitHighScore(const char *leaderboardId, uint64_t score);
    void showAchievements();
    void showLeaderboard(const char *leaderboardId);
    bool isSignedIn() { return _isSignedIn; }
    void saveSnapshot(std::string description, std::chrono::milliseconds playtime, std::vector<uint8_t> png_data, std::vector< uint8_t > snapData);
    void selectSnapshot(std::string title, uint32_t max_snapshots, bool allow_delete = true, bool allow_create = true);

private:
    StateManager()
    : _isSignedIn(false)
    , _gameServices()
    , _currentSnapshot("") {}

    bool _isSignedIn;
    static StateManager* _instance;
    std::unique_ptr<gpg::GameServices> _gameServices;
    std::string _currentSnapshot;

    void onAuthActionStarted(gpg::AuthOperation op);
    void onAuthActionFinished(gpg::AuthOperation op, gpg::AuthStatus status);
    void loadedSnapshot(gpg::SnapshotManager::ReadResponse const & response);
    void committedSnapshot(gpg::SnapshotManager::CommitResponse const & response);
    void selectedSnapshot(gpg::SnapshotManager::SnapshotSelectUIResponse const & response);
};


#endif // STATE_MANAGER_HPP

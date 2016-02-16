/* Copyright (c) 2014 Google Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "StateManager.h"

#ifdef __APPLE__
//
//Logging for CoreFoundation
//
#include <CoreFoundation/CoreFoundation.h>
extern "C" void NSLog(CFStringRef format, ...);
const int32_t BUFFER_SIZE = 256;
#define LOGI(...) {char c[BUFFER_SIZE];\
    snprintf(c,BUFFER_SIZE,__VA_ARGS__);\
    CFStringRef str = CFStringCreateWithCString(kCFAllocatorDefault, c, kCFStringEncodingMacRoman);\
    NSLog(str);\
    CFRelease(str);\
    }
#else
#include "android/log.h"
#define DEBUG_TAG "TeapotNativeActivity"
#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, DEBUG_TAG, __VA_ARGS__))

#endif

StateManager* StateManager::_instance = nullptr;

void StateManager::onAuthActionFinished(gpg::AuthOperation op, gpg::AuthStatus status) {
    LOGI("OnAuthActionFinished");
    switch (status) {
        case gpg::AuthStatus::VALID:
            LOGI("Signed In");
            break;
        case gpg::AuthStatus::ERROR_INTERNAL:
        case gpg::AuthStatus::ERROR_NOT_AUTHORIZED:
        case gpg::AuthStatus::ERROR_VERSION_UPDATE_REQUIRED:
        case gpg::AuthStatus::ERROR_TIMEOUT:
        default:
            LOGI("Sign-in failure");
            break;
    }
}

void StateManager::onAuthActionStarted(gpg::AuthOperation op) {
    LOGI("OnAuthActionStarted");
    switch (op) {
        case gpg::AuthOperation::SIGN_IN:
            LOGI("Signing In");
            break;
        case gpg::AuthOperation::SIGN_OUT:
            LOGI("Signing Out");
            break;
    }
}

gpg::GameServices *StateManager::getGameServices() {
    return _gameServices.get();
}

void StateManager::beginUserInitiatedSignIn() {
    if (!_gameServices->IsAuthorized()) {
        LOGI("StartAuthorizationUI");
        _gameServices->StartAuthorizationUI();
    }
}

void StateManager::signOut() {
    if (_gameServices->IsAuthorized()) {
        LOGI("SignOut");
        _gameServices->SignOut();
    }
}

void StateManager::unlockAchievement(const char *achievementId) {
    if (_gameServices->IsAuthorized()) {
        LOGI("Achievement unlocked");
        _gameServices->Achievements().Unlock(achievementId);
    }
}

void StateManager::submitHighScore(const char *leaderboardId, uint64_t score) {
    if (_gameServices->IsAuthorized()) {
        LOGI("High score submitted");
        _gameServices->Leaderboards().SubmitScore(leaderboardId, score);
    }
}

void StateManager::showAchievements()
{
    if (_gameServices->IsAuthorized()) {
        LOGI("Show achievement");
        _gameServices->Achievements().ShowAllUI([](gpg::UIStatus const &status) {
            LOGI("Achievement shown");
        });
    }
}

void StateManager::showLeaderboard(const char *leaderboardId)
{
    if (_gameServices->IsAuthorized()) {
        LOGI("Show leaderboard");
        _gameServices->Leaderboards().ShowUI(leaderboardId, [](gpg::UIStatus const &status) {
            LOGI("Leaderboard shown");
        });
    }
}


void StateManager::initServices(gpg::PlatformConfiguration &pc) {

    LOGI("Initializing Services");
    if (!_gameServices) {
        LOGI("Uninitialized services, so creating");
        _gameServices = gpg::GameServices::Builder()
        .SetOnAuthActionStarted([this]( gpg::AuthOperation op ){
            LOGI("Sign in started");
            this->onAuthActionStarted(op);
        })
        .SetOnAuthActionFinished([this](gpg::AuthOperation op, gpg::AuthStatus status){
            LOGI("Sign in finished with a result of %d", status);
            _isSignedIn = status == gpg::AuthStatus::VALID;
            this->onAuthActionFinished(op, status);
        })
        .SetDefaultOnLog(gpg::LogLevel::VERBOSE)
        .EnableSnapshots()
        .Create(pc);
    }
    LOGI("Created");
}

void StateManager::committedSnapshot(gpg::SnapshotManager::CommitResponse const & response)
{
    switch (response.status) {
        case gpg::ResponseStatus::VALID:
            LOGI("Snapshot CommitResponse VALID");
            break;
        case gpg::ResponseStatus::VALID_BUT_STALE:
            LOGI("Snapshot CommitResponse VALID_BUT_STALE");
            break;
        case gpg::ResponseStatus::ERROR_LICENSE_CHECK_FAILED:
            LOGI("Snapshot CommitResponse ERROR_LICENSE_CHECK_FAILED");
            break;
        case gpg::ResponseStatus::ERROR_INTERNAL:
            LOGI("Snapshot CommitResponse ERROR_INTERNAL");
            break;
        case gpg::ResponseStatus::ERROR_NOT_AUTHORIZED:
            LOGI("Snapshot CommitResponse ERROR_NOT_AUTHORIZED");
            break;
        case gpg::ResponseStatus::ERROR_VERSION_UPDATE_REQUIRED:
            LOGI("Snapshot CommitResponse ERROR_VERSION_UPDATE_REQUIRED");
            break;
        case gpg::ResponseStatus::ERROR_TIMEOUT:
            LOGI("Snapshot CommitResponse ERROR_TIMEOUT");
            break;
        default:
            break;
    }
}

void StateManager::loadedSnapshot(gpg::SnapshotManager::ReadResponse const & response)
{
    switch (response.status) {
        case gpg::ResponseStatus::VALID:
            LOGI("Snapshot CommitResponse VALID");
            break;
        case gpg::ResponseStatus::VALID_BUT_STALE:
            LOGI("Snapshot CommitResponse VALID_BUT_STALE");
            break;
        case gpg::ResponseStatus::ERROR_LICENSE_CHECK_FAILED:
            LOGI("Snapshot CommitResponse ERROR_LICENSE_CHECK_FAILED");
            break;
        case gpg::ResponseStatus::ERROR_INTERNAL:
            LOGI("Snapshot CommitResponse ERROR_INTERNAL");
            break;
        case gpg::ResponseStatus::ERROR_NOT_AUTHORIZED:
            LOGI("Snapshot CommitResponse ERROR_NOT_AUTHORIZED");
            break;
        case gpg::ResponseStatus::ERROR_VERSION_UPDATE_REQUIRED:
            LOGI("Snapshot CommitResponse ERROR_VERSION_UPDATE_REQUIRED");
            break;
        case gpg::ResponseStatus::ERROR_TIMEOUT:
            LOGI("Snapshot CommitResponse ERROR_TIMEOUT");
            break;
        default:
            break;
    }
}

void StateManager::selectedSnapshot(gpg::SnapshotManager::SnapshotSelectUIResponse const & response)
{
    switch (response.status) {
        case gpg::UIStatus::VALID:
        {
            if ( response.data.Valid() )
            {
                _currentSnapshot = response.data.FileName();
                if (_gameServices)
                {
                    LOGI("Loading Snapshot %s",_currentSnapshot.c_str());
                    _gameServices->Snapshots().Open(response.data.FileName(),
                                                    gpg::SnapshotConflictPolicy::LONGEST_PLAYTIME,
                                                    [this](gpg::SnapshotManager::OpenResponse const & response) {
                                                        LOGI("Reading file");
                                                        _gameServices->Snapshots().Read(response.data,
                                                                                        std::bind(&StateManager::loadedSnapshot, this,
                                                                                                  std::placeholders::_1));
                                                   });
                }
            } else {
                _currentSnapshot.clear();
            }
            break;
        }
        case gpg::UIStatus::ERROR_INTERNAL:
            LOGI("Snapshot SelectUIResponse ERROR_INTERNAL");
            break;
        case gpg::UIStatus::ERROR_NOT_AUTHORIZED:
            LOGI("Snapshot SelectUIResponse ERROR_NOT_AUTHORIZED");
            break;
        case gpg::UIStatus::ERROR_VERSION_UPDATE_REQUIRED:
            LOGI("Snapshot SelectUIResponse ERROR_VERSION_UPDATE_REQUIRED");
            break;
        case gpg::UIStatus::ERROR_TIMEOUT:
            LOGI("Snapshot SelectUIResponse ERROR_TIMEOUT");
            break;
        case gpg::UIStatus::ERROR_CANCELED:
            LOGI("Snapshot SelectUIResponse ERROR_CANCELED");
            break;
        case gpg::UIStatus::ERROR_UI_BUSY:
            LOGI("Snapshot SelectUIResponse ERROR_UI_BUSY");
            break;
        case gpg::UIStatus::ERROR_LEFT_ROOM:
            LOGI("Snapshot SelectUIResponse ERROR_LEFT_ROOM");
            break;
        default:
            break;
    }
}

void StateManager::saveSnapshot(std::string description,
                                std::chrono::milliseconds playtime,
                                std::vector<uint8_t> png_data,
                                std::vector< uint8_t > snapData)
{
    if (_gameServices)
    {
        //if we do not have current snapshot we generate the filename
        if (_currentSnapshot.empty())
        {

            std::chrono::system_clock::time_point today = std::chrono::system_clock::now();
            std::time_t tt = std::chrono::system_clock::to_time_t ( today );

            //using strftime
            time_t rawtime;
            struct tm * timeinfo;
            char buffer [32];

            time (&tt);
            timeinfo = localtime (&rawtime);

            strftime (buffer,32,"save_%Y%m%d%H%M%S",timeinfo);

            //put_time not available in GCC < 5.0
            //std::ostringstream snapshot_name;
            //snapshot_name << std::put_time(ctime(&tt),"save_%Y%m%d%H%M%S");
            _currentSnapshot.assign(buffer,32);
            LOGI("new snapshot name is: %s",_currentSnapshot.c_str());
        }

        LOGI("Saving Snapshot %s",_currentSnapshot.c_str());
        LOGI("%s",description.c_str());
        _gameServices->Snapshots().Open(_currentSnapshot,
                                       gpg::SnapshotConflictPolicy::LONGEST_PLAYTIME,
                                       [this, description, playtime, png_data, snapData](gpg::SnapshotManager::OpenResponse const &response)
                                       {
                                           gpg::SnapshotMetadata metadata;
                                           if (IsSuccess(response.status)) {
                                               metadata = response.data;
                                               gpg::SnapshotMetadataChange::Builder builder;
                                               if ( description != metadata.Description())
                                               {
                                                   builder.SetDescription(description);
                                               }
                                               if ( playtime != metadata.PlayedTime())
                                               {
                                                   builder.SetPlayedTime(static_cast<gpg::Duration>(playtime));
                                               }
                                               if ( ! png_data.empty() )
                                               {
                                                   builder.SetCoverImageFromPngData(png_data);
                                               }
                                               gpg::SnapshotMetadataChange metadata_change = builder.Create();
                                               _gameServices->Snapshots().Commit(metadata, metadata_change,snapData,std::bind(&StateManager::committedSnapshot,this,std::placeholders::_1));
                                               
                                           } else {
                                               // Handle snapshot open error here
                                           }
                                       });
    }
}

void StateManager::selectSnapshot(std::string title, uint32_t max_snapshots, bool allow_create, bool allow_delete)
{
    if (_gameServices)
    {
        LOGI("Listing Snapshot");
        LOGI("%s",title.c_str());
        _gameServices->Snapshots().ShowSelectUIOperation(allow_create,
                                                         allow_delete,
                                                         max_snapshots,
                                                         title,
                                                         std::bind(&StateManager::selectedSnapshot, this,
                                                                   std::placeholders::_1));
    }
}


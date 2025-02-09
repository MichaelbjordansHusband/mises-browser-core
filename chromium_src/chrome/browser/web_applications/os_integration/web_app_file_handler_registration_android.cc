#include "chrome/browser/web_applications/os_integration/web_app_file_handler_registration.h"

#include "base/functional/bind.h"
#include "base/functional/callback_helpers.h"
#include "base/files/file_path.h"
#include "base/files/file_util.h"
#include "base/files/scoped_temp_dir.h"
#include "base/metrics/histogram_functions.h"
#include "base/metrics/histogram_macros.h"
#include "base/no_destructor.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/web_applications/os_integration/os_integration_manager.h"
#include "chrome/browser/web_applications/os_integration/web_app_shortcut.h"
#include "chrome/browser/web_applications/web_app_constants.h"
#include "chrome/browser/web_applications/web_app_provider.h"
#include "chrome/browser/web_applications/web_app_registrar.h"

namespace web_app {

namespace {

// Result of registering file handlers.
// These values are persisted to logs. Entries should not be renumbered and
// numeric values should never be reused.
enum class RegistrationResult {
  kSuccess = 0,
  kFailToCreateTempDir = 1,
  kFailToWriteMimetypeFile = 2,
  kXdgReturnNonZeroCode = 3,
  kMaxValue = kXdgReturnNonZeroCode
};

// Result of re-creating shortcut.
// These values are persisted to logs. Entries should not be renumbered and
// numeric values should never be reused.
enum class RecreateShortcutResult {
  kSuccess = 0,
  kFailToCreateShortcut = 1,
  kMaxValue = kFailToCreateShortcut
};

// UMA metric name for MIME type registration result.
constexpr const char* kRegistrationResultMetric =
    "Apps.FileHandler.Registration.Linux.Result";

// UMA metric name for MIME type unregistration result.
constexpr const char* kUnregistrationResultMetric =
    "Apps.FileHandler.Unregistration.Linux.Result";

// UMA metric name for file handler shortcut re-create result.
constexpr const char* kRecreateShortcutResultMetric =
    "Apps.FileHandler.Registration.Linux.RecreateShortcut.Result";

// Records UMA metric for result of MIME type registration/unregistration.
void RecordRegistration(RegistrationResult result, bool install) {
  base::UmaHistogramEnumeration(
      install ? kRegistrationResultMetric : kUnregistrationResultMetric,
      result);
}

void OnCreateShortcut(ResultCallback callback, bool shortcut_created) {
  UMA_HISTOGRAM_ENUMERATION(
      kRecreateShortcutResultMetric,
      shortcut_created ? RecreateShortcutResult::kSuccess
                       : RecreateShortcutResult::kFailToCreateShortcut);
  std::move(callback).Run(shortcut_created ? Result::kOk : Result::kError);
}

void OnShortcutInfoReceived(ResultCallback callback,
                            std::unique_ptr<ShortcutInfo> info) {
  if (!info) {
    UMA_HISTOGRAM_ENUMERATION(kRecreateShortcutResultMetric,
                              RecreateShortcutResult::kFailToCreateShortcut);
    std::move(callback).Run(Result::kError);
    return;
  }

  base::FilePath shortcut_data_dir = internals::GetShortcutDataDir(*info);

  ShortcutLocations locations;
  locations.applications_menu_location = APP_MENU_LOCATION_SUBDIR_CHROMEAPPS;

  internals::ScheduleCreatePlatformShortcuts(
      std::move(shortcut_data_dir), locations,
      ShortcutCreationReason::SHORTCUT_CREATION_BY_USER, std::move(info),
      base::BindOnce(OnCreateShortcut, std::move(callback)));
}

void UpdateFileHandlerRegistrationInOs(const AppId& app_id,
                                       Profile* profile,
                                       ResultCallback callback) {
  // On Linux, file associations are managed through shortcuts in the app menu,
  // so after enabling or disabling file handling for an app its shortcuts
  // need to be recreated.
  WebAppProvider::GetForWebApps(profile)
      ->os_integration_manager()
      .GetShortcutInfoForApp(
          app_id, base::BindOnce(&OnShortcutInfoReceived, std::move(callback)));
}

void OnMimeInfoDatabaseUpdated(bool install, bool registration_succeeded) {
  if (!registration_succeeded) {
    LOG(ERROR) << (install ? "Registering MIME types failed."
                           : "Unregistering MIME types failed.");
  }
}

bool UpdateMimeInfoDatabase(bool install,
                            base::FilePath filename,
                            std::string file_contents) {
  DCHECK(!filename.empty());
  DCHECK_NE(install, file_contents.empty());

  base::ScopedTempDir temp_dir;
  if (!temp_dir.CreateUniqueTempDir()) {
    RecordRegistration(RegistrationResult::kFailToCreateTempDir, install);
    return false;
  }

  base::FilePath temp_file_path(temp_dir.GetPath().Append(filename));
  if (!base::WriteFile(temp_file_path, file_contents)) {
    RecordRegistration(RegistrationResult::kFailToWriteMimetypeFile, install);
    return false;
  }

  bool success = true;
  RecordRegistration(success ? RegistrationResult::kSuccess
                             : RegistrationResult::kXdgReturnNonZeroCode,
                     install);
  return success;
}

using UpdateMimeInfoDatabaseOnLinuxCallback =
    base::OnceCallback<bool(base::FilePath profile_path,
		                                std::string file_contents)>;

UpdateMimeInfoDatabaseOnLinuxCallback&
GetUpdateMimeInfoDatabaseCallbackForTesting() {
  static base::NoDestructor<UpdateMimeInfoDatabaseOnLinuxCallback> instance;
  return *instance;
}

// Returns the callback to use for updating MIME info.
UpdateMimeInfoDatabaseOnLinuxCallback GetUpdateMimeInfoDatabaseCallback(
    bool install) {

  return base::BindOnce(&UpdateMimeInfoDatabase, install);
}

void UninstallMimeInfoOnLinux(const AppId& app_id, Profile* profile) {
  std::string file_contents;
  base::FilePath filename;
  internals::GetShortcutIOTaskRunner()->PostTaskAndReplyWithResult(
      FROM_HERE,
      base::BindOnce(GetUpdateMimeInfoDatabaseCallback(/*install=*/false),
                     std::move(filename), std::move(file_contents)),
      base::BindOnce(&OnMimeInfoDatabaseUpdated, /*install=*/false));
}

}  // namespace

bool ShouldRegisterFileHandlersWithOs() {
  return true;
}

bool FileHandlingIconsSupportedByOs() {
  // File type icons are not supported on Linux: see https://crbug.com/1218235
  return false;
}

void InstallMimeInfoOnLinux(const AppId& app_id,
		                            Profile* profile,
					                                const apps::FileHandlers& file_handlers,base::OnceClosure on_done);

void RegisterFileHandlersWithOs(const AppId& app_id,
                                const std::string& app_name,
                                Profile* profile,
                                const apps::FileHandlers& file_handlers,
                                ResultCallback callback) {
  DCHECK(!file_handlers.empty());
  InstallMimeInfoOnLinux(app_id, profile, file_handlers,
                         base::BindOnce(UpdateFileHandlerRegistrationInOs,
                                        app_id, profile, std::move(callback)));
}

void UnregisterFileHandlersWithOs(const AppId& app_id,
                                  Profile* profile,
                                  ResultCallback callback) {
  UninstallMimeInfoOnLinux(app_id, profile);

  // If this was triggered as part of the uninstallation process, nothing more
  // is needed. Uninstalling already cleans up shortcuts (and thus, file
  // handlers).
  auto* provider = WebAppProvider::GetForWebApps(profile);
  if (provider->registrar_unsafe().IsUninstalling(app_id)) {
    std::move(callback).Run(Result::kOk);
    return;
  }
  DCHECK(provider->registrar_unsafe().IsInstalled(app_id));

  // Otherwise, simply recreate the .desktop file.
  UpdateFileHandlerRegistrationInOs(app_id, profile, std::move(callback));
}

void InstallMimeInfoOnLinux(const AppId& app_id,
                            Profile* profile,
                            const apps::FileHandlers& file_handlers, base::OnceClosure on_done) {
  DCHECK(!app_id.empty() && !file_handlers.empty());

  base::FilePath filename;
  std::string file_contents;

 internals::GetShortcutIOTaskRunner()->PostTaskAndReplyWithResult(
      FROM_HERE,
      base::BindOnce(base::BindOnce(&UpdateMimeInfoDatabase, /*install=*/true),
                     std::move(filename), std::move(file_contents)),
      base::BindOnce(&OnMimeInfoDatabaseUpdated, /*install=*/true)
          .Then(std::move(on_done)));
}

void SetUpdateMimeInfoDatabaseOnLinuxCallbackForTesting(  // IN-TEST
    UpdateMimeInfoDatabaseOnLinuxCallback callback) {
  GetUpdateMimeInfoDatabaseCallbackForTesting() =  // IN-TEST
      std::move(callback);
}

}  // namespace web_app

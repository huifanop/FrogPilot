#include "selfdrive/ui/qt/offroad/settings.h"

#include <cassert>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <string>

#include <QDebug>
#include <QLabel>
#include <QProcess>

#include "common/params.h"
#include "common/util.h"
#include "selfdrive/ui/ui.h"
#include "selfdrive/ui/qt/util.h"
#include "selfdrive/ui/qt/widgets/controls.h"
#include "selfdrive/ui/qt/widgets/input.h"
#include "system/hardware/hw.h"

#include "selfdrive/frogpilot/ui/frogpilot_ui_functions.h"

void SoftwarePanel::checkForUpdates() {
  std::system("pkill -SIGUSR1 -f selfdrive.updated");
}

SoftwarePanel::SoftwarePanel(QWidget* parent) : ListWidget(parent), scene(uiState()->scene) {
  // Params paramsMemory = Params("/dev/shm/params");
  onroadLbl = new QLabel(tr("系統更新只會在熄火時下載."));
  onroadLbl->setStyleSheet("font-size: 50px; font-weight: 400; text-align: left; padding-top: 30px; padding-bottom: 30px;");
  addItem(onroadLbl);

  // current version
  versionLbl = new LabelControl(tr("目前版本"), "");
  addItem(versionLbl);

//////////////////////////////////////////////////////////////////////////////////////////////
  fastinstallBtn = new ButtonControl(tr("快速更新"), tr("更新"), "立刻進行更新並重啟機器.");
  connect(fastinstallBtn, &ButtonControl::clicked, [=]() {
    params.putBool("Faststart", false);
    // paramsMemory.putBool("FrogPilotTogglesUpdated", true);
    std::system("git pull");
    Hardware::reboot();
  });
  addItem(fastinstallBtn);
//////////////////////////////////////////////////////////////////////////////////////////////  

  // download update btn
  downloadBtn = new ButtonControl(tr("下載"), tr("檢查"));
  connect(downloadBtn, &ButtonControl::clicked, [=]() {
    downloadBtn->setEnabled(false);
    if (downloadBtn->text() == tr("檢查")) {
      // params.putBool("Faststart", false);
      // paramsMemory.putBool("FrogPilotTogglesUpdated", true);
      if (schedule == 0) {
        params.putBool("ManualUpdateInitiated", true);
      }
      checkForUpdates();
    } else {
      std::system("pkill -SIGHUP -f selfdrive.updated");
    }
  });
  addItem(downloadBtn);

  // install update btn
  installBtn = new ButtonControl(tr("安裝更新"), tr("安裝"));
  connect(installBtn, &ButtonControl::clicked, [=]() {
    installBtn->setEnabled(false);
    ////////////////////////////
    params.putBool("Faststart", false);
    // paramsMemory.putBool("FrogPilotTogglesUpdated", true);
    ////////////////////////////
    params.putBool("DoReboot", true);
  });
  addItem(installBtn);

  // branch selecting
  targetBranchBtn = new ButtonControl(tr("目標分支"), tr("選擇"));
  connect(targetBranchBtn, &ButtonControl::clicked, [=]() {
    auto current = params.get("GitBranch");
    QStringList branches = QString::fromStdString(params.get("UpdaterAvailableBranches")).split(",");
    for (QString b : {current.c_str(), "devel-staging", "devel", "nightly", "master-ci", "master"}) {
      auto i = branches.indexOf(b);
      if (i >= 0) {
        branches.removeAt(i);
        branches.insert(0, b);
      }
    }

    QString cur = QString::fromStdString(params.get("UpdaterTargetBranch"));
    QString selection = MultiOptionDialog::getSelection(tr("選擇分支"), branches, cur, this);
    if (!selection.isEmpty()) {
      params.put("UpdaterTargetBranch", selection.toStdString());
      targetBranchBtn->setValue(QString::fromStdString(params.get("UpdaterTargetBranch")));
      checkForUpdates();
    }
  });
  if (!params.getBool("IsTestedBranch")) {
    addItem(targetBranchBtn);
  }

  // Update scheduler
  std::vector<QString> scheduleOptions{tr("手動"), tr("每日"), tr("每週")};
  FrogPilotButtonParamControl *preferredSchedule = new FrogPilotButtonParamControl("UpdateSchedule", tr("更新頻率"),
                                          tr("選擇自動更新的更新頻率.\n\n"
                                          "開啟此功能後將自動處理下載、安裝和裝置重啟.\n\n"
                                          "每週更新從每週日午夜開始."),
                                          "",
                                          scheduleOptions);
  schedule = params.getInt("UpdateSchedule");
  QObject::connect(preferredSchedule, &FrogPilotButtonParamControl::buttonClicked, [this](int id) {
    schedule = id;
    updateLabels();
  });
  addItem(preferredSchedule);

  updateTime = new ButtonControl(tr("更新時間"), tr("選擇"));
  QStringList hours;
  for (int h = 0; h < 24; h++) {
    int displayHour = (h % 12 == 0) ? 12 : h % 12;
    QString meridiem = (h < 12) ? "AM" : "PM";
    hours << QString("%1:00 %2").arg(displayHour).arg(meridiem)
          << QString("%1:30 %2").arg(displayHour).arg(meridiem);
  }

  QObject::connect(updateTime, &ButtonControl::clicked, [=]() {
    int currentHourIndex = params.getInt("UpdateTime");
    QString currentHourLabel = hours[currentHourIndex];

    QString selection = MultiOptionDialog::getSelection(tr("選擇時間自動更新"), hours, currentHourLabel, this);
    if (!selection.isEmpty()) {
      int selectedHourIndex = hours.indexOf(selection);
      params.putInt("UpdateTime", selectedHourIndex);
      updateTime->setValue(selection);
    }
  });
  time = params.getInt("UpdateTime");
  updateTime->setValue(hours[time]);
  updateTime->setVisible(schedule != 0);
  addItem(updateTime);

  // error log button
  errorLogBtn = new ButtonControl(tr("錯誤資訊"), tr("查看"), "查看錯誤訊息.");
  connect(errorLogBtn, &ButtonControl::clicked, [=]() {
    std::string txt = util::read_file("/data/community/crashes/error.txt");
    ConfirmationDialog::rich(QString::fromStdString(txt), this);
  });
  addItem(errorLogBtn);

  // uninstall button
  auto uninstallBtn = new ButtonControl(tr("解除安裝 %1").arg(getBrand()), tr("解除安裝"));
  connect(uninstallBtn, &ButtonControl::clicked, [&]() {
    if (ConfirmationDialog::confirm(tr("是否確定要解除安裝?"), tr("解除安裝"), this)) {
      params.putBool("DoUninstall", true);
    }
  });
  addItem(uninstallBtn);

//////////////////////////////////////////////////////////////////////////////////////////////
  delLogBtn = new ButtonControl(tr("刪除訊息"), tr("刪除"), "刪除訊息.");
  connect(delLogBtn, &ButtonControl::clicked, [=]() {
    std::system("rm -r /data/community/crashes && mkdir -p /data/community/crashes/");
  });
  addItem(delLogBtn);
//////////////////////////////////////////////////////////////////////////////////////////////

  fs_watch = new ParamWatcher(this);
  QObject::connect(fs_watch, &ParamWatcher::paramChanged, [=](const QString &param_name, const QString &param_value) {
    updateLabels();
  });

  connect(uiState(), &UIState::offroadTransition, [=](bool offroad) {
    is_onroad = !offroad;
    updateLabels();
  });

  QObject::connect(uiState(), &UIState::uiUpdate, this, &SoftwarePanel::automaticUpdate);

  updateLabels();
}

void SoftwarePanel::showEvent(QShowEvent *event) {
  // nice for testing on PC
  installBtn->setEnabled(true);

  updateLabels();
}

void SoftwarePanel::updateLabels() {
  // add these back in case the files got removed
  fs_watch->addParam("LastUpdateTime");
  fs_watch->addParam("UpdateFailedCount");
  fs_watch->addParam("UpdaterState");
  fs_watch->addParam("UpdateAvailable");

  if (!isVisible()) {
    return;
  }

  // updater only runs offroad or when parked
  bool parked = scene.parked;

  onroadLbl->setVisible(is_onroad && !parked);
  downloadBtn->setVisible(!is_onroad || parked);

  // download update
  QString updater_state = QString::fromStdString(params.get("UpdaterState"));
  bool failed = std::atoi(params.get("UpdateFailedCount").c_str()) > 0;
  if (updater_state != "idle") {
    downloadBtn->setEnabled(false);
    downloadBtn->setValue(updater_state);
  } else {
    if (failed && schedule != 0) {
      downloadBtn->setText(tr("檢查"));
      downloadBtn->setValue(tr("檢查更新失敗"));
    } else if (params.getBool("UpdaterFetchAvailable")) {
      downloadBtn->setText(tr("下載"));
      downloadBtn->setValue(tr("有新版本"));
    } else {
      QString lastUpdate = tr("從未更新");
      auto tm = params.get("LastUpdateTime");
      if (!tm.empty()) {
        lastUpdate = timeAgo(QDateTime::fromString(QString::fromStdString(tm + "Z"), Qt::ISODate));
      }
      downloadBtn->setText(tr("檢查"));
      downloadBtn->setValue(tr("已經是最新版本，上次檢查時間為 %1").arg(lastUpdate));
    }
    downloadBtn->setEnabled(true);
  }
  targetBranchBtn->setValue(QString::fromStdString(params.get("UpdaterTargetBranch")));

  // current + new versions
  versionLbl->setText(QString::fromStdString(params.get("UpdaterCurrentDescription")));
  versionLbl->setDescription(QString::fromStdString(params.get("UpdaterCurrentReleaseNotes")));

  installBtn->setVisible((!is_onroad || parked) && params.getBool("UpdateAvailable"));
  installBtn->setValue(QString::fromStdString(params.get("UpdaterNewDescription")));
  installBtn->setDescription(QString::fromStdString(params.get("UpdaterNewReleaseNotes")));

  updateTime->setVisible(params.getInt("UpdateSchedule"));

  update();
}

void SoftwarePanel::automaticUpdate() {
  // Variable declarations
  static bool isDownloadCompleted = false;
  static bool updateCheckedToday = false;

  std::time_t currentTimeT = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
  std::tm now = *std::localtime(&currentTimeT);

  // Check to make sure we're not onroad and have a WiFi connection
  bool isWifiConnected = (*uiState()->sm)["deviceState"].getDeviceState().getNetworkType() == cereal::DeviceState::NetworkType::WIFI;
  if (schedule == 0 || is_onroad || !isWifiConnected || isVisible()) return;

  // Reboot if an automatic update was completed
  if (isDownloadCompleted) {
    if (installBtn->isVisible()) Hardware::reboot();
    return;
  }

  // Format "Updated" to a useable format
  std::tm lastUpdate;
  std::istringstream ss(params.get("Updated"));
  ss >> std::get_time(&lastUpdate, "%Y-%m-%d %H:%M:%S");
  std::time_t lastUpdateTimeT = std::mktime(&lastUpdate);

  // Check if an update was already performed today
  static int lastCheckedDay = now.tm_yday;
  if (lastCheckedDay != now.tm_yday) {
    updateCheckedToday = false;
    lastCheckedDay = now.tm_yday;
  } else if (lastUpdate.tm_yday == now.tm_yday) {
    return;
  }

  // Check if it's time to update
  std::chrono::hours durationSinceLastUpdate = std::chrono::duration_cast<std::chrono::hours>(std::chrono::system_clock::now() - std::chrono::system_clock::from_time_t(lastUpdateTimeT));
  int daysSinceLastUpdate = durationSinceLastUpdate.count() / 24;

  if ((schedule == 1 && daysSinceLastUpdate >= 1) || (schedule == 2 && (now.tm_yday / 7) != (std::localtime(&lastUpdateTimeT)->tm_yday / 7))) {
    if (downloadBtn->text() == tr("檢查") && !updateCheckedToday) {
      checkForUpdates();
      updateCheckedToday = true;
    } else {
      std::system("pkill -SIGHUP -f selfdrive.updated");
      isDownloadCompleted = true;
    }
  }
}

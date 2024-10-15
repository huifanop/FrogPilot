#include <regex>
#include <string>

#include <QDateTime>
#include <QtConcurrent>

#include "selfdrive/frogpilot/navigation/ui/maps_settings.h"

FrogPilotMapsPanel::FrogPilotMapsPanel(FrogPilotSettingsWindow *parent) : FrogPilotListWidget(parent) {
  std::vector<QString> scheduleOptions{tr("手動"), tr("每週"), tr("每月")};
  preferredSchedule = new ButtonParamControl("PreferredSchedule", tr("地圖排程"),
                                          tr("選擇使用最新 OpenStreetMap (OSM) 變更更新地圖的頻率. "
                                             "每週更新從每週日午夜開始，每月更新從每月 1 日午夜開始."),
                                             "",
                                             scheduleOptions);
  addItem(preferredSchedule);

  selectMapsButton = new FrogPilotButtonsControl(tr("選擇離線地圖"), tr("選擇要與“曲線速度控制”和“速度限制控制器”一起使用的地圖'."), {tr("國家"), tr("州")});
  QObject::connect(selectMapsButton, &FrogPilotButtonsControl::buttonClicked, [this](int id) {
    if (id == 0) {
      countriesOpen = true;
    }
    displayMapButtons();
    openMapSelection();
  });
  addItem(selectMapsButton);

  downloadMapsButton = new ButtonControl(tr("下載地圖"), tr("下載"), tr("下載您選擇的地圖以與“曲線速度控制”和“速度限制控制器”一起使用."));
  QObject::connect(downloadMapsButton, &ButtonControl::clicked, [this] {
    if (downloadMapsButton->text() == tr("取消")) {
      cancelDownload();
    } else {
      downloadMaps();
    }
  });
  addItem(downloadMapsButton);

  addItem(mapsSize = new LabelControl(tr("下載地圖的大小"), calculateDirectorySize(mapsFolderPath)));
  addItem(downloadStatus = new LabelControl(tr("下載進度")));
  addItem(downloadETA = new LabelControl(tr("下載完成預計時間")));
  addItem(downloadTimeElapsed = new LabelControl(tr("下載已用時間")));
  addItem(lastMapsDownload = new LabelControl(tr("地圖上次更新"), params.get("LastMapsUpdate").empty() ? "Never" : QString::fromStdString(params.get("LastMapsUpdate"))));

  downloadETA->setVisible(false);
  downloadStatus->setVisible(false);
  downloadTimeElapsed->setVisible(false);

  removeMapsButton = new ButtonControl(tr("刪除地圖"), tr("刪除"), tr("刪除下載的地圖以清理儲存空間."));
  QObject::connect(removeMapsButton, &ButtonControl::clicked, [this] {
    if (FrogPilotConfirmationDialog::yesorno(tr("您確定要刪除所有下載的地圖嗎?"), this)) {
      std::thread([this] {
        mapsSize->setText("0 MB");
        std::system("rm -rf /data/media/0/osm/offline");
      }).detach();
    }
  });
  addItem(removeMapsButton);
  removeMapsButton->setVisible(QDir(mapsFolderPath).exists());

  addItem(midwestLabel = new LabelControl(tr("United States - Midwest"), ""));
  addItem(midwestMaps = new MapSelectionControl(midwestMap));

  addItem(northeastLabel = new LabelControl(tr("United States - Northeast"), ""));
  addItem(northeastMaps = new MapSelectionControl(northeastMap));

  addItem(southLabel = new LabelControl(tr("United States - South"), ""));
  addItem(southMaps = new MapSelectionControl(southMap));

  addItem(westLabel = new LabelControl(tr("United States - West"), ""));
  addItem(westMaps = new MapSelectionControl(westMap));

  addItem(territoriesLabel = new LabelControl(tr("United States - Territories"), ""));
  addItem(territoriesMaps = new MapSelectionControl(territoriesMap));

  addItem(taiwanLabel = new LabelControl(tr("Taiwan"), ""));
  addItem(taiwanMaps = new MapSelectionControl(taiwanMap, true));

  addItem(africaLabel = new LabelControl(tr("Africa"), ""));
  addItem(africaMaps = new MapSelectionControl(africaMap, true));

  addItem(antarcticaLabel = new LabelControl(tr("Antarctica"), ""));
  addItem(antarcticaMaps = new MapSelectionControl(antarcticaMap, true));

  addItem(asiaLabel = new LabelControl(tr("Asia"), ""));
  addItem(asiaMaps = new MapSelectionControl(asiaMap, true));

  addItem(europeLabel = new LabelControl(tr("Europe"), ""));
  addItem(europeMaps = new MapSelectionControl(europeMap, true));

  addItem(northAmericaLabel = new LabelControl(tr("North America"), ""));
  addItem(northAmericaMaps = new MapSelectionControl(northAmericaMap, true));

  addItem(oceaniaLabel = new LabelControl(tr("Oceania"), ""));
  addItem(oceaniaMaps = new MapSelectionControl(oceaniaMap, true));

  addItem(southAmericaLabel = new LabelControl(tr("South America"), ""));
  addItem(southAmericaMaps = new MapSelectionControl(southAmericaMap, true));

  QObject::connect(parent, &FrogPilotSettingsWindow::closeMapSelection, [this]() {
    displayMapButtons(false);
    countriesOpen = false;
    mapsSelected = QString::fromStdString(params.get("MapsSelected"));
  });
  QObject::connect(uiState(), &UIState::uiUpdate, this, &FrogPilotMapsPanel::updateState);

  displayMapButtons(false);
}

void FrogPilotMapsPanel::showEvent(QShowEvent *event) {
  mapsSelected = QString::fromStdString(params.get("MapsSelected"));
}

void FrogPilotMapsPanel::hideEvent(QHideEvent *event) {
  displayMapButtons(false);
  countriesOpen = false;
}

void FrogPilotMapsPanel::updateState(const UIState &s) {
  if (!isVisible()) {
    return;
  }

  if (downloadActive) {
    updateDownloadStatusLabels();
  }

  downloadMapsButton->setEnabled(!mapsSelected.isEmpty() && s.scene.online);
}

void FrogPilotMapsPanel::cancelDownload() {
  if (FrogPilotConfirmationDialog::yesorno(tr("您確定要取消下載嗎?"), this)) {
    std::system("pkill mapd");
    resetDownloadState();
  }
}

void FrogPilotMapsPanel::downloadMaps() {
  device()->resetInteractiveTimeout(300);

  params.remove("OSMDownloadProgress");

  resetDownloadLabels();

  downloadETA->setVisible(true);
  downloadStatus->setVisible(true);
  downloadTimeElapsed->setVisible(true);

  lastMapsDownload->setVisible(false);
  removeMapsButton->setVisible(false);

  update();

  int retryCount = 0;
  while (!isMapdRunning() && retryCount < 3) {
    retryCount++;
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }

  if (retryCount >= 3) {
    handleDownloadError();
    return;
  }

  paramsMemory.put("OSMDownloadLocations", params.get("MapsSelected"));

  downloadActive = true;

  startTime = QDateTime::currentMSecsSinceEpoch();
}

void FrogPilotMapsPanel::updateDownloadStatusLabels() {
  static const std::regex fileStatuses(R"("total_files":(\d+),.*"downloaded_files":(\d+))");
  static int previousDownloadedFiles = 0;
  static qint64 lastUpdatedTime = QDateTime::currentMSecsSinceEpoch();
  static qint64 lastRemainingTime = 0;

  std::string osmDownloadProgress = params.get("OSMDownloadProgress");
  std::smatch match;

  if (!std::regex_search(osmDownloadProgress, match, fileStatuses)) {
    resetDownloadLabels();
    return;
  }

  int totalFiles = std::stoi(match[1].str());
  int downloadedFiles = std::stoi(match[2].str());

  qint64 elapsedMilliseconds = QDateTime::currentMSecsSinceEpoch() - startTime;
  qint64 timePerFile = (downloadedFiles > 0) ? (elapsedMilliseconds / downloadedFiles) : 0;
  qint64 remainingFiles = totalFiles - downloadedFiles;
  qint64 remainingTime = timePerFile * remainingFiles;

  if (downloadedFiles >= totalFiles) {
    finalizeDownload();
    return;
  }

  if (downloadedFiles != previousDownloadedFiles) {
    previousDownloadedFiles = downloadedFiles;
    lastUpdatedTime = QDateTime::currentMSecsSinceEpoch();
    lastRemainingTime = remainingTime;

    QtConcurrent::run([this]() {
      QString sizeText = calculateDirectorySize(mapsFolderPath);
      QMetaObject::invokeMethod(mapsSize, [this, sizeText]() {
        mapsSize->setText(sizeText);
      }, Qt::QueuedConnection);
    });
  } else {
    qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
    qint64 timeSinceLastUpdate = (currentTime - lastUpdatedTime) / 1000;
    remainingTime = std::max(qint64(0), lastRemainingTime - timeSinceLastUpdate * 1000);
  }

  updateDownloadLabels(downloadedFiles, totalFiles, remainingTime, elapsedMilliseconds);
}

void FrogPilotMapsPanel::resetDownloadState() {
  downloadMapsButton->setText(tr("下載"));

  downloadETA->setVisible(false);
  downloadStatus->setVisible(false);
  downloadTimeElapsed->setVisible(false);
  removeMapsButton->setVisible(QDir(mapsFolderPath).exists());

  downloadActive = false;

  update();
}

void FrogPilotMapsPanel::handleDownloadError() {
  std::system("pkill mapd");

  downloadMapsButton->setText(tr("下載"));
  downloadStatus->setText("下載錯誤...請重試!");

  downloadETA->setVisible(false);
  downloadTimeElapsed->setVisible(false);

  downloadMapsButton->setEnabled(false);

  downloadActive = false;

  update();

  std::this_thread::sleep_for(std::chrono::seconds(3));

  downloadStatus->setText("");

  downloadStatus->setVisible(false);

  downloadMapsButton->setEnabled(true);

  update();
}

void FrogPilotMapsPanel::finalizeDownload() {
  QString formattedDate = formatCurrentDate();

  params.putNonBlocking("LastMapsUpdate", formattedDate.toStdString());
  params.remove("OSMDownloadProgress");

  resetDownloadLabels();

  downloadMapsButton->setText(tr("下載"));
  lastMapsDownload->setText(formattedDate);

  downloadETA->setVisible(false);
  downloadStatus->setVisible(false);
  downloadTimeElapsed->setVisible(false);
  removeMapsButton->setVisible(true);

  downloadActive = false;

  update();
}

void FrogPilotMapsPanel::resetDownloadLabels() {
  downloadETA->setText("計算...");
  downloadMapsButton->setText(tr("取消"));
  downloadStatus->setText("計算...");
  downloadTimeElapsed->setText("計算...");
}

void FrogPilotMapsPanel::updateDownloadLabels(int downloadedFiles, int totalFiles, qint64 remainingTime, qint64 elapsedMilliseconds) {
  int percentage = (totalFiles > 0) ? (downloadedFiles * 100 / totalFiles) : 0;
  downloadStatus->setText(QString("%1 / %2 (%3%)").arg(downloadedFiles).arg(totalFiles).arg(percentage));

  QDateTime currentDateTime = QDateTime::currentDateTime();
  QDateTime completionTime = currentDateTime.addMSecs(remainingTime);

  QString completionTimeFormatted = completionTime.toString("h:mm AP");
  QString remainingTimeFormatted = QString("%1 (%2)").arg(formatElapsedTime(remainingTime)).arg(completionTimeFormatted);
  downloadETA->setText(remainingTimeFormatted);

  downloadTimeElapsed->setText(formatElapsedTime(elapsedMilliseconds));
}

void FrogPilotMapsPanel::displayMapButtons(bool visible) {
  setUpdatesEnabled(false);

  downloadETA->setVisible(!visible && downloadActive);
  downloadMapsButton->setVisible(!visible);
  downloadStatus->setVisible(!visible && downloadActive);
  downloadTimeElapsed->setVisible(!visible && downloadActive);
  lastMapsDownload->setVisible(!visible && !downloadActive);
  mapsSize->setVisible(!visible);
  preferredSchedule->setVisible(!visible);
  removeMapsButton->setVisible(!visible && QDir(mapsFolderPath).exists());
  selectMapsButton->setVisible(!visible);

  taiwanMaps->setVisible(visible && countriesOpen);
  africaMaps->setVisible(visible && countriesOpen);
  antarcticaMaps->setVisible(visible && countriesOpen);
  asiaMaps->setVisible(visible && countriesOpen);
  europeMaps->setVisible(visible && countriesOpen);
  northAmericaMaps->setVisible(visible && countriesOpen);
  oceaniaMaps->setVisible(visible && countriesOpen);
  southAmericaMaps->setVisible(visible && countriesOpen);

  africaLabel->setVisible(visible && countriesOpen);
  antarcticaLabel->setVisible(visible && countriesOpen);
  asiaLabel->setVisible(visible && countriesOpen);
  europeLabel->setVisible(visible && countriesOpen);
  northAmericaLabel->setVisible(visible && countriesOpen);
  oceaniaLabel->setVisible(visible && countriesOpen);
  southAmericaLabel->setVisible(visible && countriesOpen);

  midwestMaps->setVisible(visible && !countriesOpen);
  northeastMaps->setVisible(visible && !countriesOpen);
  southMaps->setVisible(visible && !countriesOpen);
  territoriesMaps->setVisible(visible && !countriesOpen);
  westMaps->setVisible(visible && !countriesOpen);

  midwestLabel->setVisible(visible && !countriesOpen);
  northeastLabel->setVisible(visible && !countriesOpen);
  southLabel->setVisible(visible && !countriesOpen);
  territoriesLabel->setVisible(visible && !countriesOpen);
  westLabel->setVisible(visible && !countriesOpen);

  setUpdatesEnabled(true);
  update();
}
#include <QMouseEvent>

#include "selfdrive/frogpilot/navigation/ui/navigation_functions.h"
#include "selfdrive/frogpilot/navigation/ui/navigation_settings.h"

FrogPilotNavigationPanel::FrogPilotNavigationPanel(QWidget *parent) : QFrame(parent), scene(uiState()->scene) {
  mainLayout = new QStackedLayout(this);

  navigationWidget = new QWidget();
  QVBoxLayout *navigationLayout = new QVBoxLayout(navigationWidget);
  navigationLayout->setMargin(40);

  ListWidget *list = new ListWidget(navigationWidget);

  Primeless *primelessPanel = new Primeless(this);
  mainLayout->addWidget(primelessPanel);

  ButtonControl *manageNOOButton = new ButtonControl(tr("管理導航設定"), tr("管理"), tr("在設備上管理導航資訊."));
  QObject::connect(manageNOOButton, &ButtonControl::clicked, [=]() { mainLayout->setCurrentWidget(primelessPanel); });
  QObject::connect(primelessPanel, &Primeless::backPress, [=]() { mainLayout->setCurrentWidget(navigationWidget); });
  list->addItem(manageNOOButton);
  manageNOOButton->setVisible(!uiState()->hasPrime());

  std::vector<QString> scheduleOptions{tr("手動"), tr("每週"), tr("每月")};
  ButtonParamControl *preferredSchedule = new ButtonParamControl("PreferredSchedule", tr("地圖更新頻率"),
                                          tr("選擇使用最新 OpenStreetMap (OSM) 變更更新地圖的頻率. "
                                          "每週更新從每週日午夜開始，每月更新從每月 1 日午夜開始. "
                                          "如果您的裝置在計劃更新期間關閉或離線，則下次您越野時間超過 5 分鐘時就會下載."),
                                          "",
                                          scheduleOptions);
  schedule = params.getInt("PreferredSchedule");
  schedulePending = params.getBool("SchedulePending");
  list->addItem(preferredSchedule);

  list->addItem(offlineMapsSize = new LabelControl(tr("離線地圖檔案大小"), formatSize(calculateDirectorySize(offlineFolderPath))));
  offlineMapsSize->setVisible(true);
  list->addItem(offlineMapsStatus = new LabelControl(tr("離線地圖狀態"), ""));
  offlineMapsStatus->setVisible(false);
  list->addItem(offlineMapsETA = new LabelControl(tr("離線地圖預計下載時間"), ""));
  offlineMapsETA->setVisible(false);
  list->addItem(offlineMapsElapsed = new LabelControl(tr("時間已過"), ""));
  offlineMapsElapsed->setVisible(false);

  cancelDownloadButton = new ButtonControl(tr("取消下載"), tr("取消"), tr("取消下載目前選定地圖."));
  QObject::connect(cancelDownloadButton, &ButtonControl::clicked, [this] { cancelDownload(this); });
  list->addItem(cancelDownloadButton);
  cancelDownloadButton->setVisible(false);

  downloadOfflineMapsButton = new ButtonControl(tr("下載離線地圖"), tr("下載"), tr("下載您選擇的離線地圖在 openpilot 上使用."));
  QObject::connect(downloadOfflineMapsButton, &ButtonControl::clicked, [this] { downloadMaps(); });
  list->addItem(downloadOfflineMapsButton);
  downloadOfflineMapsButton->setVisible(!params.get("MapsSelected").empty());

  SelectMaps *mapsPanel = new SelectMaps(this);
  mainLayout->addWidget(mapsPanel);

  QObject::connect(mapsPanel, &SelectMaps::setMaps, [=]() { setMaps(); });

  ButtonControl *selectMapsButton = new ButtonControl(tr("選擇離線地圖"), tr("選擇"), tr("選擇地圖來使用 OSM."));
  QObject::connect(selectMapsButton, &ButtonControl::clicked, [=]() { mainLayout->setCurrentWidget(mapsPanel); });
  QObject::connect(mapsPanel, &SelectMaps::backPress, [=]() { mainLayout->setCurrentWidget(navigationWidget); });
  list->addItem(selectMapsButton);

  removeOfflineMapsButton = new ButtonControl(tr("移除離線地圖"), tr("移除"), tr("刪除下載的離線地圖以清理儲存空間."));
  QObject::connect(removeOfflineMapsButton, &ButtonControl::clicked, [this] { removeMaps(this); });
  list->addItem(removeOfflineMapsButton);
  removeOfflineMapsButton->setVisible(QDir(offlineFolderPath).exists());

  navigationLayout->addWidget(new ScrollView(list, navigationWidget));
  navigationWidget->setLayout(navigationLayout);
  mainLayout->addWidget(navigationWidget);
  mainLayout->setCurrentWidget(navigationWidget);

  QObject::connect(uiState(), &UIState::uiUpdate, this, &FrogPilotNavigationPanel::updateState);

  checkIfUpdateMissed();
}

void FrogPilotNavigationPanel::hideEvent(QHideEvent *event) {
  QWidget::hideEvent(event);
  mainLayout->setCurrentWidget(navigationWidget);
}

void FrogPilotNavigationPanel::updateState() {
  if (!isVisible()) updateVisibility(downloadActive);
  if (downloadActive) updateStatuses();
  if (schedule) downloadSchedule();
}

void FrogPilotNavigationPanel::updateStatuses() {
  static std::chrono::steady_clock::time_point startTime = std::chrono::steady_clock::now();
  osmDownloadProgress = params.get("OSMDownloadProgress");

  const int totalFiles = extractFromJson<int>(osmDownloadProgress, "\"total_files\":");
  const int downloadedFiles = extractFromJson<int>(osmDownloadProgress, "\"downloaded_files\":");

  if (downloadedFiles >= totalFiles && !osmDownloadProgress.empty()) {
    downloadActive = false;
  }

  if (osmDownloadProgress != previousOSMDownloadProgress && isVisible()) {
    qint64 fileSize = calculateDirectorySize(offlineFolderPath);
    offlineMapsSize->setText(formatSize(fileSize));
    previousOSMDownloadProgress = osmDownloadProgress;
  }

  elapsedTime = calculateElapsedTime(totalFiles, downloadedFiles, startTime);

  offlineMapsElapsed->setText(elapsedTime);
  offlineMapsETA->setText(calculateETA(totalFiles, downloadedFiles, startTime));
  offlineMapsStatus->setText(formatDownloadStatus(totalFiles, downloadedFiles));

  if (downloadActive != previousDownloadActive) {
    startTime = !downloadActive ? std::chrono::steady_clock::now() : startTime;
    updateVisibility(downloadActive);
    previousDownloadActive = downloadActive;
  }
}

void FrogPilotNavigationPanel::updateVisibility(bool visibility) {
  cancelDownloadButton->setVisible(visibility);
  offlineMapsElapsed->setVisible(visibility);
  offlineMapsETA->setVisible(visibility);
  offlineMapsStatus->setVisible(visibility);
  downloadOfflineMapsButton->setVisible(!visibility);
  removeOfflineMapsButton->setVisible(QDir(offlineFolderPath).exists());
}

void FrogPilotNavigationPanel::checkIfUpdateMissed() {
  std::string lastScheduledUpdate = params.get("LastScheduledUpdate");

  if (lastScheduledUpdate.empty() || schedule == 0) {
    return;
  }

  std::time_t t = std::time(nullptr);
  std::tm *now = std::localtime(&t);

  std::tm lastUpdate = {};
  sscanf(lastScheduledUpdate.c_str(), "%d-%d-%d", &lastUpdate.tm_year, &lastUpdate.tm_mon, &lastUpdate.tm_mday);
  lastUpdate.tm_year -= 1900;
  lastUpdate.tm_mon -= 1;

  std::time_t lastUpdateTime = std::mktime(&lastUpdate);

  if (schedule == 1) {
    bool isTodaySunday = (now->tm_wday == 0);
    std::tm *lastUpdateDay = std::localtime(&lastUpdateTime);
    bool wasLastUpdateSunday = (lastUpdateDay->tm_wday == 0);

    schedulePending = (isTodaySunday && !wasLastUpdateSunday) || (now->tm_wday > lastUpdateDay->tm_wday);
  } else if (schedule == 2) {
    bool isTodayFirstOfMonth = (now->tm_mday == 1);
    bool wasLastUpdateFirstOfMonth = (lastUpdate.tm_mday == 1);

    schedulePending = (isTodayFirstOfMonth && !wasLastUpdateFirstOfMonth) || (now->tm_mon != lastUpdate.tm_mon);
  }
}

void FrogPilotNavigationPanel::downloadSchedule() {
  const bool wifi = (*uiState()->sm)["deviceState"].getDeviceState().getNetworkType() == cereal::DeviceState::NetworkType::WIFI;

  const std::time_t t = std::time(nullptr);
  const std::tm *now = std::localtime(&t);

  const bool isScheduleTime = (schedule == 1 && now->tm_wday == 0) || (schedule == 2 && now->tm_mday == 1);

  if ((isScheduleTime || schedulePending) && !(scene.started || scheduleCompleted) && wifi) {
    downloadMaps();
    scheduleCompleted = true;

    char dateStr[11];
    snprintf(dateStr, sizeof(dateStr), "%04d-%02d-%02d", now->tm_year + 1900, now->tm_mon + 1, now->tm_mday);
    std::string lastScheduledUpdate(dateStr);

    params.put("LastScheduledUpdate", lastScheduledUpdate);
    if (schedulePending) {
      schedulePending = false;
      params.putBool("SchedulePending", false);
    }
  } else if (!isScheduleTime) {
    scheduleCompleted = false;
  } else {
    if (!schedulePending) {
      params.putBool("SchedulePending", true);
    }
    schedulePending = true;
  }
}

void FrogPilotNavigationPanel::cancelDownload(QWidget *parent) {
  if (ConfirmationDialog::yesorno("Are you sure you want to cancel the download?", parent)) {
    std::thread([&] {
      std::system("pkill mapd");
    }).detach();
    if (ConfirmationDialog::toggle("Reboot required to re-enable map downloads", "Reboot Now", parent)) {
      Hardware::reboot();
    }
    downloadActive = false;
    updateVisibility(downloadActive);
    downloadOfflineMapsButton->setVisible(downloadActive);
  }
}

void FrogPilotNavigationPanel::downloadMaps() {
  params.remove("OSMDownloadProgress");
  paramsMemory.put("OSMDownloadLocations", params.get("MapsSelected"));
  removeOfflineMapsButton->setVisible(true);
  downloadActive = true;
}

void FrogPilotNavigationPanel::removeMaps(QWidget *parent) {
  if (ConfirmationDialog::yesorno("Are you sure you want to delete all of your downloaded maps?", parent)) {
    std::thread([&] {
      removeOfflineMapsButton->setVisible(false);
      offlineMapsSize->setText(formatSize(0));
      std::system("rm -rf /data/media/0/osm/offline");
    }).detach();
  }
}

void FrogPilotNavigationPanel::setMaps() {
  std::thread([&] {
    QStringList states = ButtonSelectionControl::selectedStates.split(',', QString::SkipEmptyParts);
    QStringList countries = ButtonSelectionControl::selectedCountries.split(',', QString::SkipEmptyParts);

    QJsonObject json;
    json.insert("states", QJsonArray::fromStringList(states));
    json.insert("nations", QJsonArray::fromStringList(countries));

    params.put("MapsSelected", QJsonDocument(json).toJson(QJsonDocument::Compact).toStdString());

    if (!states.isEmpty() || !countries.isEmpty()) {
      downloadOfflineMapsButton->setVisible(true);
    } else {
      params.remove("MapsSelected");
    }
  }).detach();
}

SelectMaps::SelectMaps(QWidget *parent) : QWidget(parent) {
  QVBoxLayout *mainLayout = new QVBoxLayout(this);

  QHBoxLayout *buttonsLayout = new QHBoxLayout();
  buttonsLayout->setContentsMargins(20, 40, 20, 0);

  backButton = new QPushButton(tr("返回"), this);
  statesButton = new QPushButton(tr("州"), this);
  countriesButton = new QPushButton(tr("國家"), this);

  backButton->setFixedSize(400, 100);
  statesButton->setFixedSize(400, 100);
  countriesButton->setFixedSize(400, 100);

  buttonsLayout->addWidget(backButton);
  buttonsLayout->addStretch();
  buttonsLayout->addWidget(statesButton);
  buttonsLayout->addStretch();
  buttonsLayout->addWidget(countriesButton);
  mainLayout->addLayout(buttonsLayout);

  mainLayout->addWidget(horizontalLine());
  mainLayout->setSpacing(20);

  mapsLayout = new QStackedLayout();
  mapsLayout->setMargin(40);
  mapsLayout->setSpacing(20);
  mainLayout->addLayout(mapsLayout);

  QObject::connect(backButton, &QPushButton::clicked, this, [this]() { emit backPress(), emit setMaps(); });

  ListWidget *statesList = new ListWidget();

  LabelControl *northeastLabel = new LabelControl(tr("美國 - 東北部"), "");
  statesList->addItem(northeastLabel);

  ButtonSelectionControl *northeastControl = new ButtonSelectionControl("", tr(""), tr(""), northeastMap, false);
  statesList->addItem(northeastControl);

  LabelControl *midwestLabel = new LabelControl(tr("美國 - 中西部"), "");
  statesList->addItem(midwestLabel);

  ButtonSelectionControl *midwestControl = new ButtonSelectionControl("", tr(""), tr(""), midwestMap, false);
  statesList->addItem(midwestControl);

  LabelControl *southLabel = new LabelControl(tr("美國 - 南部"), "");
  statesList->addItem(southLabel);

  ButtonSelectionControl *southControl = new ButtonSelectionControl("", tr(""), tr(""), southMap, false);
  statesList->addItem(southControl);

  LabelControl *westLabel = new LabelControl(tr("美國 - 西部"), "");
  statesList->addItem(westLabel);

  ButtonSelectionControl *westControl = new ButtonSelectionControl("", tr(""), tr(""), westMap, false);
  statesList->addItem(westControl);

  LabelControl *territoriesLabel = new LabelControl(tr("美國 - 領土"), "");
  statesList->addItem(territoriesLabel);

  ButtonSelectionControl *territoriesControl = new ButtonSelectionControl("", tr(""), tr(""), territoriesMap, false);
  statesList->addItem(territoriesControl);

  statesScrollView = new ScrollView(statesList);
  mapsLayout->addWidget(statesScrollView);

  QObject::connect(statesButton, &QPushButton::clicked, this, [this]() {
    mapsLayout->setCurrentWidget(statesScrollView);
    statesButton->setStyleSheet(activeButtonStyle);
    countriesButton->setStyleSheet(normalButtonStyle);
  });

  ListWidget *countriesList = new ListWidget();

  LabelControl *asiaLabel = new LabelControl(tr("亞洲"), "");
  countriesList->addItem(asiaLabel);

  ButtonSelectionControl *asiaControl = new ButtonSelectionControl("", tr(""), tr(""), asiaMap, true);
  countriesList->addItem(asiaControl);

  LabelControl *africaLabel = new LabelControl(tr("非洲"), "");
  countriesList->addItem(africaLabel);

  ButtonSelectionControl *africaControl = new ButtonSelectionControl("", tr(""), tr(""), africaMap, true);
  countriesList->addItem(africaControl);

  LabelControl *antarcticaLabel = new LabelControl(tr("南極洲"), "");
  countriesList->addItem(antarcticaLabel);

  ButtonSelectionControl *antarcticaControl = new ButtonSelectionControl("", tr(""), tr(""), antarcticaMap, true);
  countriesList->addItem(antarcticaControl);

  LabelControl *europeLabel = new LabelControl(tr("歐洲"), "");
  countriesList->addItem(europeLabel);

  ButtonSelectionControl *europeControl = new ButtonSelectionControl("", tr(""), tr(""), europeMap, true);
  countriesList->addItem(europeControl);

  LabelControl *northAmericaLabel = new LabelControl(tr("北美洲"), "");
  countriesList->addItem(northAmericaLabel);

  ButtonSelectionControl *northAmericaControl = new ButtonSelectionControl("", tr(""), tr(""), northAmericaMap, true);
  countriesList->addItem(northAmericaControl);

  LabelControl *oceaniaLabel = new LabelControl(tr("大洋洲"), "");
  countriesList->addItem(oceaniaLabel);

  ButtonSelectionControl *oceaniaControl = new ButtonSelectionControl("", tr(""), tr(""), oceaniaMap, true);
  countriesList->addItem(oceaniaControl);

  LabelControl *southAmericaLabel = new LabelControl(tr("南美洲"), "");
  countriesList->addItem(southAmericaLabel);

  ButtonSelectionControl *southAmericaControl = new ButtonSelectionControl("", tr(""), tr(""), southAmericaMap, true);
  countriesList->addItem(southAmericaControl);

  countriesScrollView = new ScrollView(countriesList);
  mapsLayout->addWidget(countriesScrollView);

  QObject::connect(countriesButton, &QPushButton::clicked, this, [this]() {
    mapsLayout->setCurrentWidget(countriesScrollView);
    statesButton->setStyleSheet(normalButtonStyle);
    countriesButton->setStyleSheet(activeButtonStyle);
  });

  mapsLayout->setCurrentWidget(statesScrollView);
  statesButton->setStyleSheet(activeButtonStyle);

  setStyleSheet(R"(
    QPushButton {
      font-size: 50px;
      margin: 0px;
      padding: 15px;
      border-width: 0;
      border-radius: 30px;
      color: #dddddd;
      background-color: #393939;
    }
    QPushButton:pressed {
      background-color: #4a4a4a;
    }
  )");
}

QString SelectMaps::activeButtonStyle = R"(
  font-size: 50px;
  margin: 0px;
  padding: 15px;
  border-width: 0;
  border-radius: 30px;
  color: #dddddd;
  background-color: #33Ab4C;
)";

QString SelectMaps::normalButtonStyle = R"(
  font-size: 50px;
  margin: 0px;
  padding: 15px;
  border-width: 0;
  border-radius: 30px;
  color: #dddddd;
  background-color: #393939;
)";

QFrame *SelectMaps::horizontalLine(QWidget *parent) const {
  QFrame *line = new QFrame(parent);

  line->setFrameShape(QFrame::StyledPanel);
  line->setStyleSheet(R"(
    border-width: 2px;
    border-bottom-style: solid;
    border-color: gray;
  )");
  line->setFixedHeight(2);

  return line;
}

void SelectMaps::hideEvent(QHideEvent *event) {
  QWidget::hideEvent(event);
  emit setMaps();
}

Primeless::Primeless(QWidget *parent) : QWidget(parent) {
  QStackedLayout *primelessLayout = new QStackedLayout(this);

  QWidget *mainWidget = new QWidget();
  mainLayout = new QVBoxLayout(mainWidget);
  mainLayout->setMargin(40);

  backButton = new QPushButton(tr("返回"), this);
  backButton->setObjectName("backButton");
  backButton->setFixedSize(400, 100);
  QObject::connect(backButton, &QPushButton::clicked, this, [this]() { emit backPress(); });
  mainLayout->addWidget(backButton, 0, Qt::AlignLeft);

  list = new ListWidget(mainWidget);

  wifi = new WifiManager(this);
  ipLabel = new LabelControl(tr("管理您的設置在"), QString("%1:8082").arg(wifi->getIp4Address()));
  list->addItem(ipLabel);

  std::vector<QString> searchOptions{tr("MapBox"), tr("Amap"), tr("Google")};
  ButtonParamControl *searchInput = new ButtonParamControl("SearchInput", tr("目的地搜尋方式"), 
                                       tr("在 Navigate on Openpilot 中為目的地查詢選擇搜尋提供者。 選項包括 MapBox（建議）、Amap 和 Google 地圖."),
                                       "", searchOptions);
  list->addItem(searchInput);

  createMapboxKeyControl(publicMapboxKeyControl, tr("公共 Mapbox 金鑰"), "MapboxPublicKey", "pk.");
  createMapboxKeyControl(secretMapboxKeyControl, tr("私人 Mapbox 金鑰"), "MapboxSecretKey", "sk.");

  mapboxPublicKeySet = !params.get("MapboxPublicKey").empty();
  mapboxSecretKeySet = !params.get("MapboxSecretKey").empty();
  setupCompleted = mapboxPublicKeySet && mapboxSecretKeySet;

  QHBoxLayout *setupLayout = new QHBoxLayout();
  setupLayout->setMargin(0);

  imageLabel = new QLabel(this);
  pixmap.load(currentStep);
  imageLabel->setPixmap(pixmap.scaledToWidth(1500, Qt::SmoothTransformation));
  setupLayout->addWidget(imageLabel, 0, Qt::AlignCenter);
  imageLabel->hide();

  ButtonControl *setupButton = new ButtonControl(tr("Mapbox 設置說明"), tr("查看"), tr("查看如何設定 MapBox 來使用導航."), this);
  QObject::connect(setupButton, &ButtonControl::clicked, this, [this]() {
    updateStep();
    backButton->hide();
    list->setVisible(false);
    imageLabel->show();
  });
  list->addItem(setupButton);

  QObject::connect(uiState(), &UIState::uiUpdate, this, &Primeless::updateState);

  mainLayout->addLayout(setupLayout);
  mainLayout->addWidget(new ScrollView(list, mainWidget));
  mainWidget->setLayout(mainLayout);
  primelessLayout->addWidget(mainWidget);

  setLayout(primelessLayout);

  setStyleSheet(R"(
    QPushButton {
      font-size: 50px;
      margin: 0px;
      padding: 15px;
      border-width: 0;
      border-radius: 30px;
      color: #dddddd;
      background-color: #393939;
    }
    QPushButton:pressed {
      background-color: #4a4a4a;
    }
  )");
}

void Primeless::hideEvent(QHideEvent *event) {
  QWidget::hideEvent(event);
  backButton->show();
  list->setVisible(true);
  imageLabel->hide();
}

void Primeless::mousePressEvent(QMouseEvent *event) {
  backButton->show();
  list->setVisible(true);
  imageLabel->hide();
}

void Primeless::updateState() {
  if (!isVisible()) return;

  QString ipAddress = wifi->getIp4Address();
  ipLabel->setText(ipAddress.isEmpty() ? tr("裝置離線") : QString("%1:8082").arg(ipAddress));

  mapboxPublicKeySet = !params.get("MapboxPublicKey").empty();
  mapboxSecretKeySet = !params.get("MapboxSecretKey").empty();
  setupCompleted = mapboxPublicKeySet && mapboxSecretKeySet && setupCompleted;

  publicMapboxKeyControl->setText(mapboxPublicKeySet ? tr("移除") : tr("添加"));
  secretMapboxKeyControl->setText(mapboxSecretKeySet ? tr("移除") : tr("添加"));

  if (imageLabel->isVisible()) {
    updateStep();
  }
}

void Primeless::createMapboxKeyControl(ButtonControl *&control, const QString &label, const std::string &paramKey, const QString &prefix) {
  control = new ButtonControl(label, "", tr("管理你的 %1."), this);
  QObject::connect(control, &ButtonControl::clicked, this, [this, control, label, paramKey, prefix] {
    if (control->text() == tr("增加")) {
      QString key = InputDialog::getText(tr("管理你的 %1").arg(label), this);
      if (!key.startsWith(prefix)) {
        key = prefix + key;
      }
      if (key.length() >= 80) {
        params.put(paramKey, key.toStdString());
      }
    } else {
      params.remove(paramKey);
    }
  });
  list->addItem(control);
  control->setText(params.get(paramKey).empty() ? tr("增加") : tr("移除"));
}

void Primeless::updateStep() {
  currentStep = setupCompleted ? "../frogpilot/navigation/navigation_training/setup_completed.png" : 
                (mapboxPublicKeySet && mapboxSecretKeySet) ? "../frogpilot/navigation/navigation_training/both_keys_set.png" :
                mapboxPublicKeySet ? "../frogpilot/navigation/navigation_training/public_key_set.png" : "../frogpilot/navigation/navigation_training/no_keys_set.png";

  pixmap.load(currentStep);
  imageLabel->setPixmap(pixmap.scaledToWidth(1500, Qt::SmoothTransformation));
}

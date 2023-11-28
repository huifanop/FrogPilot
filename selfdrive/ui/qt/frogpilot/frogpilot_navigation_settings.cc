#include <QMouseEvent>

#include "selfdrive/ui/qt/frogpilot/frogpilot_navigation_functions.h"
#include "selfdrive/ui/qt/frogpilot/frogpilot_navigation_settings.h"
#include "selfdrive/ui/qt/offroad/frogpilot_settings.h"
#include "selfdrive/ui/qt/widgets/scrollview.h"

FrogPilotNavigationPanel::FrogPilotNavigationPanel(QWidget *parent) : QFrame(parent), scene(uiState()->scene) {
  mainLayout = new QStackedLayout(this);

  navigationWidget = new QWidget();
  QVBoxLayout *navigationLayout = new QVBoxLayout(navigationWidget);
  navigationLayout->setMargin(40);

  ListWidget *list = new ListWidget(navigationWidget);

  primelessPanel = new Primeless(this);
  mainLayout->addWidget(primelessPanel);

  manageNOOButton = new ButtonControl(tr("管理導航設定"), tr("管理"), tr("在設備上管理導航資訊."));
  QObject::connect(manageNOOButton, &ButtonControl::clicked, [=]() { mainLayout->setCurrentWidget(primelessPanel); });
  QObject::connect(primelessPanel, &Primeless::backPress, [=]() { mainLayout->setCurrentWidget(navigationWidget); });
  list->addItem(manageNOOButton);
  manageNOOButton->setVisible(!uiState()->hasPrime());

  QObject::connect(uiState(), &UIState::primeTypeChanged, this, [=](PrimeType prime_type) {
    bool notPrime = prime_type == PrimeType::NONE || prime_type == PrimeType::UNKNOWN;
    manageNOOButton->setVisible(notPrime);
  });

  std::vector<QString> scheduleOptions{tr("手動"), tr("每週"), tr("每月")};
  preferredSchedule = new ButtonParamControl("PreferredSchedule", tr("地圖更新頻率"),
                                          tr("選擇使用最新 OpenStreetMap (OSM) 變更更新地圖的頻率. "
                                          "每週更新從每週日午夜開始，每月更新從每月 1 日午夜開始. "
                                          "如果您的裝置在計劃更新期間關閉或離線，則下次您越野時間超過 5 分鐘時就會下載."),
                                          "",
                                          scheduleOptions);
  schedule = params.getInt("PreferredSchedule");
  list->addItem(preferredSchedule);

  list->addItem(offlineMapsSize = new LabelControl(tr("離線地圖檔案大小"), ""));
  list->addItem(offlineMapsStatus = new LabelControl(tr("離線地圖狀態"), ""));
  list->addItem(offlineMapsETA = new LabelControl(tr("離線地圖預計下載時間"), ""));
  list->addItem(offlineMapsElapsed = new LabelControl(tr("時間已過"), ""));

  cancelDownloadButton = new ButtonControl(tr("取消下載"), tr("取消"), tr("取消下載目前選定地圖."));
  QObject::connect(cancelDownloadButton, &ButtonControl::clicked, [this] { cancelDownload(this); });
  list->addItem(cancelDownloadButton);

  downloadOfflineMapsButton = new ButtonControl(tr("下載離線地圖"), tr("下載"), tr("下載您選擇的離線地圖在 openpilot 上使用."));
  QObject::connect(downloadOfflineMapsButton, &ButtonControl::clicked, [this] { downloadMaps(this); });
  list->addItem(downloadOfflineMapsButton);

  mapsPanel = new ManageMaps(this);
  mainLayout->addWidget(mapsPanel);

  manageMapsButton = new ButtonControl(tr("管理離線地圖"), tr("管理"), tr("管理地圖應用於 OSM."));
  QObject::connect(manageMapsButton, &ButtonControl::clicked, [=]() { mainLayout->setCurrentWidget(mapsPanel); });
  QObject::connect(mapsPanel, &ManageMaps::backPress, [=]() { mainLayout->setCurrentWidget(navigationWidget); });
  list->addItem(manageMapsButton);

  redownloadOfflineMapsButton = new ButtonControl(tr("重新下載離線地圖"), tr("下載"), tr("重新下載您選擇的離線地圖在 openpilot 上使用."));
  QObject::connect(redownloadOfflineMapsButton, &ButtonControl::clicked, [this] { downloadMaps(this); });
  list->addItem(redownloadOfflineMapsButton);

  removeOfflineMapsButton = new ButtonControl(tr("移除離線地圖"), tr("移除"), tr("刪除下載的離線地圖以清理儲存空間."));
  QObject::connect(removeOfflineMapsButton, &ButtonControl::clicked, [this] { removeMaps(this); });
  list->addItem(removeOfflineMapsButton);

  navigationLayout->addWidget(new ScrollView(list, navigationWidget));
  navigationLayout->addStretch(1);
  navigationWidget->setLayout(navigationLayout);
  mainLayout->addWidget(navigationWidget);
  mainLayout->setCurrentWidget(navigationWidget);

  QObject::connect(uiState(), &UIState::uiUpdate, this, &FrogPilotNavigationPanel::downloadSchedule);
  QObject::connect(uiState(), &UIState::uiUpdate, this, &FrogPilotNavigationPanel::updateState);

  QObject::connect(mapsPanel, &ManageMaps::startDownload, [=]() { downloadMaps(this); });
}

void FrogPilotNavigationPanel::hideEvent(QHideEvent *event) {
  QWidget::hideEvent(event);
  mainLayout->setCurrentWidget(navigationWidget);
}

void FrogPilotNavigationPanel::updateState() {
  if (!isVisible()) return;

  const QString offlineFolderPath = "/data/media/0/osm/offline";
  const bool dirExists = QDir(offlineFolderPath).exists();
  const bool mapsSelected = !params.get("MapsSelected").empty();
  const std::string osmDownloadProgress = params.get("OSMDownloadProgress");

  if (osmDownloadProgress != previousOSMDownloadProgress || !(fileSize || dirExists)) {
    fileSize = 0;
    offlineMapsSize->setText("0 MB");
  }

  previousOSMDownloadProgress = osmDownloadProgress;

  const QString elapsedTime = calculateElapsedTime(osmDownloadProgress, startTime);
  const bool isDownloaded = elapsedTime == "Downloaded";

  cancelDownloadButton->setVisible(!isDownloaded);

  offlineMapsElapsed->setVisible(!isDownloaded);
  offlineMapsETA->setVisible(!isDownloaded);

  offlineMapsElapsed->setText(elapsedTime);
  offlineMapsETA->setText(calculateETA(osmDownloadProgress, startTime));
  offlineMapsStatus->setText(formatDownloadStatus(osmDownloadProgress));

  downloadOfflineMapsButton->setVisible(!dirExists && mapsSelected);
  redownloadOfflineMapsButton->setVisible(dirExists && mapsSelected && osmDownloadProgress.empty());
  removeOfflineMapsButton->setVisible(dirExists && osmDownloadProgress.empty());

  offlineMapsSize->setVisible(false);
}

void FrogPilotNavigationPanel::downloadSchedule() {
  if (!schedule) return;

  std::time_t t = std::time(nullptr);
  std::tm *now = std::localtime(&t);

  bool isScheduleTime = (schedule == 1 && now->tm_wday == 0) || (schedule == 2 && now->tm_mday == 1);
  bool wifi = (*uiState()->sm)["deviceState"].getDeviceState().getNetworkType() == cereal::DeviceState::NetworkType::WIFI;

  if ((isScheduleTime || schedulePending) && !(scene.started || scheduleCompleted) && wifi) {
    downloadMaps(this);
    scheduleCompleted = true;
  } else if (!isScheduleTime) {
    scheduleCompleted = false;
  } else {
    schedulePending = true;
  }
}

void FrogPilotNavigationPanel::cancelDownload(QWidget *parent) {
  std::lock_guard<std::mutex> lock(manageMapsMutex);

  if (ConfirmationDialog::yesorno("Are you sure you want to cancel the download?", parent)) {
    paramsMemory.putBool("OSM", false);
    paramsMemory.remove("OSMDownloadLocations");
    params.remove("OSMDownloadProgress");
    std::thread([&] {
      std::system("pkill mapd");
      std::system("rm -rf /data/media/0/osm/offline");
    }).detach();
    if (ConfirmationDialog::toggle("Reboot required to enable map downloads", "Reboot Now", parent)) {
      Hardware::reboot();
    }
  }
}

void FrogPilotNavigationPanel::downloadMaps(QWidget *parent) {
  std::lock_guard<std::mutex> lock(manageMapsMutex);

  std::thread([&] {
    QStringList states = ButtonSelectionControl::selectedStates.split(',', QString::SkipEmptyParts);
    QStringList countries = ButtonSelectionControl::selectedCountries.split(',', QString::SkipEmptyParts);

    states.removeAll(QString());
    countries.removeAll(QString());

    QJsonObject json;
    if (!states.isEmpty()) {
      json.insert("states", QJsonArray::fromStringList(states));
    }
    if (!countries.isEmpty()) {
      json.insert("nations", QJsonArray::fromStringList(countries));
    }

    paramsMemory.put("OSMDownloadLocations", QJsonDocument(json).toJson(QJsonDocument::Compact).toStdString());
    params.put("MapsSelected", QJsonDocument(json).toJson(QJsonDocument::Compact).toStdString());
  }).detach();

  startTime = std::chrono::steady_clock::now();
}

void FrogPilotNavigationPanel::removeMaps(QWidget *parent) {
  std::lock_guard<std::mutex> lock(manageMapsMutex);
  if (ConfirmationDialog::yesorno("Are you sure you want to delete all of your downloaded maps?", parent)) {
    std::thread([&] {
      std::system("rm -rf /data/media/0/osm/offline");
    }).detach();
  }
}

ManageMaps::ManageMaps(QWidget *parent) : QFrame(parent) {
  back_btn = new QPushButton(tr("返回"), this);
  states_btn = new QPushButton(tr("狀態"), this);
  countries_btn = new QPushButton(tr("國家"), this);

  back_btn->setFixedSize(400, 100);
  states_btn->setFixedSize(400, 100);
  countries_btn->setFixedSize(400, 100);

  QHBoxLayout *buttonsLayout = new QHBoxLayout();
  buttonsLayout->addWidget(back_btn);
  buttonsLayout->addWidget(states_btn);
  buttonsLayout->addWidget(countries_btn);

  mapsLayout = new QStackedLayout();
  mapsLayout->setMargin(40);
  mapsLayout->setSpacing(20);

  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  mainLayout->addLayout(buttonsLayout);
  mainLayout->addLayout(mapsLayout);

  QWidget *buttonsWidget = new QWidget();
  buttonsWidget->setLayout(buttonsLayout);
  mapsLayout->addWidget(buttonsWidget);

  QObject::connect(back_btn, &QPushButton::clicked, this, [this]() { emit backPress(); });

  statesList = new ListWidget();

  northeastLabel = new LabelControl(tr("美國 - 東北部"), "");
  statesList->addItem(northeastLabel);

  ButtonSelectionControl *northeastControl = new ButtonSelectionControl("", tr(""), tr(""), northeastMap, false);
  statesList->addItem(northeastControl);

  midwestLabel = new LabelControl(tr("美國 - 中西部"), "");
  statesList->addItem(midwestLabel);

  ButtonSelectionControl *midwestControl = new ButtonSelectionControl("", tr(""), tr(""), midwestMap, false);
  statesList->addItem(midwestControl);

  southLabel = new LabelControl(tr("美國 - 南部"), "");
  statesList->addItem(southLabel);

  ButtonSelectionControl *southControl = new ButtonSelectionControl("", tr(""), tr(""), southMap, false);
  statesList->addItem(southControl);

  westLabel = new LabelControl(tr("美國 - 西部"), "");
  statesList->addItem(westLabel);

  ButtonSelectionControl *westControl = new ButtonSelectionControl("", tr(""), tr(""), westMap, false);
  statesList->addItem(westControl);

  territoriesLabel = new LabelControl(tr("美國 - 領土"), "");
  statesList->addItem(territoriesLabel);

  ButtonSelectionControl *territoriesControl = new ButtonSelectionControl("", tr(""), tr(""), territoriesMap, false);
  statesList->addItem(territoriesControl);

  ScrollView *statesScrollView = new ScrollView(statesList);
  mapsLayout->addWidget(statesScrollView);

  QObject::connect(states_btn, &QPushButton::clicked, this, [this, statesScrollView]() {
    mapsLayout->setCurrentWidget(statesScrollView);
    states_btn->setStyleSheet(activeButtonStyle);
    countries_btn->setStyleSheet(normalButtonStyle);
  });

  countriesList = new ListWidget();

  asiaLabel = new LabelControl(tr("亞洲"), "");
  countriesList->addItem(asiaLabel);

  ButtonSelectionControl *asiaControl = new ButtonSelectionControl("", tr(""), tr(""), asiaMap, true);
  countriesList->addItem(asiaControl);

  africaLabel = new LabelControl(tr("非洲"), "");
  countriesList->addItem(africaLabel);

  ButtonSelectionControl *africaControl = new ButtonSelectionControl("", tr(""), tr(""), africaMap, true);
  countriesList->addItem(africaControl);

  antarcticaLabel = new LabelControl(tr("南極洲"), "");
  countriesList->addItem(antarcticaLabel);

  ButtonSelectionControl *antarcticaControl = new ButtonSelectionControl("", tr(""), tr(""), antarcticaMap, true);
  countriesList->addItem(antarcticaControl);

  europeLabel = new LabelControl(tr("歐洲"), "");
  countriesList->addItem(europeLabel);

  ButtonSelectionControl *europeControl = new ButtonSelectionControl("", tr(""), tr(""), europeMap, true);
  countriesList->addItem(europeControl);

  northAmericaLabel = new LabelControl(tr("北美洲"), "");
  countriesList->addItem(northAmericaLabel);

  ButtonSelectionControl *northAmericaControl = new ButtonSelectionControl("", tr(""), tr(""), northAmericaMap, true);
  countriesList->addItem(northAmericaControl);

  oceaniaLabel = new LabelControl(tr("大洋洲"), "");
  countriesList->addItem(oceaniaLabel);

  ButtonSelectionControl *oceaniaControl = new ButtonSelectionControl("", tr(""), tr(""), oceaniaMap, true);
  countriesList->addItem(oceaniaControl);

  southAmericaLabel = new LabelControl(tr("南美洲"), "");
  countriesList->addItem(southAmericaLabel);

  ButtonSelectionControl *southAmericaControl = new ButtonSelectionControl("", tr(""), tr(""), southAmericaMap, true);
  countriesList->addItem(southAmericaControl);

  ScrollView *countriesScrollView = new ScrollView(countriesList);
  mapsLayout->addWidget(countriesScrollView);

  QObject::connect(countries_btn, &QPushButton::clicked, this, [this, countriesScrollView]() {
    mapsLayout->setCurrentWidget(countriesScrollView);
    states_btn->setStyleSheet(normalButtonStyle);
    countries_btn->setStyleSheet(activeButtonStyle);
  });

  mapsLayout->setCurrentWidget(statesScrollView);
  states_btn->setStyleSheet(activeButtonStyle);

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

QString ManageMaps::activeButtonStyle = R"(
  font-size: 50px;
  margin: 0px;
  padding: 15px;
  border-width: 0;
  border-radius: 30px;
  color: #dddddd;
  background-color: #33Ab4C;
)";

QString ManageMaps::normalButtonStyle = R"(
  font-size: 50px;
  margin: 0px;
  padding: 15px;
  border-width: 0;
  border-radius: 30px;
  color: #dddddd;
  background-color: #393939;
)";

void ManageMaps::hideEvent(QHideEvent *event) {
  QWidget::hideEvent(event);

  emit startDownload();
}

Primeless::Primeless(QWidget *parent) : QWidget(parent), wifi(new WifiManager(this)), list(new ListWidget(this)), 
    setupMapbox(new SetupMapbox(this)), back(new QPushButton(tr("返回"), this)) {

  QVBoxLayout *primelessLayout = new QVBoxLayout(this);
  primelessLayout->setMargin(40);
  primelessLayout->setSpacing(20);

  back->setObjectName("back_btn");
  back->setFixedSize(400, 100);
  QObject::connect(back, &QPushButton::clicked, this, [this]() { emit backPress(); });
  primelessLayout->addWidget(back, 0, Qt::AlignLeft);

  ipLabel = new LabelControl(tr("管理您的設置在"), QString("%1:8082").arg(wifi->getIp4Address()));
  list->addItem(ipLabel);

  std::vector<QString> searchOptions{tr("MapBox"), tr("Amap"), tr("Google")};
  searchInput = new ButtonParamControl("SearchInput", tr("目的地搜尋方式"),
                                          tr("在 Navigate on Openpilot 中為目的地查詢選擇搜尋提供者。 選項包括 MapBox（建議）、Amap 和 Google 地圖."),
                                          "",
                                          searchOptions);
  list->addItem(searchInput);

  createMapboxKeyControl(publicMapboxKeyControl, tr("公共 Mapbox 金鑰"), "MapboxPublicKey", "pk.");
  createMapboxKeyControl(secretMapboxKeyControl, tr("私人 Mapbox 金鑰"), "MapboxSecretKey", "sk.");

  setupMapbox->setMinimumSize(QSize(1625, 1050));
  setupMapbox->hide();

  setupButton = new ButtonControl(tr("Mapbox 設定說明"), tr("查看"), tr("查看為 Primeless 導覽設定 MapBox 的說明."), this);
  QObject::connect(setupButton, &ButtonControl::clicked, this, [this]() {
    back->hide();
    list->setVisible(false);
    setupMapbox->show();
  });
  list->addItem(setupButton);

  QObject::connect(uiState(), &UIState::uiUpdate, this, &Primeless::updateState);

  primelessLayout->addWidget(new ScrollView(list, this));

  setStyleSheet(R"(
    #setupMapbox > QPushButton, #back_btn {
      font-size: 50px;
      margin: 0px;
      padding: 15px;
      border-width: 0;
      border-radius: 30px;
      color: #dddddd;
      background-color: #393939;
    }
    #back_btn:pressed {
      background-color: #4a4a4a;
    }
  )");
}

void Primeless::hideEvent(QHideEvent *event) {
  QWidget::hideEvent(event);
  list->setVisible(true);
  setupMapbox->hide();
}

void Primeless::mousePressEvent(QMouseEvent *event) {
  back->show();
  list->setVisible(true);
  setupMapbox->hide();
}

void Primeless::updateState() {
  if (!isVisible()) return;

  const bool mapboxPublicKeySet = !params.get("MapboxPublicKey").empty();
  const bool mapboxSecretKeySet = !params.get("MapboxSecretKey").empty();

  publicMapboxKeyControl->setText(mapboxPublicKeySet ? tr("移除") : tr("加入"));
  secretMapboxKeyControl->setText(mapboxSecretKeySet ? tr("移除") : tr("加入"));

  QString ipAddress = wifi->getIp4Address();
  ipLabel->setText(ipAddress.isEmpty() ? tr("裝置離線") : QString("%1:8082").arg(ipAddress));
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

SetupMapbox::SetupMapbox(QWidget *parent) : QFrame(parent) {
  setAttribute(Qt::WA_OpaquePaintEvent);

  mapboxPublicKeySet = !params.get("MapboxPublicKey").empty();
  mapboxSecretKeySet = !params.get("MapboxSecretKey").empty();
  setupCompleted = mapboxPublicKeySet && mapboxSecretKeySet;

  QObject::connect(uiState(), &UIState::uiUpdate, this, &SetupMapbox::updateState);
}

void SetupMapbox::updateState() {
  if (!isVisible()) return;

  mapboxPublicKeySet = !params.get("MapboxPublicKey").empty();
  mapboxSecretKeySet = !params.get("MapboxSecretKey").empty();

  if (!mapboxPublicKeySet || !mapboxSecretKeySet) {
    setupCompleted = false;
  }

  QString newStep = setupCompleted ? "setup_completed" : 
                    mapboxPublicKeySet && mapboxSecretKeySet ? "both_keys_set" :
                    mapboxPublicKeySet ? "public_key_set" : "no_keys_set";

  if (newStep != currentStep) {
    currentStep = newStep;
    currentImage = QImage(QString::fromUtf8(imagePath) + currentStep + ".png");
    repaint();
  }
}

void SetupMapbox::paintEvent(QPaintEvent *event) {
  QPainter painter(this);

  if (!currentImage.isNull()) {
    int x = qMax(0, (width() - currentImage.width()) / 2);
    int y = qMax(0, (height() - currentImage.height()) / 2);

    painter.drawImage(QPoint(x, y), currentImage);
  }
}

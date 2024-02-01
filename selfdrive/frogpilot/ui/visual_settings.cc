#include "selfdrive/frogpilot/ui/visual_settings.h"
#include "selfdrive/ui/ui.h"

FrogPilotVisualsPanel::FrogPilotVisualsPanel(SettingsWindow *parent) : FrogPilotListWidget(parent) {
  const std::vector<std::tuple<QString, QString, QString, QString>> visualToggles {
    {"CustomTheme", "自訂外觀主題", "啟動後使用自訂外觀.", "../frogpilot/assets/wheel_images/frog.png"},
    {"CustomColors", "  顏色", "使用自訂配色方案替換庫存 openpilot 顏色", ""},
    {"CustomIcons", "  圖示", "用自訂圖標包替換庫存 openpilot 圖標", ""},
    {"CustomSignals", "  訊號", "啟用自訂方向燈動畫", ""},
    {"CustomSounds", "  聲音", "用自訂聲音包替換庫存 openpilot 聲音", ""},

    {"CameraView", "相機視圖", "為 UI 設定您首選的相機視圖。此切換純粹是裝飾性的，不會影響openpilot 對其他相機的使用.", "../frogpilot/assets/toggle_icons/icon_camera.png"},
    {"Compass", "指南針", "畫面中添加指南針，顯示您的行駛方位.", "../frogpilot/assets/toggle_icons/icon_compass.png"},

    {"CustomUI", "自定義道路畫面", "定義自己喜歡的道路介面.", "../assets/offroad/icon_road.png"},
    {"AdjacentPath", "  相鄰路徑", "顯示汽車左側和右側的路徑，可視化模型偵測車道的位置.", ""},
    {"BlindSpotPath", "  盲點路徑", "當附近偵測到另一輛車時，將用紅色路徑視覺化您的盲點.", ""},
    {"ShowFPS", "  顯示 FPS", "顯示道路 UI 的每秒幀數 (FPS)，以監控系統效能.", ""},
    {"LeadInfo", "  前車資訊", "獲取有關前方車輛的詳細信息，包括速度和距離，以及跟隨距離背後的邏輯.", ""},
    {"RoadNameUI", "  道路名稱", "在螢幕底部查看您所在道路的名稱。來源自 OpenStreetMap.", ""},
    {"UseVienna", "使用維也納限速標誌", "使用維也納（歐盟）限速樣式標誌，而不是 MUTCD（美國）).", ""},

    {"DriverCamera", "倒車顯示駕駛鏡頭", "當您換至倒車檔時顯示駕駛者的攝影機畫面.", "../assets/img_driver_face_static.png"},
    {"GreenLightAlert", "綠燈提醒", "當交通燈由紅變綠時收到警報.", "../frogpilot/assets/toggle_icons/icon_green_light.png"},

    {"ModelUI", "路徑外觀", "個性化模型的可視化在螢幕上的顯示方式.", "../assets/offroad/icon_calibration.png"},
    {"AccelerationPath", "  加速路徑", "使用顏色編碼的路徑可視化汽車的預期加速或減速.", ""},
    {"LaneLinesWidth", "  車道寬", "調整顯示器上車道線的視覺粗細.\n\nDefault matches the MUTCD average of 4 inches.", ""},
    {"PathEdgeWidth", "  路徑邊寬", "自定義顯示當前駕駛狀態的路徑邊緣寬度。預設為總路徑的 20%。\n\n藍色 =導航\n\n淺藍色 =全時置中\n綠色 = 默認使用“FrogPilot 顏色”\n淺綠色 = 預設使用原始顏色\n橙色 = 實驗模式啟動 \n黃色 = 條件模式", ""},
    {"PathWidth", "  路徑寬", "自定義路徑寬度。\n\n預設為 skoda kodiaq 的寬度.", ""},
    {"RoadEdgesWidth", "  道路邊寬", "自定義道路邊緣寬度。\n\n預設值為 MUTCD 平均車道線寬度 4 英寸.", ""},
    {"UnlimitedLength", "  “無限”道路畫面長度", "將路徑、車道線和道路邊緣的顯示器擴展到系統可以偵測的範圍內，提供更廣闊的前方道路視野.", ""},

    {"QOLVisuals", "優化體驗", "各種控制細項的調整可改善您的openpilot體驗.", "../frogpilot/assets/toggle_icons/quality_of_life.png"},
    {"DriveStats", "主畫面中的統計訊息", "在主畫面中顯示裝置的統計資訊.", ""},
    {"HideSpeed", "隱藏速度", "隱藏介面中的速度指示器.", ""},
    {"ShowSLCOffset", "顯示速度限制偏移", "使用「速度限制控制器」時，顯示與道路 UI 中的速度限制分開的速度限制偏移.", ""},

    {"RandomEvents", "隨機事件", "在某些駕駛條件下使用隨機事件帶來的一點樂趣.", "../frogpilot/assets/toggle_icons/icon_random.png"},
    {"ScreenBrightness", "螢幕亮度", "自行設定螢幕亮度或使用預設自動亮度設置.", "../frogpilot/assets/toggle_icons/icon_light.png"},
    {"SilentMode", "靜音模式", "關閉所有聲音保持完全靜音的運作.", "../frogpilot/assets/toggle_icons/icon_mute.png"},
    {"WheelIcon", "方向盤圖示", "用自定義圖示替換 openpilot 方向盤圖標.", "../assets/offroad/icon_openpilot.png"},
  };

  for (const auto &[param, title, desc, icon] : visualToggles) {
    ParamControl *toggle;

    if (param == "CameraView") {
      std::vector<QString> cameraOptions{tr("自動"), tr("標準"), tr("廣角"), tr("司機")};
      FrogPilotButtonParamControl *preferredCamera = new FrogPilotButtonParamControl(param, title, desc, icon, cameraOptions);
      toggle = preferredCamera;

    } else if (param == "CustomTheme") {
      FrogPilotParamManageControl *customThemeToggle = new FrogPilotParamManageControl(param, title, desc, icon, this);
      QObject::connect(customThemeToggle, &FrogPilotParamManageControl::manageButtonClicked, this, [this]() {
        parentToggleClicked();
        for (auto &[key, toggle] : toggles) {
          toggle->setVisible(customThemeKeys.find(key.c_str()) != customThemeKeys.end());
        }
      });
      toggle = customThemeToggle;
    } else if (param == "CustomColors" || param == "CustomIcons" || param == "CustomSignals" || param == "CustomSounds") {
      std::vector<QString> themeOptions{tr("原始"), tr("Frog"), tr("Tesla"), tr("Stalin")};
      FrogPilotButtonParamControl *themeSelection = new FrogPilotButtonParamControl(param, title, desc, icon, themeOptions);
      toggle = themeSelection;

      if (param == "CustomSounds") {
        QObject::connect(static_cast<FrogPilotButtonParamControl*>(toggle), &FrogPilotButtonParamControl::buttonClicked, [this](int id) {
          if (id == 1) {
            if (FrogPilotConfirmationDialog::yesorno("Do you want to enable the bonus 'Goat' sound effect?", this)) {
              params.putBool("GoatScream", true);
            } else {
              params.putBool("GoatScream", false);
            }
          }
        });
      }

    } else if (param == "CustomUI") {
      FrogPilotParamManageControl *customUIToggle = new FrogPilotParamManageControl(param, title, desc, icon, this);
      QObject::connect(customUIToggle, &FrogPilotParamManageControl::manageButtonClicked, this, [this]() {
        parentToggleClicked();
        for (auto &[key, toggle] : toggles) {
          toggle->setVisible(customOnroadUIKeys.find(key.c_str()) != customOnroadUIKeys.end());
        }
      });
      toggle = customUIToggle;
    } else if (param == "LeadInfo") {
      std::vector<QString> leadInfoToggles{tr("使用 SI")};
      std::vector<QString> leadInfoToggleNames{tr("使用 SI 值")};
      toggle = new FrogPilotParamToggleControl(param, title, desc, icon, leadInfoToggles, leadInfoToggleNames);

    } else if (param == "ModelUI") {
      FrogPilotParamManageControl *modelUIToggle = new FrogPilotParamManageControl(param, title, desc, icon, this);
      QObject::connect(modelUIToggle, &FrogPilotParamManageControl::manageButtonClicked, this, [this]() {
        parentToggleClicked();
        for (auto &[key, toggle] : toggles) {
          toggle->setVisible(modelUIKeys.find(key.c_str()) != modelUIKeys.end());
        }
      });
      toggle = modelUIToggle;
    } else if (param == "LaneLinesWidth" || param == "RoadEdgesWidth") {
      toggle = new FrogPilotParamValueControl(param, title, desc, icon, 0, 24, std::map<int, QString>(), this, false, " inches");
    } else if (param == "PathEdgeWidth") {
      toggle = new FrogPilotParamValueControl(param, title, desc, icon, 0, 100, std::map<int, QString>(), this, false, "%");
    } else if (param == "PathWidth") {
      toggle = new FrogPilotParamValueControl(param, title, desc, icon, 0, 100, std::map<int, QString>(), this, false, " feet", 10);

    } else if (param == "QOLVisuals") {
      FrogPilotParamManageControl *qolToggle = new FrogPilotParamManageControl(param, title, desc, icon, this);
      QObject::connect(qolToggle, &FrogPilotParamManageControl::manageButtonClicked, this, [this]() {
        parentToggleClicked();
        for (auto &[key, toggle] : toggles) {
          toggle->setVisible(qolKeys.find(key.c_str()) != qolKeys.end());
        }
      });
      toggle = qolToggle;

    } else if (param == "ScreenBrightness") {
      std::map<int, QString> brightnessLabels;
      for (int i = 0; i <= 101; ++i) {
        brightnessLabels[i] = i == 0 ? "關閉" : i == 101 ? "自動" : QString::number(i) + "%";
      }
      toggle = new FrogPilotParamValueControl(param, title, desc, icon, 0, 101, brightnessLabels, this, false);

    } else if (param == "WheelIcon") {
      std::vector<QString> wheelToggles{"方向盤"};
      std::vector<QString> wheelToggleNames{tr("旋轉")};
      std::map<int, QString> steeringWheelLabels = {{0, "原始"}, {1, "Lexus"}, {2, "Toyota"}, {3, "Frog"}, {4, "Rocket"}, {5, "Hyundai"}, {6, "Stalin"}};
      toggle = new FrogPilotParamValueToggleControl(param, title, desc, icon, 0, 6, steeringWheelLabels, this, true, "", 1, wheelToggles, wheelToggleNames);

    } else {
      toggle = new ParamControl(param, title, desc, icon, this);
    }

    addItem(toggle);
    toggles[param.toStdString()] = toggle;

    QObject::connect(toggle, &ToggleControl::toggleFlipped, [this]() {
      updateToggles();
    });

    QObject::connect(static_cast<FrogPilotParamValueControl*>(toggle), &FrogPilotParamValueControl::buttonPressed, [this]() {
      updateToggles();
    });

    QObject::connect(toggle, &AbstractControl::showDescriptionEvent, [this]() {
      update();
    });

    QObject::connect(static_cast<FrogPilotParamManageControl*>(toggle), &FrogPilotParamManageControl::manageButtonClicked, [this]() {
      update();
    });
  }

  std::set<std::string> rebootKeys = {"DriveStats"};
  for (const std::string &key : rebootKeys) {
    QObject::connect(toggles[key], &ToggleControl::toggleFlipped, [this]() {
      if (FrogPilotConfirmationDialog::toggle("Reboot required to take effect.", "Reboot Now", this)) {
        Hardware::reboot();
      }
    });
  }

  customOnroadUIKeys = {"AdjacentPath", "BlindSpotPath", "ShowFPS", "LeadInfo", "RoadNameUI", "UseVienna"};
  customThemeKeys = {"CustomColors", "CustomIcons", "CustomSignals", "CustomSounds"};
  modelUIKeys = {"AccelerationPath", "LaneLinesWidth", "PathEdgeWidth", "PathWidth", "RoadEdgesWidth", "UnlimitedLength"};
  qolKeys = {"DriveStats", "HideSpeed", "ShowSLCOffset"};

  QObject::connect(device(), &Device::interactiveTimeout, this, &FrogPilotVisualsPanel::hideSubToggles);
  QObject::connect(parent, &SettingsWindow::closeParentToggle, this, &FrogPilotVisualsPanel::hideSubToggles);
  QObject::connect(parent, &SettingsWindow::updateMetric, this, &FrogPilotVisualsPanel::updateMetric);

  hideSubToggles();
  updateMetric();
}

void FrogPilotVisualsPanel::updateToggles() {
  std::thread([this]() {
    paramsMemory.putBool("FrogPilotTogglesUpdated", true);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    paramsMemory.putBool("FrogPilotTogglesUpdated", false);
  }).detach();
}

void FrogPilotVisualsPanel::updateMetric() {
  bool previousIsMetric = isMetric;
  isMetric = params.getBool("IsMetric");

  if (isMetric != previousIsMetric) {
    double distanceConversion = isMetric ? INCH_TO_CM : CM_TO_INCH;
    double speedConversion = isMetric ? FOOT_TO_METER : METER_TO_FOOT;
    params.putInt("LaneLinesWidth", std::nearbyint(params.getInt("LaneLinesWidth") * distanceConversion));
    params.putInt("RoadEdgesWidth", std::nearbyint(params.getInt("RoadEdgesWidth") * distanceConversion));
    params.putInt("PathWidth", std::nearbyint(params.getInt("PathWidth") * speedConversion));
  }

  FrogPilotParamValueControl *laneLinesWidthToggle = static_cast<FrogPilotParamValueControl*>(toggles["LaneLinesWidth"]);
  FrogPilotParamValueControl *roadEdgesWidthToggle = static_cast<FrogPilotParamValueControl*>(toggles["RoadEdgesWidth"]);
  FrogPilotParamValueControl *pathWidthToggle = static_cast<FrogPilotParamValueControl*>(toggles["PathWidth"]);

  if (isMetric) {
    laneLinesWidthToggle->setDescription("自訂車道線寬度。\n\n預設匹配平均值 10 厘米.");
    roadEdgesWidthToggle->setDescription("自訂道路邊緣寬度。\n\n預設為平均車道線寬度 10 公分的 1/2.");

    laneLinesWidthToggle->updateControl(0, 60, " 公分");
    roadEdgesWidthToggle->updateControl(0, 60, " 公分");
    pathWidthToggle->updateControl(0, 30, " 公尺");
  } else {
    laneLinesWidthToggle->setDescription("Customize the lane line width.\n\nDefault matches the MUTCD average of 4 inches.");
    roadEdgesWidthToggle->setDescription("Customize the road edges width.\n\nDefault is 1/2 of the MUTCD average lane line width of 4 inches.");

    laneLinesWidthToggle->updateControl(0, 24, " inches");
    roadEdgesWidthToggle->updateControl(0, 24, " inches");
    pathWidthToggle->updateControl(0, 100, " feet");
  }

  laneLinesWidthToggle->refresh();
  roadEdgesWidthToggle->refresh();

  previousIsMetric = isMetric;
}

void FrogPilotVisualsPanel::parentToggleClicked() {
  this->openParentToggle();
}

void FrogPilotVisualsPanel::hideSubToggles() {
  for (auto &[key, toggle] : toggles) {
    bool subToggles = modelUIKeys.find(key.c_str()) != modelUIKeys.end() ||
                      customOnroadUIKeys.find(key.c_str()) != customOnroadUIKeys.end() ||
                      customThemeKeys.find(key.c_str()) != customThemeKeys.end() ||
                      qolKeys.find(key.c_str()) != qolKeys.end();
    toggle->setVisible(!subToggles);
  }

  this->closeParentToggle();
}

void FrogPilotVisualsPanel::hideEvent(QHideEvent *event) {
  hideSubToggles();
}

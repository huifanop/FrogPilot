#include "selfdrive/frogpilot/ui/visual_settings.h"

FrogPilotVisualsPanel::FrogPilotVisualsPanel(SettingsWindow *parent) : FrogPilotListWidget(parent) {
  const std::vector<std::tuple<QString, QString, QString, QString>> visualToggles {
    {"CustomTheme", "自訂外觀主題", "啟動後使用自訂外觀.", "../frogpilot/assets/wheel_images/frog.png"},
    {"HolidayThemes", "  節日主題", "openpilot 主題會根據當前/即將到來的假期而變化。小假期持續一天，大假期（復活節、聖誕節、萬聖節等）持續一周.", ""},
    {"CustomColors", "  顏色", "使用自訂配色方案替換庫存 openpilot 顏色", ""},
    {"CustomIcons", "  圖示", "用自訂圖標包替換庫存 openpilot 圖標", ""},
    {"CustomSignals", "  訊號", "啟用自訂方向燈動畫", ""},
    {"CustomSounds", "  聲音", "用自訂聲音包替換庫存 openpilot 聲音", ""},

    {"AlertVolumeControl", "警報音量控制", "控制 openpilot 中每個聲音的音量級別.", "../frogpilot/assets/toggle_icons/icon_mute.png"},
    {"DisengageVolume", "失效音量", "相關提醒:\n\n巡航停用\n手煞車\n煞車\n速度太低", ""},
    {"EngageVolume", "使用音量", "相關提醒:\n\nNNFF 扭矩控制器已加載", ""},
    {"PromptVolume", "提示音量", "相關提醒:\n\n盲點偵測到汽車\n綠燈提醒\n速度太低\n轉向低於“X”時不可用\n控制，轉彎超出轉向極限", ""},
    {"PromptDistractedVolume", "注意力分散的音量", "相關提醒:\n\n請注意，司機分心\n觸摸方向盤，駕駛反應遲鈍", ""},
    {"RefuseVolume", "拒絕音量", "相關提醒:\n\nopenpilot 不可用", ""},
    {"WarningSoftVolume", "警告音量", "相關提醒:\n\n煞車！，碰撞危險\n立即控制", ""},
    {"WarningImmediateVolume", "警告即時音量", "相關提醒:\n\n立即脫離，駕駛分心\n立即脫離，駕駛員沒有反應", ""},

    {"CameraView", "相機視圖", "為 UI 設定您首選的相機視圖。此切換純粹是裝飾性的，不會影響openpilot 對其他相機的使用.", "../frogpilot/assets/toggle_icons/icon_camera.png"},
    {"Compass", "指南針", "畫面中添加指南針，顯示您的行駛方位.", "../frogpilot/assets/toggle_icons/icon_compass.png"},

    {"CustomAlerts", "自訂警報", "針對各種邏輯或情況變化啟用自訂警報.", "../frogpilot/assets/toggle_icons/icon_green_light.png"},
    {"GreenLightAlert", "綠燈提醒", "當交通燈由紅變綠時收到警報.", "../frogpilot/assets/toggle_icons/icon_green_light.png"},
    {"LeadDepartingAlert", "  前車遠離警告", "當您處於靜止狀態時您的領頭車輛開始出發時收到警報.", ""},
    {"LoudBlindspotAlert", "  大聲盲點警報", "當嘗試變換車道時在盲點偵測到車輛時，啟用更響亮的警報.", ""},
    {"SpeedLimitChangedAlert", "  速度限制更改警報", "每當當前速度限制發生變化時觸發警報.", ""},

    {"CustomUI", "自定義道路畫面", "定義自己喜歡的道路介面.", "../assets/offroad/icon_road.png"},
    {"AccelerationPath", "  加速路徑", "使用顏色編碼的路徑可視化汽車的預期加速或減速.", ""},
    {"AdjacentPath", "  相鄰路徑", "顯示汽車左側和右側的路徑，可視化模型偵測車道的位置.", ""},
    {"BlindSpotPath", "  盲點路徑", "當附近偵測到另一輛車時，將用紅色路徑視覺化您的盲點.", ""},
    {"FPSCounter", "  顯示 FPS", "顯示道路 UI 的每秒幀數 (FPS)，以監控系統效能.", ""},
    {"LeadInfo", "  前車資訊", "獲取有關前方車輛的詳細信息，包括速度和距離，以及跟隨距離背後的邏輯.", ""},
    {"PedalsOnUI", "  踏板被踩下", "在方向盤圖示下方的道路 UI 上顯示正在踩下的踏板.", ""},
    {"RoadNameUI", "  道路名稱", "在螢幕底部查看您所在道路的名稱。來源自 OpenStreetMap.", ""},

    {"DriverCamera", "倒車顯示駕駛鏡頭", "當您換至倒車檔時顯示駕駛者的攝影機畫面.", "../assets/img_driver_face_static.png"},

    {"ModelUI", "路徑外觀", "個性化模型的可視化在螢幕上的顯示方式.", "../assets/offroad/icon_calibration.png"},
    {"DynamicPathWidth", "  動態路徑寬度", "根據 openpilot 目前的接合狀態動態調整路徑寬度.", ""},
    {"HideLeadMarker", "  隱藏引導標記", "從道路 UI 中隱藏領先標記.", ""},
    {"LaneLinesWidth", "  車道寬", "調整顯示器上車道線的視覺粗細.\n\nDefault matches the MUTCD average of 4 inches.", ""},
    {"PathEdgeWidth", "  路徑邊寬", "自定義顯示當前駕駛狀態的路徑邊緣寬度。預設為總路徑的 20%。\n\n藍色 =導航\n\n淺藍色 =全時置中\n綠色 = 默認使用“FrogPilot 顏色”\n淺綠色 = 預設使用原始顏色\n橙色 = 實驗模式啟動 \n黃色 = 條件模式", ""},
    {"PathWidth", "  路徑寬", "自定義路徑寬度。\n\n預設為 skoda kodiaq 的寬度.", ""},
    {"RoadEdgesWidth", "  道路邊寬", "自定義道路邊緣寬度。\n\n預設值為 MUTCD 平均車道線寬度 4 英寸.", ""},
    {"UnlimitedLength", "  “無限”道路畫面長度", "將路徑、車道線和道路邊緣的顯示器擴展到系統可以偵測的範圍內，提供更廣闊的前方道路視野.", ""},

    {"NumericalTemp", "數位溫度顯示", "將「良好」、「正常」和「高」溫度狀態替換為基於記憶體、CPU 和 GPU 之間最高溫度的數位溫度計.", "../frogpilot/assets/toggle_icons/icon_temperature.png"},

    {"QOLVisuals", "優化體驗", "各種控制細項的調整可改善您的openpilot體驗.", "../frogpilot/assets/toggle_icons/quality_of_life.png"},
    {"DriveStats", "主畫面中的統計訊息", "在主畫面中顯示裝置的統計資訊.", ""},
    {"FullMap", "全尺寸地圖", "全螢幕顯示地圖導航畫面.", ""},
    {"HideSpeed", "隱藏速度", "隱藏介面中的速度指示器.", ""},
    {"MapStyle", "地圖樣式", "使用自訂地圖樣式用於“在 openpilot 上導航”.", ""},
    {"WheelSpeed", "使用輪速", "使用輪速度量而不是人工速度.", ""},

    {"RandomEvents", "隨機事件", "在某些駕駛條件下使用隨機事件帶來的一點樂趣.", "../frogpilot/assets/toggle_icons/icon_random.png"},

    {"ScreenManagement", "螢幕管理", "管理螢幕的亮度、逾時設定並隱藏特定的道路 UI 元素。", "../frogpilot/assets/toggle_icons/icon_light.png"},
    {"HideUIElements", "  隱藏使用者介面元素", "從道路畫面中隱藏選定的 UI 元素.", ""},
    {"ScreenBrightness", "  螢幕亮度", "自行設定螢幕亮度或使用預設自動亮度設置.", "../frogpilot/assets/toggle_icons/icon_light.png"},
    {"ScreenBrightnessOnroad", "  螢幕亮度（路上）", "在路上自訂您的螢幕亮度.", ""},
    {"ScreenRecorder", "  螢幕錄影機", "啟用螢幕錄影按鈕來錄製螢幕.", ""},
    {"ScreenTimeout", "  螢幕超時", "自訂螢幕關閉所需的時間.", ""},
    {"ScreenTimeoutOnroad", "  螢幕超時（公路）", "自訂上路後螢幕關閉的時間.", ""},
    {"StandbyMode", "  待機模式", "在路上螢幕超時後關閉螢幕，但在參與狀態變更或觸發重要警報時將其喚醒.", ""},

    {"WheelIcon", "方向盤圖示", "用自定義圖示替換 openpilot 方向盤圖標.", "../assets/offroad/icon_openpilot.png"},
  };

  for (const auto &[param, title, desc, icon] : visualToggles) {
    ParamControl *toggle;

    if (param == "AlertVolumeControl") {
      FrogPilotParamManageControl *alertVolumeControlToggle = new FrogPilotParamManageControl(param, title, desc, icon, this);
      QObject::connect(alertVolumeControlToggle, &FrogPilotParamManageControl::manageButtonClicked, this, [this]() {
        parentToggleClicked();
        for (auto &[key, toggle] : toggles) {
          toggle->setVisible(alertVolumeControlKeys.find(key.c_str()) != alertVolumeControlKeys.end());
        }
      });
      toggle = alertVolumeControlToggle;
    } else if (alertVolumeControlKeys.find(param) != alertVolumeControlKeys.end()) {
      if (param == "WarningImmediateVolume") {
        toggle = new FrogPilotParamValueControl(param, title, desc, icon, 25, 100, std::map<int, QString>(), this, false, "%");
      } else {
        toggle = new FrogPilotParamValueControl(param, title, desc, icon, 0, 100, std::map<int, QString>(), this, false, "%");
      }

    } else if (param == "CameraView") {
      std::vector<QString> cameraOptions{tr("自動"), tr("標準"), tr("廣角"), tr("駕駛")};
      FrogPilotButtonParamControl *preferredCamera = new FrogPilotButtonParamControl(param, title, desc, icon, cameraOptions);
      toggle = preferredCamera;

    } else if (param == "CustomAlerts") {
      FrogPilotParamManageControl *customAlertsToggle = new FrogPilotParamManageControl(param, title, desc, icon, this);
      QObject::connect(customAlertsToggle, &FrogPilotParamManageControl::manageButtonClicked, this, [this]() {
        parentToggleClicked();
        for (auto &[key, toggle] : toggles) {
          toggle->setVisible(customAlertsKeys .find(key.c_str()) != customAlertsKeys .end());
        }
      });
      toggle = customAlertsToggle;

    } else if (param == "CustomTheme") {
      FrogPilotParamManageControl *customThemeToggle = new FrogPilotParamManageControl(param, title, desc, icon, this);
      QObject::connect(customThemeToggle, &FrogPilotParamManageControl::manageButtonClicked, this, [this]() {
        parentToggleClicked();
        for (auto &[key, toggle] : toggles) {
          toggle->setVisible(customThemeKeys.find(key.c_str()) != customThemeKeys.end());
        }
      });
      toggle = customThemeToggle;
    } else if (customThemeKeys.find(param) != customThemeKeys.end() && param != "HolidayThemes") {
      std::vector<QString> themeOptions{tr("原始"), tr("Frog"), tr("Tesla"), tr("Stalin")};
      FrogPilotButtonParamControl *themeSelection = new FrogPilotButtonParamControl(param, title, desc, icon, themeOptions);
      toggle = themeSelection;

      if (param == "CustomSounds") {
        QObject::connect(static_cast<FrogPilotButtonParamControl*>(toggle), &FrogPilotButtonParamControl::buttonClicked, [this](int id) {
          if (id == 1) {
            if (FrogPilotConfirmationDialog::yesorno("Do you want to enable the bonus 'Goat' sound effect?", this)) {
              params.putBoolNonBlocking("GoatScream", true);
            } else {
              params.putBoolNonBlocking("GoatScream", false);
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
      std::vector<QString> leadInfoToggles{"UseSI"};
      std::vector<QString> leadInfoToggleNames{tr("Use SI Values")};
      toggle = new FrogPilotParamToggleControl(param, title, desc, icon, leadInfoToggles, leadInfoToggleNames);
    } else if (param == "AdjacentPath") {
      std::vector<QString> adjacentPathToggles{"AdjacentPathMetrics"};
      std::vector<QString> adjacentPathToggleNames{tr("顯示指標")};
      toggle = new FrogPilotParamToggleControl(param, title, desc, icon, adjacentPathToggles, adjacentPathToggleNames);

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

    } else if (param == "NumericalTemp") {
      std::vector<QString> temperatureToggles{"Fahrenheit"};
      std::vector<QString> temperatureToggleNames{tr("Fahrenheit")};
      toggle = new FrogPilotParamToggleControl(param, title, desc, icon, temperatureToggles, temperatureToggleNames);

    } else if (param == "QOLVisuals") {
      FrogPilotParamManageControl *qolToggle = new FrogPilotParamManageControl(param, title, desc, icon, this);
      QObject::connect(qolToggle, &FrogPilotParamManageControl::manageButtonClicked, this, [this]() {
        parentToggleClicked();
        for (auto &[key, toggle] : toggles) {
          toggle->setVisible(qolKeys.find(key.c_str()) != qolKeys.end());
        }
        mapStyleButton->setVisible(true);
      });
      toggle = qolToggle;
    } else if (param == "HideSpeed") {
      std::vector<QString> hideSpeedToggles{"HideSpeedUI"};
      std::vector<QString> hideSpeedToggleNames{tr("Control Via UI")};
      toggle = new FrogPilotParamToggleControl(param, title, desc, icon, hideSpeedToggles, hideSpeedToggleNames);
    } else if (param == "MapStyle") {
      QMap<int, QString> styleMap = {
        {0, tr("Stock openpilot")},
        {1, tr("Mapbox Streets")},
        {2, tr("Mapbox Outdoors")},
        {3, tr("Mapbox Light")},
        {4, tr("Mapbox Dark")},
        {5, tr("Mapbox Satellite")},
        {6, tr("Mapbox Satellite Streets")},
        {7, tr("Mapbox Navigation Day")},
        {8, tr("Mapbox Navigation Night")},
        {9, tr("Mapbox Traffic Night")},
        {10, tr("mike854's (Satellite hybrid)")},
      };

      QStringList styles = styleMap.values();

      mapStyleButton = new ButtonControl(title, tr("SELECT"), desc);
      QObject::connect(mapStyleButton, &ButtonControl::clicked, this, [this, styleMap]() {
        QStringList styles = styleMap.values();
        QString selection = MultiOptionDialog::getSelection(tr("Select a map style"), styles, "", this);
        if (!selection.isEmpty()) {
          int selectedStyle = styleMap.key(selection);
          params.putInt("MapStyle", selectedStyle);
          mapStyleButton->setValue(selection);
          updateToggles();
        }
      });

      int currentStyle = params.getInt("MapStyle");
      mapStyleButton->setValue(styleMap[currentStyle]);

      addItem(mapStyleButton);

    } else if (param == "ScreenManagement") {
      FrogPilotParamManageControl *screenToggle = new FrogPilotParamManageControl(param, title, desc, icon, this);
      QObject::connect(screenToggle, &FrogPilotParamManageControl::manageButtonClicked, this, [this]() {
        parentToggleClicked();
        for (auto &[key, toggle] : toggles) {
          toggle->setVisible(screenKeys.find(key.c_str()) != screenKeys.end());
        }
      });
      toggle = screenToggle;
    } else if (param == "HideUIElements") {
      std::vector<QString> uiElementsToggles{"HideAlerts", "HideMapIcon", "HideMaxSpeed"};
      std::vector<QString> uiElementsToggleNames{tr("警告"), tr("導航"), tr("速限")};
      toggle = new FrogPilotParamToggleControl(param, title, desc, icon, uiElementsToggles, uiElementsToggleNames);
    } else if (param == "ScreenBrightness" || param == "ScreenBrightnessOnroad") {
      std::map<int, QString> brightnessLabels;
      for (int i = 0; i <= 101; ++i) {
        brightnessLabels[i] = i == 0 ? "Screen Off" : i == 101 ? "Auto" : QString::number(i) + "%";
      }
      toggle = new FrogPilotParamValueControl(param, title, desc, icon, 0, 101, brightnessLabels, this, false);
    } else if (param == "ScreenTimeout" || param == "ScreenTimeoutOnroad") {
      toggle = new FrogPilotParamValueControl(param, title, desc, icon, 5, 60, std::map<int, QString>(), this, false, " seconds");

    } else if (param == "WheelIcon") {
      std::vector<QString> wheelToggles{"RotatingWheel"};
      std::vector<QString> wheelToggleNames{tr("旋轉")};
      std::map<int, QString> steeringWheelLabels = {{-1, "None"}, {0, "Stock"}, {1, "Lexus"}, {2, "Toyota"}, {3, "Frog"}, {4, "Rocket"}, {5, "Hyundai"}, {6, "Stalin"}};
      toggle = new FrogPilotParamValueToggleControl(param, title, desc, icon, -1, 6, steeringWheelLabels, this, true, "", 1, wheelToggles, wheelToggleNames);

    } else {
      toggle = new ParamControl(param, title, desc, icon, this);
    }

    addItem(toggle);
    toggles[param.toStdString()] = toggle;

    QObject::connect(toggle, &ToggleControl::toggleFlipped, [this]() {
      updateToggles();
    });

    QObject::connect(static_cast<FrogPilotButtonParamControl*>(toggle), &FrogPilotButtonParamControl::buttonClicked, [this]() {
      updateToggles();
    });

    QObject::connect(static_cast<FrogPilotParamValueControl*>(toggle), &FrogPilotParamValueControl::valueChanged, [this]() {
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
    QObject::connect(toggles[key], &ToggleControl::toggleFlipped, [this, key]() {
      if (started || key == "DriveStats") {
        if (FrogPilotConfirmationDialog::toggle("需要重新啟動才能生效.", "馬上重啟", this)) {
          Hardware::soft_reboot();
        }
      }
    });
  }

  QObject::connect(device(), &Device::interactiveTimeout, this, &FrogPilotVisualsPanel::hideSubToggles);
  QObject::connect(parent, &SettingsWindow::closeParentToggle, this, &FrogPilotVisualsPanel::hideSubToggles);
  QObject::connect(parent, &SettingsWindow::closeSubParentToggle, this, &FrogPilotVisualsPanel::hideSubSubToggles);
  QObject::connect(parent, &SettingsWindow::updateMetric, this, &FrogPilotVisualsPanel::updateMetric);
  QObject::connect(uiState(), &UIState::uiUpdate, this, &FrogPilotVisualsPanel::updateState);

  hideSubToggles();
  updateMetric();
}

void FrogPilotVisualsPanel::updateState(const UIState &s) {
  started = s.scene.started;
}

void FrogPilotVisualsPanel::updateToggles() {
  std::thread([this]() {
    paramsMemory.putBool("FrogPilotTogglesUpdated", true);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    paramsMemory.putBool("FrogPilotTogglesUpdated", false);
  }).detach();
}

void FrogPilotVisualsPanel::updateMetric() {
  bool previousIsMetric = isMetric;
  isMetric = params.getBool("IsMetric");

  if (isMetric != previousIsMetric) {
    double distanceConversion = isMetric ? INCH_TO_CM : CM_TO_INCH;
    double speedConversion = isMetric ? FOOT_TO_METER : METER_TO_FOOT;
    params.putIntNonBlocking("LaneLinesWidth", std::nearbyint(params.getInt("LaneLinesWidth") * distanceConversion));
    params.putIntNonBlocking("RoadEdgesWidth", std::nearbyint(params.getInt("RoadEdgesWidth") * distanceConversion));
    params.putIntNonBlocking("PathWidth", std::nearbyint(params.getInt("PathWidth") * speedConversion));
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
    pathWidthToggle->updateControl(0, 100, " feet", 10);
  }

  laneLinesWidthToggle->refresh();
  roadEdgesWidthToggle->refresh();

  previousIsMetric = isMetric;
}

void FrogPilotVisualsPanel::parentToggleClicked() {
  mapStyleButton->setVisible(false);

  openParentToggle();
}

void FrogPilotVisualsPanel::subParentToggleClicked() {
  mapStyleButton->setVisible(false);

  openSubParentToggle();
}

void FrogPilotVisualsPanel::hideSubToggles() {
  mapStyleButton->setVisible(false);

  for (auto &[key, toggle] : toggles) {
    bool subToggles = alertVolumeControlKeys.find(key.c_str()) != alertVolumeControlKeys.end() ||
                      customAlertsKeys.find(key.c_str()) != customAlertsKeys.end() ||
                      customOnroadUIKeys.find(key.c_str()) != customOnroadUIKeys.end() ||
                      customThemeKeys.find(key.c_str()) != customThemeKeys.end() ||
                      modelUIKeys.find(key.c_str()) != modelUIKeys.end() ||
                      qolKeys.find(key.c_str()) != qolKeys.end() ||
                      screenKeys.find(key.c_str()) != screenKeys.end();
    toggle->setVisible(!subToggles);
  }

  closeParentToggle();
}

void FrogPilotVisualsPanel::hideSubSubToggles() {
  for (auto &[key, toggle] : toggles) {
    bool isVisible = false;
    toggle->setVisible(isVisible);
  }

  closeSubParentToggle();
  update();
}

void FrogPilotVisualsPanel::hideEvent(QHideEvent *event) {
  hideSubToggles();
}

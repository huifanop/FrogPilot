#include <cmath>
#include <filesystem>
#include <unordered_set>

#include "selfdrive/frogpilot/ui/visual_settings.h"
#include "selfdrive/ui/ui.h"

FrogPilotVisualsPanel::FrogPilotVisualsPanel(SettingsWindow *parent) : ListWidget(parent) {
  const std::vector<std::tuple<QString, QString, QString, QString>> visualToggles {
    {"CustomTheme", "自訂外觀主題", "啟動後使用自訂外觀.", "../frogpilot/assets/wheel_images/frog.png"},
    {"CustomColors", "  顏色", "使用自訂配色方案替換庫存 openpilot 顏色", ""},
    {"CustomIcons", "  圖示", "用自訂圖標包替換庫存 openpilot 圖標", ""},
    {"CustomSignals", "  訊號", "啟用自訂方向燈動畫", ""},
    {"CustomSounds", "  聲音", "用自訂聲音包替換庫存 openpilot 聲音", ""},

    {"CameraView", "相機視圖（僅限外觀）", "為 UI 設定您首選的相機視圖。此切換純粹是裝飾性的，不會影響openpilot 對其他相機的使用.", "../frogpilot/assets/toggle_icons/icon_camera.png"},
    {"Compass", "指南針", "畫面中添加指南針，顯示您的行駛方位.", "../frogpilot/assets/toggle_icons/icon_compass.png"},

    {"CustomUI", "自定義道路畫面", "定義自己喜歡的道路介面.", "../assets/offroad/icon_road.png"},
    {"AdjacentPath", "  相鄰路徑", "顯示汽車左側和右側的路徑，可視化模型偵測車道的位置.", ""},
    {"BlindSpotPath", "  盲點路徑", "當附近偵測到另一輛車時，將用紅色路徑視覺化您的盲點.", ""},
    {"ShowFPS", "  顯示 FPS", "顯示道路 UI 的每秒幀數 (FPS)，以監控系統效能.", ""},
    {"LeadInfo", "  前車資訊", "獲取有關前方車輛的詳細信息，包括速度和距離，以及跟隨距離背後的邏輯.", ""},
    {"RoadNameUI", "  道路名稱", "在螢幕底部查看您所在道路的名稱。來源自 OpenStreetMap.", ""},

    {"DriverCamera", "倒車顯示駕駛鏡頭", "當您換至倒車檔時顯示駕駛者的攝影機畫面.", "../assets/img_driver_face_static.png"},
    {"GreenLightAlert", "綠燈提醒", "當交通燈由紅變綠時收到警報.", "../frogpilot/assets/toggle_icons/icon_green_light.png"},

    {"ModelUI", "路徑外觀", "個性化模型的可視化在螢幕上的顯示方式.", "../assets/offroad/icon_calibration.png"},
    {"AccelerationPath", "  加速路徑", "使用顏色編碼的路徑可視化汽車的預期加速或減速.", ""},
    {"LaneLinesWidth", "  車道寬", "調整顯示器上車道線的視覺粗細.\n\nDefault matches the MUTCD average of 4 inches.", ""},
    {"PathEdgeWidth", "  路徑邊寬", "自定義顯示當前駕駛狀態的路徑邊緣寬度。預設為總路徑的 20%。\n\n藍色 =導航\n\n淺藍色 =全時置中\n綠色 = 默認使用“FrogPilot 顏色”\n淺綠色 = 預設使用原始顏色\n橙色 = 實驗模式啟動 \n黃色 = 條件模式", ""},
    {"PathWidth", "  路徑寬", "自定義路徑寬度。\n\n預設為 skoda kodiaq 的寬度.", ""},
    {"RoadEdgesWidth", "  道路邊寬", "自定義道路邊緣寬度。\n\n預設值為 MUTCD 平均車道線寬度 4 英寸.", ""},
    {"UnlimitedLength", "  “無限”道路畫面長度", "將路徑、車道線和道路邊緣的顯示器擴展到系統可以偵測的範圍內，提供更廣闊的前方道路視野.", ""},

    {"ScreenBrightness", "螢幕亮度", "自行設定螢幕亮度或使用預設自動亮度設置.", "../frogpilot/assets/toggle_icons/icon_light.png"},
    {"SilentMode", "靜音模式", "關閉所有聲音保持完全靜音的運作.", "../frogpilot/assets/toggle_icons/icon_mute.png"},
    {"WheelIcon", "方向盤圖示", "用自定義圖示替換 openpilot 方向盤圖標.", "../assets/offroad/icon_openpilot.png"},
  };

  for (const auto &[param, title, desc, icon] : visualToggles) {
    ParamControl *toggle;

    if (param == "CameraView") {
      toggle = new ParamValueControl(param, title, desc, icon, 0, 3, {{0, "自動"}, {1, "標準"}, {2, "廣角"}, {3, "司機"}}, this, true);

    } else if (param == "CustomTheme") {
      ParamManageControl *customThemeToggle = new ParamManageControl(param, title, desc, icon, this);
      QObject::connect(customThemeToggle, &ParamManageControl::manageButtonClicked, this, [this]() {
        parentToggleClicked();
        for (auto &[key, toggle] : toggles) {
          toggle->setVisible(customThemeKeys.find(key.c_str()) != customThemeKeys.end());
        }
      });
      toggle = customThemeToggle;
    } else if (param == "CustomColors" || param == "CustomIcons" || param == "CustomSignals" || param == "CustomSounds") {
      std::map<int, QString> themeLabels = {{0, "原始"}, {1, "Frog"}, {2, "Tesla"}, {3, "Stalin"}};
      toggle = new ParamValueControl(param, title, desc, icon, 0, 3, themeLabels, this, true);

    } else if (param == "CustomUI") {
      ParamManageControl *customUIToggle = new ParamManageControl(param, title, desc, icon, this);
      QObject::connect(customUIToggle, &ParamManageControl::manageButtonClicked, this, [this]() {
        parentToggleClicked();
        for (auto &[key, toggle] : toggles) {
          toggle->setVisible(customOnroadUIKeys.find(key.c_str()) != customOnroadUIKeys.end());
        }
      });
      toggle = customUIToggle;

    } else if (param == "ModelUI") {
      ParamManageControl *modelUIToggle = new ParamManageControl(param, title, desc, icon, this);
      QObject::connect(modelUIToggle, &ParamManageControl::manageButtonClicked, this, [this]() {
        parentToggleClicked();
        for (auto &[key, toggle] : toggles) {
          toggle->setVisible(modelUIKeys.find(key.c_str()) != modelUIKeys.end());
        }
      });
      toggle = modelUIToggle;
    } else if (param == "LaneLinesWidth" || param == "RoadEdgesWidth") {
      toggle = new ParamValueControl(param, title, desc, icon, 0, 24, std::map<int, QString>(), this, false, " inches");
    } else if (param == "PathEdgeWidth") {
      toggle = new ParamValueControl(param, title, desc, icon, 0, 100, std::map<int, QString>(), this, false, "%");
    } else if (param == "PathWidth") {
      toggle = new ParamValueControl(param, title, desc, icon, 0, 100, std::map<int, QString>(), this, false, " feet", 10);

    } else if (param == "ScreenBrightness") {
      std::map<int, QString> brightnessLabels;
      for (int i = 0; i <= 101; ++i) {
        brightnessLabels[i] = i == 0 ? "關閉" : i == 101 ? "自動" : QString::number(i) + "%";
      }
      toggle = new ParamValueControl(param, title, desc, icon, 0, 101, brightnessLabels, this, false);

    } else if (param == "WheelIcon") {
      std::vector<QString> wheelToggles{tr("旋轉")};
      std::vector<QString> wheelToggleNames{tr("旋轉圖示")};
      std::map<int, QString> steeringWheelLabels = {{0, "原始"}, {1, "Lexus"}, {2, "Toyota"}, {3, "Frog"}, {4, "Rocket"}, {5, "Hyundai"}, {6, "Stalin"}};
      toggle = new ParamValueToggleControl(param, title, desc, icon, 0, 6, steeringWheelLabels, this, true, "", 1, wheelToggles, wheelToggleNames);

    } else {
      toggle = new ParamControl(param, title, desc, icon, this);
    }

    addItem(toggle);
    toggles[param.toStdString()] = toggle;

    QObject::connect(toggle, &ToggleControl::toggleFlipped, [this]() {
      std::thread([this]() {
        paramsMemory.putBool("FrogPilotTogglesUpdated", true);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        paramsMemory.putBool("FrogPilotTogglesUpdated", false);
      }).detach();
    });

    QObject::connect(static_cast<ParamValueControl*>(toggles["ScreenBrightness"]), &ParamValueControl::valueChanged, [](int value) {
      uiState()->scene.screen_brightness = value;
    });

    QObject::connect(static_cast<ParamValueControl*>(toggle), &ParamValueControl::buttonPressed, [this]() {
      std::thread([this]() {
        paramsMemory.putBool("FrogPilotTogglesUpdated", true);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        paramsMemory.putBool("FrogPilotTogglesUpdated", false);
      }).detach();
    });
  }

  customOnroadUIKeys = {"AdjacentPath", "BlindSpotPath", "ShowFPS", "LeadInfo", "RoadNameUI"};
  customThemeKeys = {"CustomColors", "CustomIcons", "CustomSignals", "CustomSounds"};
  modelUIKeys = {"AccelerationPath", "LaneLinesWidth", "PathEdgeWidth", "PathWidth", "RoadEdgesWidth", "UnlimitedLength"};

  QObject::connect(uiState(), &UIState::uiUpdate, this, &FrogPilotVisualsPanel::updateState);

  hideSubToggles();
  setDefaults();
}

void FrogPilotVisualsPanel::updateState() {
  if (isVisible()) {
    if (paramsMemory.getInt("FrogPilotTogglesOpen") == 2) {
      hideSubToggles();
    }
  }

  std::thread([this] {
    static bool checkedOnBoot = false;

    bool previousIsMetric = isMetric;
    isMetric = params.getBool("IsMetric");

    if (checkedOnBoot) {
      if (previousIsMetric == isMetric) return;
    }
    checkedOnBoot = true;

    if (isMetric != previousIsMetric) {
      const double distanceConversion = isMetric ? INCH_TO_CM : CM_TO_INCH;
      const double speedConversion = isMetric ? FOOT_TO_METER : METER_TO_FOOT;
      params.putInt("LaneLinesWidth", std::nearbyint(params.getInt("LaneLinesWidth") * distanceConversion));
      params.putInt("RoadEdgesWidth", std::nearbyint(params.getInt("RoadEdgesWidth") * distanceConversion));
      params.putInt("PathWidth", std::nearbyint(params.getInt("PathWidth") * speedConversion));
    }

    ParamValueControl *laneLinesWidthToggle = static_cast<ParamValueControl*>(toggles["LaneLinesWidth"]);
    ParamValueControl *roadEdgesWidthToggle = static_cast<ParamValueControl*>(toggles["RoadEdgesWidth"]);
    ParamValueControl *pathWidthToggle = static_cast<ParamValueControl*>(toggles["PathWidth"]);

    if (isMetric) {
      laneLinesWidthToggle->setDescription("自訂車道線寬度。\n\n預設匹配平均值 10 厘米.");
      roadEdgesWidthToggle->setDescription("自訂道路邊緣寬度。\n\n預設為平均車道線寬度 10 公分的 1/2.");

      laneLinesWidthToggle->updateControl(0, 60, " 公分");
      roadEdgesWidthToggle->updateControl(0, 60, " 公分");
      pathWidthToggle->updateControl(0, 30, " 米");
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
  }).detach();
}

void FrogPilotVisualsPanel::parentToggleClicked() {
  paramsMemory.putInt("FrogPilotTogglesOpen", 1);
}

void FrogPilotVisualsPanel::hideSubToggles() {
  for (auto &[key, toggle] : toggles) {
    const bool subToggles = modelUIKeys.find(key.c_str()) != modelUIKeys.end() ||
                            customOnroadUIKeys.find(key.c_str()) != customOnroadUIKeys.end() ||
                            customThemeKeys.find(key.c_str()) != customThemeKeys.end();
    toggle->setVisible(!subToggles);
  }
}

void FrogPilotVisualsPanel::hideEvent(QHideEvent *event) {
  paramsMemory.putInt("FrogPilotTogglesOpen", 0);

  hideSubToggles();
}

void FrogPilotVisualsPanel::setDefaults() {
  const bool FrogsGoMoo = params.get("DongleId").substr(0, 3) == "be6";

  const std::map<std::string, std::string> defaultValues {
    {"AccelerationPath", "1"},
    {"AdjacentPath", FrogsGoMoo ? "1" : "1"},
    {"BlindSpotPath", "1"},
    {"CameraView", FrogsGoMoo ? "1" : "2"},
    {"Compass", FrogsGoMoo ? "1" : "0"},
    {"CustomColors", "0"},
    {"CustomIcons", "0"},
    {"CustomSignals", "0"},
    {"CustomSounds", "0"},
    {"CustomTheme", "0"},
    {"CustomUI", "1"},
    {"DriverCamera", "0"},
    {"GreenLightAlert", "1"},
    {"LaneLinesWidth", "4"},
    {"LeadInfo", FrogsGoMoo ? "1" : "0"},
    {"ModelUI", "1"},
    {"NumericalTemp", FrogsGoMoo ? "1" : "1"},
    {"PathEdgeWidth", "20"},
    {"PathWidth", "20"},
    {"RoadEdgesWidth", "2"},
    {"RoadNameUI", "1"},
    {"RotatingWheel", "0"},
    {"ScreenBrightness", "50"},
    {"ShowCPU", FrogsGoMoo ? "1" : "0"},
    {"ShowMemoryUsage", FrogsGoMoo ? "1" : "0"},
    {"ShowFPS", FrogsGoMoo ? "1" : "0"},
    {"Sidebar", FrogsGoMoo ? "1" : "0"},
    {"SilentMode", "0"},
    {"UnlimitedLength", "1"},
    {"WheelIcon", FrogsGoMoo ? "1" : "0"},
  };

  bool rebootRequired = false;
  for (const auto &[key, value] : defaultValues) {
    if (params.get(key).empty()) {
      params.put(key, value);
      rebootRequired = true;
    }
  }

  if (rebootRequired) {
    while (!std::filesystem::exists("/data/openpilot/prebuilt")) {
      std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    Hardware::reboot();
  }
}

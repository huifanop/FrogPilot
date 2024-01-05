#include <cmath>
#include <filesystem>
#include <unordered_set>

#include "selfdrive/frogpilot/ui/control_settings.h"
#include "selfdrive/ui/ui.h"

FrogPilotControlsPanel::FrogPilotControlsPanel(SettingsWindow *parent) : ListWidget(parent) {
  const std::vector<std::tuple<QString, QString, QString, QString>> controlToggles {
    {"AdjustablePersonalities", "駕駛模式", "透過畫面切換駕駛模式.\n\n1 格 = 積極\n2 格 = 標準\n3 格 = 輕鬆", "../frogpilot/assets/toggle_icons/icon_distance.png"},

    {"AlwaysOnLateral", "全時置中模式", "使用剎車或油門踏板時仍保持橫向控制.只有停用“定速”後才能解除.", "../frogpilot/assets/toggle_icons/icon_always_on_lateral.png"},
    {"AlwaysOnLateralMain", "在道路上啟動全時置中模式", "啟用巡航控制時啟動“始終開啟橫向”，無需先啟用 openpilot.", "../frogpilot/assets/toggle_icons/icon_blank.png"},

    {"ConditionalExperimental", "條件式的實驗模式", "根據特定條件自動啟動實驗模式.", "../frogpilot/assets/toggle_icons/icon_conditional.png"},
    {"CECurves", "彎道", "偵測到曲線時切換到“實驗模式”.", ""},
    {"CENavigation", "  導航", "根據導航十字路口、停車標誌等切換到“實驗模式”.", ""},
    {"CESlowerLead", "  低速前車", "當偵測到前方較慢車輛時切換到“實驗模式”.", ""},
    {"CEStopLights", "停止標誌", "當偵測到停車燈或停車標誌時切換到“實驗模式”.", ""},
    {"CESignal", "  方向燈", "當使用低於 55 英里/小時的轉向信號燈時切換到“實驗模式”以幫助轉彎.", ""},

    {"CustomPersonalities", "設定駕駛模式", "根據您的喜好設定駕駛模式細項設定.", "../frogpilot/assets/toggle_icons/icon_custom.png"},
    {"AggressiveFollow", "積極模式時間", "設定積極模式中的跟隨距離.表示跟隨前方車輛的秒數.\n\n預設:1.25 秒.", "../frogpilot/assets/other_images/aggressive.png"},
    {"AggressiveJerk", "Jerk 值", "設定積極模式中的煞車/油門踏板反應能力.數值越高，反應越“輕鬆”.\n\n預設: 0.5.", "../frogpilot/assets/other_images/aggressive.png"},
    {"StandardFollow", "標準模式時間", "設定標準模式中的跟隨距離.表示跟隨前方車輛的秒數..\n\n預設: 1.45 秒.", "../frogpilot/assets/other_images/standard.png"},
    {"StandardJerk", "Jerk 值", "設定標準模式中的煞車/油門踏板反應能力.數值越高，反應越“輕鬆”.\n\n預設: 1.0.", "../frogpilot/assets/other_images/standard.png"},
    {"RelaxedFollow", "輕鬆模式時間", "設定輕鬆模式中的跟隨距離.表示跟隨前方車輛的秒數.\n\n預設: 1.75 秒.", "../frogpilot/assets/other_images/relaxed.png"},
    {"RelaxedJerk", "Jerk 值", "設定輕鬆模式中的煞車/油門踏板反應能力.數值越高，反應越“輕鬆”.\n\n預設: 1.0.", "../frogpilot/assets/other_images/relaxed.png"},

    {"DeviceShutdown", "設備自動關機設定", "設置設備在熄火後自動關閉的時間，以減少能源浪費並防止電池耗盡.", "../frogpilot/assets/toggle_icons/icon_time.png"},
    {"ExperimentalModeViaPress", "利用畫面或方向盤開啟實驗模式", "通過雙擊方向盤上的“車道偏離”/LKAS 按鈕(Toyota/Lexus Only)以啟用或禁用實驗模式，或雙擊營幕覆蓋“條件實驗模式”'.", "../assets/img_experimental_white.svg"},

    {"FireTheBabysitter", "關閉監控", "禁用 openpilot 的一些‘保姆協議’.", "../frogpilot/assets/toggle_icons/icon_babysitter.png"},
    {"NoLogging", "  停用所有日誌記錄", "關閉所有數據追蹤以增強隱私或減少熱負荷.\n\n 警告：此操作將阻止驅動器記錄且資料無法恢復!", ""},
    {"MuteDM", "  駕駛監控", "禁用駕駛員監控.", ""},
    {"MuteDoor", "  車門", "禁用開門警報.", ""},
    {"MuteOverheated", "  系統過熱", "禁用設備過熱警報.", ""},
    {"MuteSeatbelt", "  安全帶", "禁用未扣安全帶的警報.", ""},

    {"LateralTune", "橫向調整", "改變 openpilot 的駕駛方式.", "../frogpilot/assets/toggle_icons/icon_lateral_tune.png"},
    {"AverageCurvature", "  平均期望曲率", "使用 Pfeiferj 的以距離為基準的曲率調整方法來更平滑地處理轉彎.", ""},
    {"NNFF", "  NNFF - 神經網路前饋", "使用Twilsonco's的神經網路前饋扭矩控制系統來獲得更精準的橫向控制.", ""},

    {"LongitudinalTune", "縱向調整", "改變 openpilot 加速和煞車方式.", "../frogpilot/assets/toggle_icons/icon_longitudinal_tune.png"},
    {"AccelerationProfile", "  加速曲線", "將加速度改為運動型或環保型.", ""},
    {"AggressiveAcceleration", "  積極加速跟車", "當有前車可跟隨時起步更加積極的加速.", ""},
    {"SmoothBraking", "  平穩煞車的跟車", "當接近速度較慢的車輛時，煞車行為更加自然.", ""},
    {"StoppingDistance", "  增加停車距離", "增加停車距離，讓停車更舒適.", ""},

    {"Model", "模型選擇", "選擇您喜歡的模型.", "../assets/offroad/icon_calibration.png"},
    {"MTSCEnabled", "地圖彎道速度控制", "根據下載地圖偵測到的預期曲線放慢速度.", "../frogpilot/assets/toggle_icons/icon_speed_map.png"},

    {"NudgelessLaneChange", "自動變換車道", "不需輕推方向盤即可變換車道.", "../frogpilot/assets/toggle_icons/icon_lane.png"},
    {"LaneChangeTime", "  自動變換車道延遲", "設定自動變換車道延遲時間.", ""},
    {"LaneDetection", "  車道檢測", "未偵測到車道時阻止自動變換車道.", ""},
    {"OneLaneChange", "  每次只變換一個車道", "每次啟動方向燈時，僅執行一次自動變換車道.", ""},
    {"PauseLateralOnSignal", "  打方向燈時暫停控制", "使用方向燈期間暫時停用橫向控制.", ""},

    {"SpeedLimitController", "限速控制器", "使用「開放街道地圖」、「在 openpilot 上導航」或汽車儀表板（僅限 TSS2 豐田）自動調整車速以匹配速度限制.", "../assets/offroad/icon_speed_limit.png"},
    {"Offset1", "速限微調 (0-34 mph)", "  速度介於 0-34 mph 的速限微調.", ""},
    {"Offset2", "速限微調 (35-54 mph)", "  速度介於 35-54 mph 的速限微調.", ""},
    {"Offset3", "速限微調 (55-64 mph)", "  速度介於 55-64 mph 的速限微調.", ""},
    {"Offset4", "速限微調 (65-99 mph)", "  速度介於 65-99 mph 的速限微調.", ""},
    {"SLCFallback", "  備援方式", "當導航、OSM 或汽車儀表板中沒有速度限制時，設定您的替代方法.", ""},
    {"SLCPriority", "優先選項", "選擇優先使用的速度限制模式.", ""},

    {"TurnDesires", "意圖轉彎", "打開此選項在低於最低自動換道時速40KMH以下時打方向燈時獲得更精準的轉彎.", "../assets/navigation/direction_continue_right.png"},

    {"VisionTurnControl", "視覺轉向速度控制", "根據路面曲率自動調整車速，轉彎更順暢.", "../frogpilot/assets/toggle_icons/icon_vtc.png"},
    {"CurveSensitivity", "  曲線檢測靈敏度", "設定曲線檢測靈敏度.較高的值會導致較早的反應，較低的值會導致較平滑但較晚的反應.", ""},
    {"TurnAggressiveness", "  轉彎速度積極性", "設定轉彎速度攻擊性.較高的數值會導致較快的轉彎，較低的數值會導致較平緩的轉彎.", ""},
  };

  for (const auto &[param, title, desc, icon] : controlToggles) {
    ParamControl *toggle;

    if (param == "AdjustablePersonalities") {
      toggle = new ParamValueControl(param, title, desc, icon, 0, 3, {{0, "無"}, {1, "方向盤"}, {2, "畫面按鈕"}, {3, "方向盤 + 畫面按鈕"}}, this, true);

    } else if (param == "ConditionalExperimental") {
      ParamManageControl *conditionalExperimentalToggle = new ParamManageControl(param, title, desc, icon, this);
      QObject::connect(conditionalExperimentalToggle, &ParamManageControl::manageButtonClicked, this, [this]() {
        parentToggleClicked();
        conditionalSpeedsImperial->setVisible(!isMetric);
        conditionalSpeedsMetric->setVisible(isMetric);
        for (auto &[key, toggle] : toggles) {
          toggle->setVisible(conditionalExperimentalKeys.find(key.c_str()) != conditionalExperimentalKeys.end());
        }
      });
      toggle = conditionalExperimentalToggle;

    } else if (param == "CECurves") {
      ParamValueControl *CESpeedImperial = new ParamValueControl("CESpeed", "無車時速", "沒有前方車輛時低於此速度切換“實驗模式”.", "", 0, 99, std::map<int, QString>(), this, false, " mph");
      ParamValueControl *CESpeedLeadImperial = new ParamValueControl("CESpeedLead", "有車時速", "有前方車輛時低於此速度切換到“實驗模式”.", "", 0, 99, std::map<int, QString>(), this, false, " mph");
      conditionalSpeedsImperial = new DualParamValueControl(CESpeedImperial, CESpeedLeadImperial, this);
      addItem(conditionalSpeedsImperial);

      ParamValueControl *CESpeedMetric = new ParamValueControl("CESpeed", "無車", "沒有車輛時低於此速度切換“實驗模式”.", "", 0, 99, std::map<int, QString>(), this, false, " kph");
      ParamValueControl *CESpeedLeadMetric = new ParamValueControl("CESpeedLead", "有車", "有車輛時低於此速度切換到“實驗模式”.", "", 0, 99, std::map<int, QString>(), this, false, " kph");
      conditionalSpeedsMetric = new DualParamValueControl(CESpeedMetric, CESpeedLeadMetric, this);
      addItem(conditionalSpeedsMetric);

      std::vector<QString> curveToggles{tr("CECurvesLead")};
      std::vector<QString> curveToggleNames{tr("有前車")};
      toggle = new ParamToggleControl("CECurves", tr("  偵測彎道"), tr("偵測到彎道時切換到“實驗模式”."), "", curveToggles, curveToggleNames);
    } else if (param == "CEStopLights") {
      std::vector<QString> stopLightToggles{tr("停止燈號")};
      std::vector<QString> stopLightToggleNames{tr("有前車")};
      toggle = new ParamToggleControl("CEStopLights", tr("  停車燈和停車標誌"), tr("當偵測到停車燈或停車標誌時切換到“實驗模式”."), "", stopLightToggles, stopLightToggleNames);

    } else if (param == "CustomPersonalities") {
      ParamManageControl *customPersonalitiesToggle = new ParamManageControl(param, title, desc, icon, this);
      QObject::connect(customPersonalitiesToggle, &ParamManageControl::manageButtonClicked, this, [this]() {
        parentToggleClicked();
        for (auto &[key, toggle] : toggles) {
          toggle->setVisible(customPersonalitiesKeys.find(key.c_str()) != customPersonalitiesKeys.end());
        }
      });
      toggle = customPersonalitiesToggle;
    } else if (param == "AggressiveFollow" || param == "StandardFollow" || param == "RelaxedFollow") {
      toggle = new ParamValueControl(param, title, desc, icon, 10, 50, std::map<int, QString>(), this, false, " seconds", 10);
    } else if (param == "AggressiveJerk" || param == "StandardJerk" || param == "RelaxedJerk") {
      toggle = new ParamValueControl(param, title, desc, icon, 1, 50, std::map<int, QString>(), this, false, "", 10);

    } else if (param == "DeviceShutdown") {
      std::map<int, QString> shutdownLabels;
      for (int i = 0; i <= 33; ++i) {
        shutdownLabels[i] = i == 0 ? "立即" : i <= 3 ? QString::number(i * 15) + " 分鐘" : QString::number(i - 3) + (i == 4 ? " 小時" : " 小時");
      }
      toggle = new ParamValueControl(param, title, desc, icon, 0, 33, shutdownLabels, this, false);

    } else if (param == "FireTheBabysitter") {
      ParamManageControl *fireTheBabysitterToggle = new ParamManageControl(param, title, desc, icon, this);
      QObject::connect(fireTheBabysitterToggle, &ParamManageControl::manageButtonClicked, this, [this]() {
        parentToggleClicked();
        for (auto &[key, toggle] : toggles) {
          toggle->setVisible(fireTheBabysitterKeys.find(key.c_str()) != fireTheBabysitterKeys.end());
        }
      });
      toggle = fireTheBabysitterToggle;

    } else if (param == "LateralTune") {
      ParamManageControl *lateralTuneToggle = new ParamManageControl(param, title, desc, icon, this);
      QObject::connect(lateralTuneToggle, &ParamManageControl::manageButtonClicked, this, [this]() {
        parentToggleClicked();
        for (auto &[key, toggle] : toggles) {
          toggle->setVisible(lateralTuneKeys.find(key.c_str()) != lateralTuneKeys.end());
        }
      });
      toggle = lateralTuneToggle;

    } else if (param == "LongitudinalTune") {
      ParamManageControl *longitudinalTuneToggle = new ParamManageControl(param, title, desc, icon, this);
      QObject::connect(longitudinalTuneToggle, &ParamManageControl::manageButtonClicked, this, [this]() {
        parentToggleClicked();
        for (auto &[key, toggle] : toggles) {
          toggle->setVisible(longitudinalTuneKeys.find(key.c_str()) != longitudinalTuneKeys.end());
        }
      });
      toggle = longitudinalTuneToggle;
    } else if (param == "AccelerationProfile") {
      toggle = new ParamValueControl(param, title, desc, icon, 0, 2, {{0, "標準"}, {1, "節能"}, {2, "運動"}}, this, true);
    } else if (param == "StoppingDistance") {
      toggle = new ParamValueControl(param, title, desc, icon, 0, 10, std::map<int, QString>(), this, false, " feet");

    } else if (param == "Model") {
      modelSelectorButton = new ButtonIconControl(tr("模型選擇"), tr("選擇"), tr("選擇您喜歡的開放駕駛型號."), "../assets/offroad/icon_calibration.png");
      const QStringList models = {"New Delhi", "Blue Diamond V1", "Farmville", "Blue Diamond V2", "New Lemon Pie"};
      QObject::connect(modelSelectorButton, &ButtonIconControl::clicked, this, [this, models]() {
        const int currentModel = params.getInt("Model");
        const QString currentModelLabel = models[currentModel];

        const QString selection = MultiOptionDialog::getSelection(tr("選擇駕駛模型"), models, currentModelLabel, this);
        if (!selection.isEmpty()) {
          const int selectedModel = models.indexOf(selection);
          params.putInt("Model", selectedModel);
          modelSelectorButton->setValue(selection);
          if (ConfirmationDialog::toggle("Reboot required to take effect.", "Reboot Now", this)) {
            Hardware::reboot();
          }
        }
      });
      modelSelectorButton->setValue(models[params.getInt("Model")]);
      addItem(modelSelectorButton);

    } else if (param == "NudgelessLaneChange") {
      ParamManageControl *laneChangeToggle = new ParamManageControl(param, title, desc, icon, this);
      QObject::connect(laneChangeToggle, &ParamManageControl::manageButtonClicked, this, [this]() {
        parentToggleClicked();
        for (auto &[key, toggle] : toggles) {
          toggle->setVisible(laneChangeKeys.find(key.c_str()) != laneChangeKeys.end());
        }
      });
      toggle = laneChangeToggle;
    } else if (param == "LaneChangeTime") {
      std::map<int, QString> laneChangeTimeLabels;
      for (int i = 0; i <= 10; ++i) {
        laneChangeTimeLabels[i] = i == 0 ? "立即" : QString::number(i / 2.0) + " 秒";
      }
      toggle = new ParamValueControl(param, title, desc, icon, 0, 10, laneChangeTimeLabels, this, false);

    } else if (param == "SpeedLimitController") {
      ParamManageControl *speedLimitControllerToggle = new ParamManageControl(param, title, desc, icon, this);
      QObject::connect(speedLimitControllerToggle, &ParamManageControl::manageButtonClicked, this, [this]() {
        parentToggleClicked();
        slscPriorityButton->setVisible(true);
        for (auto &[key, toggle] : toggles) {
          toggle->setVisible(speedLimitControllerKeys.find(key.c_str()) != speedLimitControllerKeys.end());
        }
      });
      toggle = speedLimitControllerToggle;
    } else if (param == "Offset1" || param == "Offset2" || param == "Offset3" || param == "Offset4") {
      toggle = new ParamValueControl(param, title, desc, icon, 0, 99, std::map<int, QString>(), this, false, " mph");
    } else if (param == "SLCFallback") {
      toggle = new ParamValueControl(param, title, desc, icon, 0, 2, {{0, "無"}, {1, "實驗模式"}, {2, "之前的限速"}}, this, true);
    } else if (param == "SLCPriority") {
      const QStringList priorities {
        "導航, 儀表板, 離線地圖",
        "導航, 離線地圖, 儀表板",
        "導航, 離線地圖",
        "導航, 儀表板",
        "導航",
        "離線地圖, 儀表板, 導航",
        "離線地圖, 導航, 儀表板",
        "離線地圖, 導航",
        "離線地圖, 儀表板",
        "離線地圖",
        "儀表板, 導航, 離線地圖",
        "儀表板, 離線地圖, 導航",
        "儀表板, 離線地圖",
        "儀表板, 導航",
        "儀表板",
        "最高速",
        "最低速",
        "",
      };

      slscPriorityButton = new ButtonControl(tr("  優先順序"), tr("選擇"), tr("使用“速度限制控制器”確定選擇速度限制的優先順序'."));
      QObject::connect(slscPriorityButton, &ButtonControl::clicked, this, [this, priorities]() {
        QStringList availablePriorities = {"儀表板", "導航", "離線地圖", "最高速", "最低速", "無"};
        QStringList selectedPriorities;
        int priorityValue = -1;

        const QStringList priorityPrompts = {tr("選擇首要優先"), tr("選擇次要優先"), tr("選擇第三優先")};

        for (int i = 0; i < 3; ++i) {
          const QString selection = MultiOptionDialog::getSelection(priorityPrompts[i], availablePriorities, "", this);
          if (selection.isEmpty()) break;

          if (selection == "無") {
            priorityValue = 17;
            break;
          } else if (selection == "最高速") {
            priorityValue = 15;
            break;
          } else if (selection == "最低速") {
            priorityValue = 16;
            break;
          } else {
            selectedPriorities.append(selection);
            availablePriorities.removeAll(selection);
            availablePriorities.removeAll("最高速");
            availablePriorities.removeAll("最低速");
          }
        }

        if (priorityValue == -1 && !selectedPriorities.isEmpty()) {
          QString priorityString = selectedPriorities.join(", ");
          priorityValue = priorities.indexOf(priorityString);
        }

        if (priorityValue != -1) {
          slscPriorityButton->setValue(priorities[priorityValue]);
          params.putInt("SLCPriority", priorityValue);
          std::thread([this]() {
            paramsMemory.putBool("FrogPilotTogglesUpdated", true);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            paramsMemory.putBool("FrogPilotTogglesUpdated", false);
          }).detach();
        }
      });
      slscPriorityButton->setValue(priorities[params.getInt("SLCPriority")]);
      addItem(slscPriorityButton);

    } else if (param == "VisionTurnControl") {
      ParamManageControl *visionTurnControlToggle = new ParamManageControl(param, title, desc, icon, this);
      QObject::connect(visionTurnControlToggle, &ParamManageControl::manageButtonClicked, this, [this]() {
        parentToggleClicked();
        for (auto &[key, toggle] : toggles) {
          toggle->setVisible(visionTurnControlKeys.find(key.c_str()) != visionTurnControlKeys.end());
        }
      });
      toggle = visionTurnControlToggle;
    } else if (param == "CurveSensitivity" || param == "TurnAggressiveness") {
      toggle = new ParamValueControl(param, title, desc, icon, 1, 200, std::map<int, QString>(), this, false, "%");

    } else {
      toggle = new ParamControl(param, title, desc, icon, this);
    }

    addItem(toggle);
    toggles[param.toStdString()] = toggle;

    QObject::connect(toggles["AlwaysOnLateral"], &ToggleControl::toggleFlipped, [this](bool state) {
      toggles["AlwaysOnLateralMain"]->setVisible(state);
    });

    QObject::connect(toggle, &ToggleControl::toggleFlipped, [this]() {
      std::thread([this]() {
        paramsMemory.putBool("FrogPilotTogglesUpdated", true);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        paramsMemory.putBool("FrogPilotTogglesUpdated", false);
      }).detach();
    });

    QObject::connect(static_cast<ParamValueControl*>(toggle), &ParamValueControl::buttonPressed, [this]() {
      std::thread([this]() {
        paramsMemory.putBool("FrogPilotTogglesUpdated", true);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        paramsMemory.putBool("FrogPilotTogglesUpdated", false);
      }).detach();
    });
  }

  conditionalExperimentalKeys = {"CECurves", "CECurvesLead", "CESlowerLead", "CENavigation", "CEStopLights", "CESignal"};
  customPersonalitiesKeys = {"AggressiveFollow", "AggressiveJerk", "StandardFollow", "StandardJerk", "RelaxedFollow", "RelaxedJerk"};
  fireTheBabysitterKeys = {"NoLogging", "MuteDM", "MuteDoor", "MuteOverheated", "MuteSeatbelt"};
  laneChangeKeys = {"LaneChangeTime", "LaneDetection", "OneLaneChange", "PauseLateralOnSignal"};
  lateralTuneKeys = {"AverageCurvature", "NNFF"};
  longitudinalTuneKeys = {"AccelerationProfile", "AggressiveAcceleration", "SmoothBraking", "StoppingDistance"};
  speedLimitControllerKeys = {"Offset1", "Offset2", "Offset3", "Offset4", "SLCFallback", "SLCPriority"};
  visionTurnControlKeys = {"CurveSensitivity", "TurnAggressiveness"};

  QObject::connect(toggles["AlwaysOnLateralMain"], &ToggleControl::toggleFlipped, [parent](bool state) {
    if (state) {
      ConfirmationDialog::toggleAlert("WARNING: This is very experimental and isn't guaranteed to work. If you run into any issues, please report it in the FrogPilot Discord!", "I understand the risks.", parent);
    }
  });

  std::set<std::string> rebootKeys = {"AlwaysOnLateral", "AlwaysOnLateralMain", "FireTheBabysitter", "NoLogging", "MuteDM", "NNFF"};
  for (const std::string &key : rebootKeys) {
    QObject::connect(toggles[key], &ToggleControl::toggleFlipped, [parent]() {
      if (ConfirmationDialog::toggle("Reboot required to take effect.", "Reboot Now", parent)) {
        Hardware::reboot();
      }
    });
  }

  QObject::connect(uiState(), &UIState::uiUpdate, this, &FrogPilotControlsPanel::updateState);

  hideSubToggles();
  setDefaults();
}

void FrogPilotControlsPanel::updateState() {
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
      const double distanceConversion = isMetric ? FOOT_TO_METER : METER_TO_FOOT;
      const double speedConversion = isMetric ? MILE_TO_KM : KM_TO_MILE;
      params.putInt("CESpeed", std::nearbyint(params.getInt("CESpeed") * speedConversion));
      params.putInt("CESpeedLead", std::nearbyint(params.getInt("CESpeedLead") * speedConversion));
      params.putInt("Offset1", std::nearbyint(params.getInt("Offset1") * speedConversion));
      params.putInt("Offset2", std::nearbyint(params.getInt("Offset2") * speedConversion));
      params.putInt("Offset3", std::nearbyint(params.getInt("Offset3") * speedConversion));
      params.putInt("Offset4", std::nearbyint(params.getInt("Offset4") * speedConversion));
      params.putInt("StoppingDistance", std::nearbyint(params.getInt("StoppingDistance") * distanceConversion));
    }

    ParamValueControl *offset1Toggle = static_cast<ParamValueControl*>(toggles["Offset1"]);
    ParamValueControl *offset2Toggle = static_cast<ParamValueControl*>(toggles["Offset2"]);
    ParamValueControl *offset3Toggle = static_cast<ParamValueControl*>(toggles["Offset3"]);
    ParamValueControl *offset4Toggle = static_cast<ParamValueControl*>(toggles["Offset4"]);
    ParamValueControl *stoppingDistanceToggle = static_cast<ParamValueControl*>(toggles["StoppingDistance"]);

    if (isMetric) {
      offset1Toggle->setTitle("  速度 0-34 kph 的微調");
      offset2Toggle->setTitle("  速度 35-54 kph 的微調");
      offset3Toggle->setTitle("  速度 55-64 kph 的微調");
      offset4Toggle->setTitle("  速度 65-99 kph 的微調");

      offset1Toggle->setDescription("設定 0-34 kph 速限微調.");
      offset2Toggle->setDescription("設定 35-54 kph 速限微調.");
      offset3Toggle->setDescription("設定 55-64 kph 速限微調.");
      offset4Toggle->setDescription("設定 65-99 kph 速限微調.");

      offset1Toggle->updateControl(0, 99, " kph");
      offset2Toggle->updateControl(0, 99, " kph");
      offset3Toggle->updateControl(0, 99, " kph");
      offset4Toggle->updateControl(0, 99, " kph");
      stoppingDistanceToggle->updateControl(0, 5, " 公尺");
    } else {
      offset1Toggle->setTitle("Speed Limit Offset (0-34 mph)");
      offset2Toggle->setTitle("Speed Limit Offset (35-54 mph)");
      offset3Toggle->setTitle("Speed Limit Offset (55-64 mph)");
      offset4Toggle->setTitle("Speed Limit Offset (65-99 mph)");

      offset1Toggle->setDescription("Set speed limit offset for limits between 0-34 mph.");
      offset2Toggle->setDescription("Set speed limit offset for limits between 35-54 mph.");
      offset3Toggle->setDescription("Set speed limit offset for limits between 55-64 mph.");
      offset4Toggle->setDescription("Set speed limit offset for limits between 65-99 mph.");

      offset1Toggle->updateControl(0, 99, " mph");
      offset2Toggle->updateControl(0, 99, " mph");
      offset3Toggle->updateControl(0, 99, " mph");
      offset4Toggle->updateControl(0, 99, " mph");
      stoppingDistanceToggle->updateControl(0, 10, " feet");
    }

    offset1Toggle->refresh();
    offset2Toggle->refresh();
    offset3Toggle->refresh();
    offset4Toggle->refresh();
    stoppingDistanceToggle->refresh();

    previousIsMetric = isMetric;
  }).detach();
}

void FrogPilotControlsPanel::parentToggleClicked() {
  paramsMemory.putInt("FrogPilotTogglesOpen", 1);
  conditionalSpeedsImperial->setVisible(false);
  conditionalSpeedsMetric->setVisible(false);
  modelSelectorButton->setVisible(false);
  slscPriorityButton->setVisible(false);
}

void FrogPilotControlsPanel::hideSubToggles() {
  paramsMemory.putInt("FrogPilotTogglesOpen", 0);

  conditionalSpeedsImperial->setVisible(false);
  conditionalSpeedsMetric->setVisible(false);
  modelSelectorButton->setVisible(true);
  slscPriorityButton->setVisible(false);

  for (auto &[key, toggle] : toggles) {
    const bool subToggles = conditionalExperimentalKeys.find(key.c_str()) != conditionalExperimentalKeys.end() ||
                            customPersonalitiesKeys.find(key.c_str()) != customPersonalitiesKeys.end() ||
                            fireTheBabysitterKeys.find(key.c_str()) != fireTheBabysitterKeys.end() ||
                            laneChangeKeys.find(key.c_str()) != laneChangeKeys.end() ||
                            lateralTuneKeys.find(key.c_str()) != lateralTuneKeys.end() ||
                            longitudinalTuneKeys.find(key.c_str()) != longitudinalTuneKeys.end() ||
                            speedLimitControllerKeys.find(key.c_str()) != speedLimitControllerKeys.end() ||
                            visionTurnControlKeys.find(key.c_str()) != visionTurnControlKeys.end();
    toggle->setVisible(!subToggles);
    if (key == "AlwaysOnLateralMain") {
      toggle->setVisible(params.getBool("AlwaysOnLateral"));
    }
  }
}

void FrogPilotControlsPanel::hideEvent(QHideEvent *event) {
  hideSubToggles();
}

void FrogPilotControlsPanel::setDefaults() {
  const bool FrogsGoMoo = params.get("DongleId").substr(0, 3) == "be6";

  const std::map<std::string, std::string> defaultValues {
    {"AccelerationProfile", "1"},
    {"AdjustablePersonalities", "3"},
    {"AggressiveAcceleration", "1"},
    {"AggressiveFollow", FrogsGoMoo ? "10" : "12"},
    {"AggressiveJerk", FrogsGoMoo ? "6" : "5"},
    {"AlwaysOnLateral", "1"},
    {"AlwaysOnLateralMain", FrogsGoMoo ? "1" : "1"},
    {"AverageCurvature", FrogsGoMoo ? "1" : "1"},
    {"CECurves", "1"},
    {"CECurvesLead", "1"},
    {"CENavigation", "1"},
    {"CESignal", "1"},
    {"CESlowerLead", "1"},
    {"CESpeed", "0"},
    {"CESpeedLead", "0"},
    {"CEStopLights", "1"},
    {"CEStopLightsLead", FrogsGoMoo ? "0" : "1"},
    {"ConditionalExperimental", "1"},
    {"CurveSensitivity", FrogsGoMoo ? "125" : "180"},
    {"CustomPersonalities", "1"},
    {"DeviceShutdown", "9"},
    {"ExperimentalModeViaPress", "1"},
    {"FireTheBabysitter", FrogsGoMoo ? "1" : "1"},
    {"LaneChangeTime", "1"},
    {"LaneDetection", "1"},
    {"LateralTune", "1"},
    {"LongitudinalTune", "1"},
    {"MTSCEnabled", "1"},
    {"MuteDM", FrogsGoMoo ? "1" : "1"},
    {"MuteDoor", FrogsGoMoo ? "1" : "1"},
    {"MuteOverheated", FrogsGoMoo ? "1" : "1"},
    {"MuteSeatbelt", FrogsGoMoo ? "1" : "1"},
    {"NNFF", FrogsGoMoo ? "1" : "1"},
    {"NudgelessLaneChange", "1"},
    {"Offset1", "0"},
    {"Offset2", FrogsGoMoo ? "7" : "0"},
    {"Offset3", "0"},
    {"Offset4", FrogsGoMoo ? "20" : "0"},
    {"OneLaneChange", "1"},
    {"PauseLateralOnSignal", "0"},
    {"RelaxedFollow", "30"},
    {"RelaxedJerk", "50"},
    {"SLCFallback", "0"},
    {"SLCPriority", "1"},
    {"SmoothBraking", "1"},
    {"SpeedLimitController", "1"},
    {"StandardFollow", "15"},
    {"StandardJerk", "10"},
    {"StoppingDistance", FrogsGoMoo ? "6" : "1"},
    {"TurnAggressiveness", FrogsGoMoo ? "150" : "150"},
    {"TurnDesires", "1"},
    {"VisionTurnControl", "1"},
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

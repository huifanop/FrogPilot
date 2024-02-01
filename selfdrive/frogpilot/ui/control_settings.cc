#include "selfdrive/frogpilot/ui/control_settings.h"
#include "selfdrive/ui/ui.h"

FrogPilotControlsPanel::FrogPilotControlsPanel(SettingsWindow *parent) : FrogPilotListWidget(parent) {
  const std::vector<std::tuple<QString, QString, QString, QString>> controlToggles {
    {"AdjustablePersonalities", "駕駛模式", "透過畫面切換駕駛模式.\n\n1 格 = 積極\n2 格 = 標準\n3 格 = 輕鬆", "../frogpilot/assets/toggle_icons/icon_distance.png"},
    {"AlwaysOnLateral", "全時置中模式", "使用剎車或油門踏板時仍保持橫向控制.只有停用“定速”後才能解除.", "../frogpilot/assets/toggle_icons/icon_always_on_lateral.png"},

    {"ConditionalExperimental", "條件式的實驗模式", "根據特定條件自動啟動實驗模式.", "../frogpilot/assets/toggle_icons/icon_conditional.png"},
    {"CECurves", "  彎道", "偵測到曲線時切換到“實驗模式”.", ""},
    {"CENavigation", "  導航", "根據導航十字路口、停車標誌等切換到“實驗模式”.", ""},
    {"CESlowerLead", "  低速前車", "當偵測到前方較慢車輛時切換到“實驗模式”.", ""},
    {"CEStopLights", "  停止標誌", "當偵測到停車燈或停車標誌時切換到“實驗模式”.", ""},
    {"CESignal", "  方向燈", "當使用低於 55 英里/小時的轉向信號燈時切換到“實驗模式”以幫助轉彎.", ""},

    {"CustomPersonalities", "設定駕駛模式", "根據您的喜好設定駕駛模式細項設定.", "../frogpilot/assets/toggle_icons/icon_custom.png"},
    {"DeviceShutdown", "設備自動關機設定", "設置設備在熄火後自動關閉的時間，以減少能源浪費並防止電池耗盡.", "../frogpilot/assets/toggle_icons/icon_time.png"},
    {"ExperimentalModeActivation", "利用畫面或方向盤開啟實驗模式", "通過雙擊方向盤上的“車道偏離”/LKAS 按鈕(Toyota/Lexus Only)以啟用或禁用實驗模式，或雙擊營幕覆蓋“條件實驗模式”'.", "../assets/img_experimental_white.svg"},

    {"FireTheBabysitter", "關閉監控", "禁用 openpilot 的一些‘保姆協議’.", "../frogpilot/assets/toggle_icons/icon_babysitter.png"},
    {"NoLogging", "  停用所有日誌記錄", "關閉所有數據追蹤以增強隱私或減少熱負荷.\n\n 警告：此操作將阻止驅動器記錄且資料無法恢復!", ""},
    {"MuteDoor", "  車門", "禁用開門警報.", ""},
    {"MuteDM", "  駕駛監控", "禁用駕駛員監控.", ""},
    {"MuteOverheated", "  系統過熱", "禁用設備過熱警報.", ""},
    {"MuteSeatbelt", "  安全帶", "禁用未扣安全帶的警報.", ""},
    {"OfflineMode", "  離線模式", "允許設備無限期離線.", ""},

    {"LateralTune", "橫向調整", "改變 openpilot 的駕駛方式.", "../frogpilot/assets/toggle_icons/icon_lateral_tune.png"},
    {"AverageCurvature", "  平均期望曲率", "使用 Pfeiferj 的以距離為基準的曲率調整方法來更平滑地處理轉彎.", ""},
    {"NNFF", "  NNFF - 神經網路前饋", "使用Twilsonco's的神經網路前饋扭矩控制系統來獲得更精準的橫向控制.", ""},
    {"SteerRatio", steerRatioStock != 0 ? QString("Steer Ratio (Default: %1)").arg(steerRatioStock, 0, 'f', 2) : "  轉向比", "為您的車輛控制設定自訂轉向比.", ""},

    {"LongitudinalTune", "縱向調整", "改變 openpilot 加速和煞車方式.", "../frogpilot/assets/toggle_icons/icon_longitudinal_tune.png"},
    {"AccelerationProfile", "  行車模式", "將加速度改為運動型或環保型.", ""},
    {"AggressiveAcceleration", "  積極加速跟車", "當有前車可跟隨時起步更加積極的加速.", ""},
    {"StoppingDistance", "  增加停車距離", "增加停車距離，讓停車更舒適.", ""},
    {"SmoothBraking", "  平穩煞車的跟車", "當接近速度較慢的車輛時，煞車行為更加自然.", ""},

    {"Model", "模型選擇", "選擇您喜歡的模型.", "../assets/offroad/icon_calibration.png"},

    {"MTSCEnabled", "地圖彎道速度控制", "根據下載地圖偵測到的預期曲線放慢速度.", "../frogpilot/assets/toggle_icons/icon_speed_map.png"},

    {"NudgelessLaneChange", "自動變換車道", "不需輕推方向盤即可變換車道.", "../frogpilot/assets/toggle_icons/icon_lane.png"},
    {"LaneChangeTime", "  自動變換車道延遲", "設定自動變換車道延遲時間.", ""},
    {"LaneDetection", "  車道檢測", "未偵測到車道時阻止自動變換車道.", ""},
    {"OneLaneChange", "  每次只變換一個車道", "每次啟動方向燈時，僅執行一次自動變換車道.", ""},

    {"QOLControls", "優化體驗", "各種控制細項的調整可改善您的openpilot體驗.", "../frogpilot/assets/toggle_icons/quality_of_life.png"},
    {"DisableOnroadUploads", "禁止車輛行進中上傳資料", "防止在車輛行進中上傳大量數據.", ""},
    {"HigherBitrate", "高畫質率錄製", "提高上傳到 comma connect 的畫質.", ""},
    {"PauseLateralOnSignal", "使用方向燈時暫停橫向控制", "在低於此設定速度下使用方向燈時暫時停用橫向控制.", ""},
    {"ReverseCruise", "反向巡航增加", "增加最大設定速度時反轉「長按」功能。有助於快速提高最大速度.", ""},
    {"SetSpeedOffset", "設定速度偏移", "設定您想要的偏移量 set speed.", ""},

    {"SpeedLimitController", "限速控制器", "使用「開放街道地圖」、「在 openpilot 上導航」或汽車儀表板（僅限 TSS2 豐田）自動調整車速以匹配速度限制.", "../assets/offroad/icon_speed_limit.png"},
    {"Offset1", "速限微調 (0-34 mph)", "  速度介於 0-34 mph 的速限微調.", ""},
    {"Offset2", "速限微調 (35-54 mph)", "  速度介於 35-54 mph 的速限微調.", ""},
    {"Offset3", "速限微調 (55-64 mph)", "  速度介於 55-64 mph 的速限微調.", ""},
    {"Offset4", "速限微調 (65-99 mph)", "  速度介於 65-99 mph 的速限微調.", ""},
    {"SLCFallback", "  備援方式", "當導航、OSM 或汽車儀表板中沒有速度限制時，設定您的替代方法.", ""},
    {"SLCOverride", "  覆蓋方法", "選擇您喜歡的方法來覆蓋當前的速度限制.", ""},
    {"SLCPriority", "  優先選項", "選擇優先使用的速度限制模式.", ""},

    {"TurnDesires", "意圖轉彎", "打開此選項在低於最低自動換道時速40KMH以下時打方向燈時獲得更精準的轉彎.", "../assets/navigation/direction_continue_right.png"},

    {"VisionTurnControl", "視覺轉向速度控制", "根據路面曲率自動調整車速，轉彎更順暢.", "../frogpilot/assets/toggle_icons/icon_vtc.png"},
    {"CurveSensitivity", "  曲線檢測靈敏度", "設定曲線檢測靈敏度.較高的值會導致較早的反應，較低的值會導致較平滑但較晚的反應.", ""},
    {"TurnAggressiveness", "  轉彎速度積極性", "設定轉彎速度攻擊性.較高的數值會導致較快的轉彎，較低的數值會導致較平緩的轉彎.", ""},
  };

  for (const auto &[param, title, desc, icon] : controlToggles) {
    ParamControl *toggle;

    if (param == "AdjustablePersonalities") {
      toggle = new FrogPilotParamValueControl(param, title, desc, icon, 0, 3, {{0, "無"}, {1, "方向盤按鍵"}, {2, "螢幕按鍵"}, {3, "方向盤 + 螢幕"}}, this, true);

    } else if (param == "AlwaysOnLateral") {
      std::vector<QString> aolToggles{"AlwaysOnLateralMain"};
      std::vector<QString> aolToggleNames{tr("Enable On Cruise Main")};
      toggle = new FrogPilotParamToggleControl(param, title, desc, icon, aolToggles, aolToggleNames);

      QObject::connect(static_cast<FrogPilotParamToggleControl*>(toggle), &FrogPilotParamToggleControl::buttonClicked, [this](const bool checked) {
        if (checked) {
          FrogPilotConfirmationDialog::toggleAlert("WARNING: This is very experimental and isn't guaranteed to work. If you run into any issues, please report it in the FrogPilot Discord!",
          "I understand the risks.", this);
        }
        if (FrogPilotConfirmationDialog::toggle("需要重新啟動才能生效.", "立刻重啟", this)) {
          Hardware::reboot();
        }
      });

    } else if (param == "ConditionalExperimental") {
      FrogPilotParamManageControl *conditionalExperimentalToggle = new FrogPilotParamManageControl(param, title, desc, icon, this);
      QObject::connect(conditionalExperimentalToggle, &FrogPilotParamManageControl::manageButtonClicked, this, [this]() {
        parentToggleClicked();
        conditionalSpeedsImperial->setVisible(!isMetric);
        conditionalSpeedsMetric->setVisible(isMetric);
        for (auto &[key, toggle] : toggles) {
          toggle->setVisible(conditionalExperimentalKeys.find(key.c_str()) != conditionalExperimentalKeys.end());
        }
      });
      toggle = conditionalExperimentalToggle;
    } else if (param == "CECurves") {
      FrogPilotParamValueControl *CESpeedImperial = new FrogPilotParamValueControl("CESpeed", "無車", "沒有前方車輛時低於此速度切換實驗模式.", "", 0, 99, std::map<int, QString>(), this, false, " mph");
      FrogPilotParamValueControl *CESpeedLeadImperial = new FrogPilotParamValueControl("CESpeedLead", " 有車", "有前方車輛時低於此速度切換到實驗模式.", "", 0, 99, std::map<int, QString>(), this, false, " mph");
      conditionalSpeedsImperial = new FrogPilotDualParamControl(CESpeedImperial, CESpeedLeadImperial, this);
      addItem(conditionalSpeedsImperial);

      FrogPilotParamValueControl *CESpeedMetric = new FrogPilotParamValueControl("CESpeed", "  無車", "沒有車輛時低於此速度切換“實驗模式”.", "", 0, 99, std::map<int, QString>(), this, false, " kph");
      FrogPilotParamValueControl *CESpeedLeadMetric = new FrogPilotParamValueControl("CESpeedLead", "  有車", "有車輛時低於此速度切換到“實驗模式”.", "", 0, 150, std::map<int, QString>(), this, false, " kph");
      conditionalSpeedsMetric = new FrogPilotDualParamControl(CESpeedMetric, CESpeedLeadMetric, this);
      addItem(conditionalSpeedsMetric);

      std::vector<QString> curveToggles{"CECurvesLead"};
      std::vector<QString> curveToggleNames{tr("有前車")};
      toggle = new FrogPilotParamToggleControl(param, title, desc, icon, curveToggles, curveToggleNames);
    } else if (param == "CEStopLights") {
      std::vector<QString> stopLightToggles{"CEStopLightsLead"};
      std::vector<QString> stopLightToggleNames{tr("有前車")};
      toggle = new FrogPilotParamToggleControl(param, title, desc, icon, stopLightToggles, stopLightToggleNames);

    } else if (param == "CustomPersonalities") {
      FrogPilotParamManageControl *customPersonalitiesToggle = new FrogPilotParamManageControl(param, title, desc, icon, this);
      QObject::connect(customPersonalitiesToggle, &FrogPilotParamManageControl::manageButtonClicked, this, [this]() {
        parentToggleClicked();
        for (auto &[key, toggle] : toggles) {
          toggle->setVisible(false);
        }
        aggressiveProfile->setVisible(true);
        standardProfile->setVisible(true);
        relaxedProfile->setVisible(true);
      });
      toggle = customPersonalitiesToggle;

      FrogPilotParamValueControl *aggressiveFollow = new FrogPilotParamValueControl("AggressiveFollow", "跟隨",
      "設定積極模式中的跟隨距離.表示跟隨前方車輛的秒數.\n\n預設:1.25 秒.", "../frogpilot/assets/other_images/aggressive.png",
      10, 50, std::map<int, QString>(), this, false, " sec", 10);
      FrogPilotParamValueControl *aggressiveJerk = new FrogPilotParamValueControl("AggressiveJerk", " Jerk",
      "設定積極模式中的煞車/油門踏板反應能力.數值越高，反應越“輕鬆”.\n\n預設: 0.5.", "", 1, 50,
                                                                std::map<int, QString>(), this, false, "", 10);
      aggressiveProfile = new FrogPilotDualParamControl(aggressiveFollow, aggressiveJerk, this, true);
      addItem(aggressiveProfile);

      FrogPilotParamValueControl *standardFollow = new FrogPilotParamValueControl("StandardFollow", "跟隨",
      "設定標準模式中的跟隨距離.表示跟隨前方車輛的秒數..\n\n預設: 1.45 秒.", "../frogpilot/assets/other_images/standard.png",
      10, 50, std::map<int, QString>(), this, false, " sec", 10);
      FrogPilotParamValueControl *standardJerk = new FrogPilotParamValueControl("StandardJerk", " Jerk",
      "設定標準模式中的煞車/油門踏板反應能力.數值越高，反應越“輕鬆”.\n\n預設: 1.0.", "", 1, 50,
                                                              std::map<int, QString>(), this, false, "", 10);
      standardProfile = new FrogPilotDualParamControl(standardFollow, standardJerk, this, true);
      addItem(standardProfile);

      FrogPilotParamValueControl *relaxedFollow = new FrogPilotParamValueControl("RelaxedFollow", "跟隨",
      "設定輕鬆模式中的跟隨距離.表示跟隨前方車輛的秒數.\n\n預設: 1.75 秒.", "../frogpilot/assets/other_images/relaxed.png",
      10, 50, std::map<int, QString>(), this, false, " sec", 10);
      FrogPilotParamValueControl *relaxedJerk = new FrogPilotParamValueControl("RelaxedJerk", " Jerk",
      "設定輕鬆模式中的煞車/油門踏板反應能力.數值越高，反應越“輕鬆”.\n\n預設: 1.0.", "", 1, 50,
                                                             std::map<int, QString>(), this, false, "", 10);
      relaxedProfile = new FrogPilotDualParamControl(relaxedFollow, relaxedJerk, this, true);
      addItem(relaxedProfile);

    } else if (param == "DeviceShutdown") {
      std::map<int, QString> shutdownLabels;
      for (int i = 0; i <= 33; ++i) {
        shutdownLabels[i] = i == 0 ? "立即" : i <= 3 ? QString::number(i * 15) + " 分鐘" : QString::number(i - 3) + (i == 4 ? " 小時" : " 小時");
      }
      toggle = new FrogPilotParamValueControl(param, title, desc, icon, 0, 33, shutdownLabels, this, false);

    } else if (param == "ExperimentalModeActivation") {
      std::vector<QString> experimentalModeToggles{"ExperimentalModeViaLKAS", "ExperimentalModeViaScreen"};
      std::vector<QString> experimentalModeNames{tr("按鍵"), tr("螢幕")};
      toggle = new FrogPilotParamToggleControl(param, title, desc, icon, experimentalModeToggles, experimentalModeNames);

    } else if (param == "FireTheBabysitter") {
      FrogPilotParamManageControl *fireTheBabysitterToggle = new FrogPilotParamManageControl(param, title, desc, icon, this);
      QObject::connect(fireTheBabysitterToggle, &FrogPilotParamManageControl::manageButtonClicked, this, [this]() {
        parentToggleClicked();
        for (auto &[key, toggle] : toggles) {
          toggle->setVisible(fireTheBabysitterKeys.find(key.c_str()) != fireTheBabysitterKeys.end());
        }
      });
      toggle = fireTheBabysitterToggle;

    } else if (param == "LateralTune") {
      FrogPilotParamManageControl *lateralTuneToggle = new FrogPilotParamManageControl(param, title, desc, icon, this);
      QObject::connect(lateralTuneToggle, &FrogPilotParamManageControl::manageButtonClicked, this, [this]() {
        parentToggleClicked();
        for (auto &[key, toggle] : toggles) {
          toggle->setVisible(lateralTuneKeys.find(key.c_str()) != lateralTuneKeys.end());
        }
      });
      toggle = lateralTuneToggle;
    } else if (param == "SteerRatio") {
      toggle = new FrogPilotParamValueControlFloat(param, title, desc, icon, steerRatioStock * 0.75, steerRatioStock * 1.25, std::map<int, QString>(), this, false, "", 10.0);

    } else if (param == "LongitudinalTune") {
      FrogPilotParamManageControl *longitudinalTuneToggle = new FrogPilotParamManageControl(param, title, desc, icon, this);
      QObject::connect(longitudinalTuneToggle, &FrogPilotParamManageControl::manageButtonClicked, this, [this]() {
        parentToggleClicked();
        for (auto &[key, toggle] : toggles) {
          toggle->setVisible(longitudinalTuneKeys.find(key.c_str()) != longitudinalTuneKeys.end());
        }
      });
      toggle = longitudinalTuneToggle;
    } else if (param == "AccelerationProfile") {
      std::vector<QString> profileOptions{tr("標準"), tr("節能"), tr("運動"), tr("超跑")};
      FrogPilotButtonParamControl *profileSelection = new FrogPilotButtonParamControl(param, title, desc, icon, profileOptions);
      toggle = profileSelection;

      QObject::connect(static_cast<FrogPilotButtonParamControl*>(toggle), &FrogPilotButtonParamControl::buttonClicked, [this](int id) {
        if (id == 3) {
          FrogPilotConfirmationDialog::toggleAlert("WARNING: This maxes out openpilot's acceleration from 2.0 m/s to 4.0 m/s and may cause oscillations when accelerating!",
          "I understand the risks.", this);
        }
      });
    } else if (param == "StoppingDistance") {
      toggle = new FrogPilotParamValueControl(param, title, desc, icon, 0, 10, std::map<int, QString>(), this, false, " feet");

    } else if (param == "Model") {
      modelSelectorButton = new FrogPilotButtonIconControl(title, tr("選擇"), desc, icon);
      QStringList models = {"Los Angeles"};
      QObject::connect(modelSelectorButton, &FrogPilotButtonIconControl::clicked, this, [this, models]() {
        int currentModel = params.getInt("Model");
        QString currentModelLabel = models[currentModel];

        QString selection = MultiOptionDialog::getSelection(tr("選擇駕駛模型"), models, currentModelLabel, this);
        if (!selection.isEmpty()) {
          int selectedModel = models.indexOf(selection);
          params.putInt("Model", selectedModel);
          params.remove("CalibrationParams");
          params.remove("LiveTorqueParameters");
          modelSelectorButton->setValue(selection);
          if (FrogPilotConfirmationDialog::toggle("需要重新啟動才能生效.", "立刻重啟", this)) {
            Hardware::reboot();
          }
        }
      });
      modelSelectorButton->setValue(models[params.getInt("Model")]);
      addItem(modelSelectorButton);

    } else if (param == "NudgelessLaneChange") {
      FrogPilotParamManageControl *laneChangeToggle = new FrogPilotParamManageControl(param, title, desc, icon, this);
      QObject::connect(laneChangeToggle, &FrogPilotParamManageControl::manageButtonClicked, this, [this]() {
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
      toggle = new FrogPilotParamValueControl(param, title, desc, icon, 0, 10, laneChangeTimeLabels, this, false);

    } else if (param == "QOLControls") {
      FrogPilotParamManageControl *qolToggle = new FrogPilotParamManageControl(param, title, desc, icon, this);
      QObject::connect(qolToggle, &FrogPilotParamManageControl::manageButtonClicked, this, [this]() {
        parentToggleClicked();
        for (auto &[key, toggle] : toggles) {
          toggle->setVisible(qolKeys.find(key.c_str()) != qolKeys.end());
        }
      });
      toggle = qolToggle;
    } else if (param == "PauseLateralOnSignal") {
      toggle = new FrogPilotParamValueControl(param, title, desc, icon, 0, 99, std::map<int, QString>(), this, false, " mph");
    } else if (param == "SetSpeedOffset") {
      toggle = new FrogPilotParamValueControl(param, title, desc, icon, 0, 99, std::map<int, QString>(), this, false, " mph");

    } else if (param == "SpeedLimitController") {
      FrogPilotParamManageControl *speedLimitControllerToggle = new FrogPilotParamManageControl(param, title, desc, icon, this);
      QObject::connect(speedLimitControllerToggle, &FrogPilotParamManageControl::manageButtonClicked, this, [this]() {
        parentToggleClicked();
        for (auto &[key, toggle] : toggles) {
          toggle->setVisible(speedLimitControllerKeys.find(key.c_str()) != speedLimitControllerKeys.end());
        }
        slscPriorityButton->setVisible(true);
      });
      toggle = speedLimitControllerToggle;
    } else if (param == "Offset1" || param == "Offset2" || param == "Offset3" || param == "Offset4") {
      toggle = new FrogPilotParamValueControl(param, title, desc, icon, 0, 99, std::map<int, QString>(), this, false, " mph");
    } else if (param == "SLCFallback") {
      std::vector<QString> fallbackOptions{tr("無"), tr("實驗模式"), tr("之前的限速")};
      FrogPilotButtonParamControl *fallbackSelection = new FrogPilotButtonParamControl(param, title, desc, icon, fallbackOptions);
      toggle = fallbackSelection;
    } else if (param == "SLCOverride") {
      std::vector<QString> overrideOptions{tr("None"), tr("Manual Set Speed"), tr("Max Set Speed")};
      FrogPilotButtonParamControl *overrideSelection = new FrogPilotButtonParamControl(param, title, desc, icon, overrideOptions);
      toggle = overrideSelection;
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

      slscPriorityButton = new ButtonControl(title, tr("SELECT"), desc);
      QObject::connect(slscPriorityButton, &ButtonControl::clicked, this, [this, priorities]() {
        QStringList availablePriorities = {"儀表板", "導航", "離線地圖", "最高速", "最低速", "無"};
        QStringList selectedPriorities;
        int priorityValue = -1;

        QStringList priorityPrompts = {tr("選擇主要優先"), tr("選擇次要順序"), tr("選擇第三順序")};

        for (int i = 0; i < 3; ++i) {
          QString selection = MultiOptionDialog::getSelection(priorityPrompts[i], availablePriorities, "", this);
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
          updateToggles();
        }
      });
      slscPriorityButton->setValue(priorities[params.getInt("SLCPriority")]);
      addItem(slscPriorityButton);

    } else if (param == "VisionTurnControl") {
      FrogPilotParamManageControl *visionTurnControlToggle = new FrogPilotParamManageControl(param, title, desc, icon, this);
      QObject::connect(visionTurnControlToggle, &FrogPilotParamManageControl::manageButtonClicked, this, [this]() {
        parentToggleClicked();
        for (auto &[key, toggle] : toggles) {
          toggle->setVisible(visionTurnControlKeys.find(key.c_str()) != visionTurnControlKeys.end());
        }
      });
      toggle = visionTurnControlToggle;
    } else if (param == "CurveSensitivity" || param == "TurnAggressiveness") {
      toggle = new FrogPilotParamValueControl(param, title, desc, icon, 1, 200, std::map<int, QString>(), this, false, "%");

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

    QObject::connect(static_cast<FrogPilotParamManageControl*>(toggle), &FrogPilotParamManageControl::manageButtonClicked, this, [this]() {
      update();
    });
  }

  std::set<std::string> rebootKeys = {"AlwaysOnLateral", "FireTheBabysitter", "HigherBitrate", "MuteDM", "NNFF"};
  for (const std::string &key : rebootKeys) {
    QObject::connect(toggles[key], &ToggleControl::toggleFlipped, [this]() {
      if (FrogPilotConfirmationDialog::toggle("需要重新啟動才能生效.", "立刻重啟", this)) {
        Hardware::reboot();
      }
    });
  }

  conditionalExperimentalKeys = {"CECurves", "CECurvesLead", "CESlowerLead", "CENavigation", "CEStopLights", "CESignal"};
  fireTheBabysitterKeys = {"NoLogging", "MuteDM", "MuteDoor", "MuteOverheated", "MuteSeatbelt", "OfflineMode"};
  laneChangeKeys = {"LaneChangeTime", "LaneDetection", "OneLaneChange"};
  lateralTuneKeys = {"AverageCurvature", "NNFF", "SteerRatio"};
  longitudinalTuneKeys = {"AccelerationProfile", "AggressiveAcceleration", "SmoothBraking", "StoppingDistance"};
  mtscKeys = {"MTSCAggressiveness"};
  qolKeys = {"DisableOnroadUploads", "HigherBitrate", "PauseLateralOnSignal", "ReverseCruise", "SetSpeedOffset"};
  speedLimitControllerKeys = {"Offset1", "Offset2", "Offset3", "Offset4", "SLCFallback", "SLCOverride", "SLCPriority"};
  visionTurnControlKeys = {"CurveSensitivity", "TurnAggressiveness"};

  QObject::connect(uiState(), &UIState::offroadTransition, this, [this](bool offroad) {
    if (!offroad) {
      updateCarToggles();
    }
  });

  QObject::connect(device(), &Device::interactiveTimeout, this, &FrogPilotControlsPanel::hideSubToggles);
  QObject::connect(parent, &SettingsWindow::closeParentToggle, this, &FrogPilotControlsPanel::hideSubToggles);
  QObject::connect(parent, &SettingsWindow::updateMetric, this, &FrogPilotControlsPanel::updateMetric);

  hideSubToggles();
  updateMetric();
}

void FrogPilotControlsPanel::updateCarToggles() {
  FrogPilotParamValueControlFloat *steerRatioToggle = static_cast<FrogPilotParamValueControlFloat*>(toggles["SteerRatio"]);
  steerRatioStock = params.getFloat("SteerRatioStock");
  steerRatioToggle->setTitle(steerRatioStock != 0 ? QString("Steer Ratio (Default: %1)").arg(steerRatioStock, 0, 'f', 2) : QString("Steer Ratio"));
  steerRatioToggle->updateControl(steerRatioStock * 0.75, steerRatioStock * 1.25, "", 10.0);
  steerRatioToggle->refresh();
}

void FrogPilotControlsPanel::updateToggles() {
  std::thread([this]() {
    paramsMemory.putBool("FrogPilotTogglesUpdated", true);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    paramsMemory.putBool("FrogPilotTogglesUpdated", false);
  }).detach();
}

void FrogPilotControlsPanel::updateMetric() {
  bool previousIsMetric = isMetric;
  isMetric = params.getBool("IsMetric");

  if (isMetric != previousIsMetric) {
    double distanceConversion = isMetric ? FOOT_TO_METER : METER_TO_FOOT;
    double speedConversion = isMetric ? MILE_TO_KM : KM_TO_MILE;
    params.putInt("CESpeed", std::nearbyint(params.getInt("CESpeed") * speedConversion));
    params.putInt("CESpeedLead", std::nearbyint(params.getInt("CESpeedLead") * speedConversion));
    params.putInt("Offset1", std::nearbyint(params.getInt("Offset1") * speedConversion));
    params.putInt("Offset2", std::nearbyint(params.getInt("Offset2") * speedConversion));
    params.putInt("Offset3", std::nearbyint(params.getInt("Offset3") * speedConversion));
    params.putInt("Offset4", std::nearbyint(params.getInt("Offset4") * speedConversion));
    params.putInt("PauseLateralOnSignal", std::nearbyint(params.getInt("PauseLateralOnSignal") * speedConversion));
    params.putInt("SetSpeedOffset", std::nearbyint(params.getInt("SetSpeedOffset") * speedConversion));
    params.putInt("StoppingDistance", std::nearbyint(params.getInt("StoppingDistance") * distanceConversion));
  }

  FrogPilotParamValueControl *offset1Toggle = static_cast<FrogPilotParamValueControl*>(toggles["Offset1"]);
  FrogPilotParamValueControl *offset2Toggle = static_cast<FrogPilotParamValueControl*>(toggles["Offset2"]);
  FrogPilotParamValueControl *offset3Toggle = static_cast<FrogPilotParamValueControl*>(toggles["Offset3"]);
  FrogPilotParamValueControl *offset4Toggle = static_cast<FrogPilotParamValueControl*>(toggles["Offset4"]);
  FrogPilotParamValueControl *pauseLateralToggle = static_cast<FrogPilotParamValueControl*>(toggles["PauseLateralOnSignal"]);
  FrogPilotParamValueControl *setSpeedOffsetToggle = static_cast<FrogPilotParamValueControl*>(toggles["SetSpeedOffset"]);
  FrogPilotParamValueControl *stoppingDistanceToggle = static_cast<FrogPilotParamValueControl*>(toggles["StoppingDistance"]);

  if (isMetric) {
      offset1Toggle->setTitle("  速度 0-34 kph 的微調");
      offset2Toggle->setTitle("  速度 35-54 kph 的微調");
      offset3Toggle->setTitle("  速度 55-64 kph 的微調");
      offset4Toggle->setTitle("  速度 65-99 kph 的微調");

      offset1Toggle->setDescription("設定 0-34 kph 速限微調.");
      offset2Toggle->setDescription("設定 35-54 kph 速限微調.");
      offset3Toggle->setDescription("設定 55-64 kph 速限微調.");
      offset4Toggle->setDescription("設定 65-99 kph 速限微調.");

    offset1Toggle->updateControl(0, 150, " kph");
    offset2Toggle->updateControl(0, 150, " kph");
    offset3Toggle->updateControl(0, 150, " kph");
    offset4Toggle->updateControl(0, 150, " kph");
    pauseLateralToggle->updateControl(0, 150, " kph");
    setSpeedOffsetToggle->updateControl(0, 150, " kph");
    stoppingDistanceToggle->updateControl(0, 5, " meters");
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
    pauseLateralToggle->updateControl(0, 99, " mph");
    setSpeedOffsetToggle->updateControl(0, 99, " mph");
    stoppingDistanceToggle->updateControl(0, 10, " feet");
  }

  offset1Toggle->refresh();
  offset2Toggle->refresh();
  offset3Toggle->refresh();
  offset4Toggle->refresh();
  pauseLateralToggle->refresh();
  setSpeedOffsetToggle->refresh();
  stoppingDistanceToggle->refresh();

  previousIsMetric = isMetric;
}

void FrogPilotControlsPanel::parentToggleClicked() {
  aggressiveProfile->setVisible(false);
  conditionalSpeedsImperial->setVisible(false);
  conditionalSpeedsMetric->setVisible(false);
  modelSelectorButton->setVisible(false);
  slscPriorityButton->setVisible(false);
  standardProfile->setVisible(false);
  relaxedProfile->setVisible(false);

  this->openParentToggle();
}

void FrogPilotControlsPanel::hideSubToggles() {
  aggressiveProfile->setVisible(false);
  conditionalSpeedsImperial->setVisible(false);
  conditionalSpeedsMetric->setVisible(false);
  modelSelectorButton->setVisible(true);
  slscPriorityButton->setVisible(false);
  standardProfile->setVisible(false);
  relaxedProfile->setVisible(false);

  for (auto &[key, toggle] : toggles) {
    bool subToggles = conditionalExperimentalKeys.find(key.c_str()) != conditionalExperimentalKeys.end() ||
                      fireTheBabysitterKeys.find(key.c_str()) != fireTheBabysitterKeys.end() ||
                      laneChangeKeys.find(key.c_str()) != laneChangeKeys.end() ||
                      lateralTuneKeys.find(key.c_str()) != lateralTuneKeys.end() ||
                      longitudinalTuneKeys.find(key.c_str()) != longitudinalTuneKeys.end() ||
                      mtscKeys.find(key.c_str()) != mtscKeys.end() ||
                      qolKeys.find(key.c_str()) != qolKeys.end() ||
                      speedLimitControllerKeys.find(key.c_str()) != speedLimitControllerKeys.end() ||
                      visionTurnControlKeys.find(key.c_str()) != visionTurnControlKeys.end();
    toggle->setVisible(!subToggles);
  }

  this->closeParentToggle();
}

void FrogPilotControlsPanel::hideEvent(QHideEvent *event) {
  hideSubToggles();
}

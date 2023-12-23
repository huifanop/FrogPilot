#include <filesystem>

#include "selfdrive/frogpilot/ui/frogpilot_settings.h"

FrogPilotControlsPanel::FrogPilotControlsPanel(QWidget *parent) : FrogPilotPanel(parent) {
  setParams();

  mainLayout = new QVBoxLayout(this);

  QLabel *const descriptionLabel = new QLabel("點選設定標題顯示詳細說明", this);
  mainLayout->addWidget(descriptionLabel);
  mainLayout->addSpacing(25);
  mainLayout->addWidget(whiteHorizontalLine());

  static const std::vector<std::tuple<QString, QString, QString, QString>> toggles = {
    {"Model", "模型選擇 (需要重啟)", "選擇你想使用的OP模型.\n\nFV = Farmville(Default)\nNLP = New Lemon Pie", "../assets/offroad/icon_calibration.png"},
    {"MTSCEnabled", "地圖彎道速度控制", "When enabled, the car will slow down when it predicts a lateral acceleration greater than 2.0 m/s^2.", "../assets/offroad/icon_speed_map.png"},
    {"SpeedLimitController", "限速控制器", "使用 Open Street Maps、Navigate On openpilot 和汽車儀表板將車輛速度設定為當前速度限制.", "../assets/offroad/icon_speed_limit.png"},
    {"VisionTurnControl", "視覺轉向速度控制", "根據路面曲率自動調整車速，轉彎更順暢.", "../assets/offroad/icon_vtc.png"},
    {"TurnDesires", "意圖轉彎", "打開此選項在低於最低自動換道時速40KMH以下時打方向燈時獲得更精準的轉彎.", "../assets/navigation/direction_continue_right.png"},
    {"ExperimentalModeViaPress", "利用畫面或方向盤開啟實驗模式", "通過雙擊方向盤上的“車道偏離”/LKAS 按鈕(Toyota/Lexus Only)以啟用或禁用實驗模式，或雙擊營幕覆蓋“條件實驗模式”'. ", "../assets/img_experimental_white.svg"},
    {"AdjustablePersonalities", "駕駛模式", "透過畫面切換駕駛模式.\n\n1 格 = 積極\n2 格 = 標準\n3 格 = 輕鬆", "../assets/offroad/icon_distance.png"},
    {"CustomPersonalities", "設定駕駛模式", "根據您的喜好設定駕駛模式細項設定.", "../assets/offroad/icon_custom.png"},
    {"AlwaysOnLateral", "全時置中模式", "使用剎車或油門踏板時仍保持橫向控制。只有停用“定速”後才能解除.", "../assets/offroad/icon_always_on_lateral.png"},
    {"ConditionalExperimental", "條件式的實驗模式", "根據特定條件自動啟動實驗模式.", "../assets/offroad/icon_conditional.png"},
    {"FireTheBabysitter", "關閉監控", "禁用 openpilot 的一些‘保姆協議’", "../assets/offroad/icon_babysitter.png"},
    {"DeviceShutdown", "設備自動關機設定", "設置設備在熄火後自動關閉的時間，以減少能源浪費並防止電池耗盡.", "../assets/offroad/icon_time.png"},
    {"LateralTune", "橫向調整", "改變 openpilot 的駕駛方式.", "../assets/offroad/icon_lateral_tune.png"},
    {"LongitudinalTune", "縱向調整", "改變 openpilot 加速和煞車方式.", "../assets/offroad/icon_longitudinal_tune.png"},   
    {"NudgelessLaneChange", "自動變換車道", "不需輕推方向盤即可變換車道.", "../assets/offroad/icon_lane.png"},
    {"PauseLateralOnSignal", "打方向燈時暫停橫向控制", "打方向燈時暫停橫向控制.", "../assets/offroad/icon_pause_lane.png"}
  };

  for (const auto &[key, label, desc, icon] : toggles) {
    ParamControl *control = createParamControl(key, label, desc, icon, this);
    if (key == "AdjustablePersonalities") {
      mainLayout->addWidget(new AdjustablePersonalities());
      mainLayout->addWidget(horizontalLine());
    } else if (key == "AlwaysOnLateral") {
      createSubControl(key, label, desc, icon, {}, {
        {"AlwaysOnLateralMain", "開啟定速啟用全時", "只需開啟定速即可啟用全時功能，而不需要先啟用 openpilot"}
      });
    } else if (key == "ConditionalExperimental") {
      createSubControl(key, label, desc, icon, {
        createDualParamControl(new CESpeed(), new CESpeedLead()),
      });
      createSubButtonControl(key, {
        {"CECurves", "過彎"},
        {"CECurvesLead", "有前車過彎"},
        {"CENavigation", "導航因素"}
      }, mainLayout);
      createSubButtonControl(key, {
        {"CESlowerLead", "低速前車"},
        {"CEStopLights", "停止標誌"},
        {"CESignal", "方向燈 < " + QString(isMetric ? "90公里" : "55mph")}
      }, mainLayout);
    } else if (key == "CustomPersonalities") {
      createSubControl(key, label, desc, icon, {
        createDualParamControl(new AggressiveFollow(), new AggressiveJerk()),
        createDualParamControl(new StandardFollow(), new StandardJerk()),
        createDualParamControl(new RelaxedFollow(), new RelaxedJerk()),
      });
    } else if (key == "DeviceShutdown") {
      mainLayout->addWidget(new DeviceShutdown());
      mainLayout->addWidget(horizontalLine());
    } else if (key == "FireTheBabysitter") {
      createSubControl(key, label, desc, icon, {}, {
        {"NoLogging", "關閉紀錄", "防止任何資料被COMMA追蹤或降低機器溫度\n\n注意這將清除所有紀錄而且不能回復!"}
      });
      createSubButtonControl(key, {
        {"MuteDM", "駕駛監控"},
        {"MuteDoor", "車門"},
        {"MuteOverheated", "系統過熱"},
        {"MuteSeatbelt", "安全帶"}
      }, mainLayout);
    } else if (key == "LateralTune") {
      createSubControl(key, label, desc, icon, {}, {
        {"AverageCurvature", "平均期望曲率", "使用 Pfeiferj 的以距離為基準的曲率調整方法來更平滑地處理轉彎"},
        {"NNFF", "NNFF - 神經網路前饋", "使用Twilsonco's的神經網路前饋扭矩控制系統來獲得更精準的橫向控制"}
      });
    } else if (key == "LongitudinalTune") {
      createSubControl(key, label, desc, icon, {
        new AccelerationProfile(),
        new StoppingDistance(),
      }, {
        {"AggressiveAcceleration", "積極加速跟車", "當有前車可跟隨時起步更加積極的加速"},
        {"SmoothBraking", "平穩煞車的跟車", "當接近速度較慢的車輛時，煞車行為更加自然"}
      });
    } else if (key == "Model") {
      mainLayout->addWidget(new Model());
      mainLayout->addWidget(horizontalLine());
    } else if (key == "NudgelessLaneChange") {
      createSubControl(key, label, desc, icon, {
        new LaneChangeTime(),
      });
      createSubButtonControl(key, {
        {"LaneDetection", "車道檢測"},
        {"OneLaneChange", "每次只變換一個車道"}
      }, mainLayout);
    } else if (key == "SpeedLimitController") {
      std::vector<QWidget*> widgets;
      widgets.push_back(new SLCFallback());
      widgets.push_back(new SLCPriority());

      if (isMetric) {
        widgets.push_back(createDualParamControl(new Offset1Metric(), new Offset2Metric()));
        widgets.push_back(createDualParamControl(new Offset3Metric(), new Offset4Metric()));
      } else {
        widgets.push_back(createDualParamControl(new Offset1(), new Offset2()));
        widgets.push_back(createDualParamControl(new Offset3(), new Offset4()));
      }
      createSubControl(key, label, desc, icon, widgets);
    } else if (key == "VisionTurnControl") {
      createSubControl(key, label, desc, icon, {
        new CurveSensitivity(),
        new TurnAggressiveness(),
      });
    } else {
      mainLayout->addWidget(control);
      if (key != std::get<0>(toggles.back())) mainLayout->addWidget(horizontalLine());
    }
  }
  setInitialToggleStates();
}

FrogPilotVisualsPanel::FrogPilotVisualsPanel(QWidget *parent) : FrogPilotPanel(parent) {
  mainLayout = new QVBoxLayout(this);

  QLabel *const descriptionLabel = new QLabel("點選設定標題顯示詳細說明", this);
  mainLayout->addWidget(descriptionLabel);
  mainLayout->addSpacing(25);
  mainLayout->addWidget(whiteHorizontalLine());

  static const std::vector<std::tuple<QString, QString, QString, QString>> toggles = {
    {"SilentMode", "靜音模式", "關閉所有聲音保持完全靜音的運作.", "../assets/offroad/icon_mute.png"},
    {"GreenLightAlert", "綠燈提醒", "交通號誌從紅燈轉為綠燈時產生提示.", "../assets/offroad/icon_green_light.png"},
    {"CameraView", "相機視圖（僅限外觀）", "為 UI 設定您首選的相機視圖。此切換純粹是裝飾性的，不會影響openpilot 對其他相機的使用.", "../assets/offroad/icon_camera.png"},
    {"CustomUI", "自定義道路畫面", "定義自己喜歡的道路介面.", "../assets/offroad/icon_road.png"},
    {"ScreenBrightness", "螢幕亮度", "自行設定螢幕亮度或使用預設自動亮度設置.", "../assets/offroad/icon_light.png"},
    {"CustomTheme", "自訂外觀主題", "啟動後使用自訂外觀，關閉則為官方外觀.", "../assets/frog.png"},
    {"Compass", "指南針", "畫面中添加指南針，顯示您的行駛方位.", "../assets/offroad/icon_compass.png"},
    {"DriverCamera", "倒車顯示駕駛鏡頭", "倒車時顯示駕駛畫面.", "../assets/img_driver_face_static.png"},
    {"RotatingWheel", "旋轉方向盤", "畫面右上角的方向盤與方向盤同步旋轉.", "../assets/offroad/icon_rotate.png"},
    {"WheelIcon", "方向盤圖示", "用自定義圖示替換 openpilot 方向盤圖標!", "../assets/offroad/icon_openpilot.png"},
  };

  for (const auto &[key, label, desc, icon] : toggles) {
    ParamControl *control = createParamControl(key, label, desc, icon, this);
    if (key == "CameraView") {
      mainLayout->addWidget(new CameraView());
      mainLayout->addWidget(horizontalLine());
    } else if (key == "CustomUI") {
      createSubControl(key, label, desc, icon, {
        createDualParamControl(new LaneLinesWidth(), new RoadEdgesWidth()),
        createDualParamControl(new PathWidth(), new PathEdgeWidth())
      });
      createSubButtonControl(key, {
        {"AccelerationPath", "加速路徑"},
        {"AdjacentPath", "相鄰路徑"},
        {"BlindSpotPath", "盲點路徑"}
      }, mainLayout);
      createSubButtonControl(key, {
        {"ShowFPS", "顯示 FPS"},
        {"LeadInfo", "前車資訊"},
        {"RoadNameUI", "道路名稱"}
      }, mainLayout);
      createSubButtonControl(key, {
        {"UnlimitedLength", "“無限”道路畫面長度"}
      }, mainLayout);
    } else if (key == "CustomTheme") {
      createSubControl(key, label, desc, icon, {
        createDualParamControl(new CustomColors(), new CustomIcons()),
        createDualParamControl(new CustomSignals(), new CustomSounds()),
      });
    } else if (key == "ScreenBrightness") {
      mainLayout->addWidget(new ScreenBrightness());
      mainLayout->addWidget(horizontalLine());
    } else if (key == "WheelIcon") {
      mainLayout->addWidget(new WheelIcon());
      mainLayout->addWidget(horizontalLine());
    } else {
      mainLayout->addWidget(control);
      if (key != std::get<0>(toggles.back())) mainLayout->addWidget(horizontalLine());
    }
  }
  setInitialToggleStates();
}

///////////////////////////////////////////////////////////////////////////////////////////
HFOPControlsPanel::HFOPControlsPanel(QWidget *parent) : FrogPilotPanel(parent) {
  setParams();
  mainLayout = new QVBoxLayout(this);

  QLabel *const descriptionLabel = new QLabel("點選設定標題顯示詳細說明", this);
  mainLayout->addWidget(descriptionLabel);
  mainLayout->addSpacing(25);
  mainLayout->addWidget(whiteHorizontalLine());

  // QHBoxLayout *forceFingerprintLayout = new QHBoxLayout();
  // forceFingerprintLayout->setSpacing(25);
  // forceFingerprintLayout->setContentsMargins(0, 0, 0, 0);

  // QLabel *forceFingerprintLabel = new QLabel(tr("指定車型"));
  // forceFingerprintLayout->addWidget(forceFingerprintLabel);

  // forceFingerprintLayout->addStretch(1);

  // QString currentCarModel = QString::fromStdString(params.get("CarModel"));
  // QLabel *carModelLabel = new QLabel(currentCarModel);
  // forceFingerprintLayout->addWidget(carModelLabel);

  // ButtonControl *forceFingerprintButton = new ButtonControl(tr(""), tr("選擇"));
  // forceFingerprintLayout->addWidget(forceFingerprintButton);

  // connect(forceFingerprintButton, &ButtonControl::clicked, this, [=]() {
  //   std::system("python3 /data/openpilot/scripts/set_fingerprints.py");
  //   std::string carModels = params.get("CarModels");

  //   QStringList cars = QString::fromStdString(carModels).split(',');
  //   QString selection = MultiOptionDialog::getSelection(tr("選擇您的車型"), cars, currentCarModel, this);

  //   if (!selection.isEmpty()) {
  //     params.put("CarModel", selection.toStdString());
  //     carModelLabel->setText(selection);
  //   }
  // });

  // mainLayout->addLayout(forceFingerprintLayout);
  // mainLayout->addWidget(whiteHorizontalLine());


  static const std::vector<std::tuple<QString, QString, QString, QString>> toggles = {
    {"AutoACC", "自動啟動ACC", "啟用後當速度大於設定，會自動啟動ACC，低於於10公里煞車會解除ACC.", "../assets/offroad/icon_conditional.png"},
    {"CarAway", "前車遠離偵測", "啟用後當前車速度改變時會有提醒.", "../assets/offroad/icon_warning.png"},
    {"Roadtype", "道路種類設定", "開啟後可依道路種類在特定條件下預設時速", "../assets/offroad/icon_road.png"},
    {"Speeddistance", "車速調控跟車距離", "開啟後可依行車路線自動切換跟車距離， 1格 60公里 2格90公里 3格120公里.", "../assets/offroad/icon_distance.png"},
    {"Navspeed", "圖資速限", "開啟後可當下所在道路的圖資速限自動更新.", "../assets/offroad/icon_map.png"},
    {"Emergencycontrol", "緊急調控車速", "開啟後可當前方車輛快速減速時，會自動調降速限.", "../assets/offroad/icon_warning.png"},
    // {"speedoverreminder", "超速提醒", "當車速高於40公里，且速度高於最高速限時會有提醒，並自動重新設定圖資最高速限.", "../assets/offroad/icon_vtc.png"},
    {"Voicereminder", "中文語音提醒", "開啟後可在特定條件下發出語音提醒，特定條件如：綠燈.前車遠離.變換車道.前車減速時會有中文語音提醒，不同模型偵測結果會有差異.", "../assets/offroad/icon_custom.png"}
  };

  for (const auto &[key, label, desc, icon] : toggles) {
    ParamControl *control = createParamControl(key, label, desc, icon, this);
    if (key == "AutoACC") {
      createSubControl(key, label, desc, icon, {
        new AutoACCspeed(),
      });
      createSubButtonControl(key, {
        {"AutoACCCarAway", "前車遠離啟動"},
        {"AutoACCGreenLight", "綠燈啟動"}
      }, mainLayout);
    } else if (key == "Roadtype") {
      createSubControl(key, label, desc, icon, {
        new RoadtypeProfile(),
      });
    } else if (key == "CarAway") {
      createSubControl(key, label, desc, icon, {
        new CarAwayspeed(),
        new CarAwaydistance(),
      });
    } else if (key == "Voicereminder") {
      createSubControl(key, label, desc, icon, {
      });
      createSubButtonControl(key, {
        {"GreenLightReminder", "綠燈語音"},
        {"ChangeLaneReminder", "變換車道"},
        {"Laneblindspotdetection", "換道盲點"},
        {"CarApproachingReminder", "前車急煞"},
        {"CarAwayReminder", "前車遠離"}
      }, mainLayout);
      // createSubButtonControl(key, {  
        
      //   {"CarAwayReminder", "前車遠離語音"}
      // }, mainLayout);
    } else if (key == "Navspeed") {
      createSubControl(key, label, desc, icon, {
      });
      createSubButtonControl(key, {
        {"NavReminder", "導航語音"},
        {"speedoverreminder", "超速提醒與重設速限"},
        {"SpeedlimituReminder", "速限變更提醒"}
      }, mainLayout);
    } else {
      mainLayout->addWidget(control);
      if (key != std::get<0>(toggles.back())) mainLayout->addWidget(horizontalLine());
    }
  }
  setInitialToggleStates();
}

ParamControl *FrogPilotPanel::createParamControl(const QString &key, const QString &label, const QString &desc, const QString &icon, QWidget *parent) {
  ParamControl *control = new ParamControl(key, label, desc, icon);
  connect(control, &ParamControl::toggleFlipped, [=](bool state) {
    paramsMemory.putBoolNonBlocking("FrogPilotTogglesUpdated", true);

    if (key == "NNFF") {
      if (params.getBool("NNFF")) {
        const bool addSSH = ConfirmationDialog::yesorno("Would you like to grant 'twilsonco' SSH access to improve NNFF? This won't affect any added SSH keys.", parent);
        params.putBoolNonBlocking("TwilsoncoSSH", addSSH);
        if (addSSH) {
          ConfirmationDialog::toggleAlert("Message 'twilsonco' on Discord to get your device properly configured.", "Acknowledge", parent);
        }
      }
    }

    static const QMap<QString, QString> parameterWarnings = {
      {"AggressiveAcceleration", "這將使 openpilot 駕駛更加積極!"},
      {"AlwaysOnLateralMain", "這是非常實驗性的，不能保證有效。!"},
      {"SmoothBraking", "這會修改openpilot 煞車模式!"},
      {"TSS2Tune", "這會修改openpilot 油門與煞車模式!"}
    };
    if (parameterWarnings.contains(key) && params.getBool(key.toStdString())) {
      ConfirmationDialog::toggleAlert("WARNING: " + parameterWarnings[key], "I understand the risks.", parent);
    }

    static const QSet<QString> parameterReboots = {
      "AlwaysOnLateral", "AlwaysOnLateralMain", "FireTheBabysitter", "NoLogging", "NNFF",
    };
    if (parameterReboots.contains(key)) {
      if (ConfirmationDialog::toggle("Reboot required to take effect.", "Reboot Now", parent)) {
        Hardware::reboot();
      }
    }

    auto it = childControls.find(key.toStdString());
    if (it != childControls.end()) {
      for (QWidget *widget : it->second) {
        widget->setVisible(state);
      }
    }

    if (key == "ConditionalExperimental") {
      params.putBoolNonBlocking("ExperimentalMode", state);
    }
  });
  return control;
}

QFrame *FrogPilotPanel::horizontalLine(QWidget *parent) const {
  QFrame *line = new QFrame(parent);

  line->setFrameShape(QFrame::StyledPanel);
  line->setStyleSheet(R"(
    border-width: 1px;
    border-bottom-style: solid;
    border-color: gray;
  )");
  line->setFixedHeight(2);

  return line;
}

QFrame *FrogPilotPanel::whiteHorizontalLine(QWidget *parent) const {
  QFrame *line = new QFrame(parent);

  line->setFrameShape(QFrame::StyledPanel);
  line->setStyleSheet(R"(
    border-width: 1px;
    border-bottom-style: solid;
    border-color: white;
  )");
  line->setFixedHeight(2);

  return line;
}

QWidget *FrogPilotPanel::createDualParamControl(ParamValueControl *control1, ParamValueControl *control2) {
  QWidget *mainControl = new QWidget(this);
  QHBoxLayout *layout = new QHBoxLayout();

  layout->addWidget(control1);
  layout->addStretch();
  layout->addWidget(control2);
  mainControl->setLayout(layout);

  return mainControl;
}

QWidget *FrogPilotPanel::addSubControls(const QString &parentKey, QVBoxLayout *layout, const std::vector<std::tuple<QString, QString, QString>> &controls) {
  QWidget *mainControl = new QWidget(this);

  mainControl->setLayout(layout);
  mainLayout->addWidget(mainControl);
  mainControl->setVisible(params.getBool(parentKey.toStdString()));

  for (const auto &[key, label, desc] : controls) addControl(key, "   " + label, desc, layout);

  return mainControl;
}

void FrogPilotPanel::addControl(const QString &key, const QString &label, const QString &desc, QVBoxLayout *layout, const QString &icon) {
  layout->addWidget(createParamControl(key, label, desc, icon, this));
  layout->addWidget(horizontalLine());
}

void FrogPilotPanel::createSubControl(const QString &key, const QString &label, const QString &desc, const QString &icon, const std::vector<QWidget*> &subControls, const std::vector<std::tuple<QString, QString, QString>> &additionalControls) {
  ParamControl *control = createParamControl(key, label, desc, icon, this);

  mainLayout->addWidget(control);
  mainLayout->addWidget(horizontalLine());

  QVBoxLayout *subControlLayout = new QVBoxLayout();

  for (QWidget *subControl : subControls) {
    subControlLayout->addWidget(subControl);
    subControlLayout->addWidget(horizontalLine());
  }

  QWidget *mainControl = addSubControls(key, subControlLayout, additionalControls);

  connect(control, &ParamControl::toggleFlipped, [=](bool state) { mainControl->setVisible(state); });
}

void FrogPilotPanel::createSubButtonControl(const QString &parentKey, const std::vector<QPair<QString, QString>> &buttonKeys, QVBoxLayout *subControlLayout) {
  QHBoxLayout *buttonsLayout = new QHBoxLayout();
  QWidget *line = horizontalLine();

  buttonsLayout->addStretch();

  for (const auto &[key, label] : buttonKeys) {
    FrogPilotButtonParamControl* button = new FrogPilotButtonParamControl(key, label);
    mainLayout->addWidget(button);
    buttonsLayout->addWidget(button);
    buttonsLayout->addStretch();
    button->setVisible(params.getBool(parentKey.toStdString()));
    childControls[parentKey.toStdString()].push_back(button);
  }

  subControlLayout->addLayout(buttonsLayout);

  line = horizontalLine();
  mainLayout->addWidget(line);

  childControls[parentKey.toStdString()].push_back(line);
}

void FrogPilotPanel::setInitialToggleStates() {
  for (const auto& [key, controlSet] : childControls) {
    bool state = params.getBool(key);
    for (QWidget *widget : controlSet) {
      widget->setVisible(state);
    }
  }
}

void FrogPilotPanel::setParams() {
  if (!std::filesystem::exists("/data/openpilot/selfdrive/modeld/models/supercombo.thneed")) {
    params.putBool("DoReboot", true);
  }

  if (params.getBool("DisableOnroadUploads")) {
    paramsMemory.putBool("DisableOnroadUploads", true);
  }
  if (params.getBool("FireTheBabysitter") and params.getBool("MuteDM")) {
    paramsMemory.putBool("MuteDM", true);
  }
  if (params.getBool("FireTheBabysitter") and params.getBool("NoLogging")) {
    paramsMemory.putBool("NoLogging", true);
  }
  if (params.getBool("RoadNameUI") || params.getBool("SpeedLimitController")) {
    paramsMemory.putBool("OSM", true);
  }

  const bool FrogsGoMoo = params.get("DongleId").substr(0, 3) == "be6";

  const std::map<std::string, std::string> default_values {
    {"AccelerationPath", "1"},
    {"AccelerationProfile", "1"},
    {"AdjacentPath", "1"},
    {"AdjustablePersonalities", "3"},
    {"AggressiveAcceleration", "1"},
    {"AggressiveFollow", FrogsGoMoo ? "10" : "12"},
    {"AggressiveJerk", FrogsGoMoo ? "6" : "5"},
    {"AlwaysOnLateral", "1"},
    {"AlwaysOnLateralMain", FrogsGoMoo ? "1" : "1"},
    {"AverageCurvature", FrogsGoMoo ? "1" : "1"},
    {"BlindSpotPath", "1"},
    {"CameraView", FrogsGoMoo ? "1" : "0"},
    {"CECurves", "1"},
    {"CECurvesLead", "1"},
    {"CENavigation", "1"},
    {"CESignal", "1"},
    {"CESlowerLead", "1"},
    {"CESpeed", "0"},
    {"CESpeedLead", "0"},
    {"CEStopLights", "1"},
    {"Compass", "0"},
    {"ConditionalExperimental", "1"},
    {"CurveSensitivity", FrogsGoMoo ? "125" : "180"},
    {"CustomColors", "0"},
    {"CustomIcons", "0"},
    {"CustomPersonalities", "1"},
    {"CustomSignals", "0"},
    {"CustomSounds", "0"},
    {"CustomTheme", "0"},
    {"CustomUI", "1"},
    {"DeviceShutdown", "1"},
    {"DriverCamera", "0"},
    {"EVTable", "0"},
    {"ExperimentalModeViaPress", "1"},
    {"FireTheBabysitter", FrogsGoMoo ? "1" : "1"},
    {"GreenLightAlert", "1"},
    {"LaneChangeTime", "1"},
    {"LaneDetection", "1"},
    {"LaneLinesWidth", "4"},
    {"LateralTune", "1"},
    {"LeadInfo", "1"},
    {"LockDoors", "1"},
    {"LongitudinalTune", "1"},
    {"LowerVolt", "0"},
    {"Model", "0"},
    {"MuteDM", "1"},
    {"MuteDoor", "1"},
    {"MuteOverheated", "1"},
    {"MuteSeatbelt", "1"},
    {"MTSCEnabled", "1"},
    {"NNFF", FrogsGoMoo ? "1" : "1"},
    {"NudgelessLaneChange", "1"},
    {"NumericalTemp", "1"},
    {"Offset1", "0"},
    {"Offset2", FrogsGoMoo ? "7" : "0"},
    {"Offset3", "0"},
    {"Offset4", FrogsGoMoo ? "20" : "0"},
    {"OneLaneChange", "1"},
    {"PathEdgeWidth", "20"},
    {"PathWidth", "20"},
    {"PauseLateralOnSignal", "0"},
    {"RelaxedFollow", "30"},
    {"RelaxedJerk", "50"},
    {"RoadEdgesWidth", "2"},
    {"RoadNameUI", "1"},
    {"RotatingWheel", "0"},
    {"SLCFallback", "0"},
    {"SLCPriority", "1"},
    {"SNGHack", "0"},
    {"ScreenBrightness", "50"},
    {"ShowCPU", FrogsGoMoo ? "1" : "0"},
    {"ShowMemoryUsage", FrogsGoMoo ? "1" : "0"},
    {"ShowFPS", FrogsGoMoo ? "1" : "1"},
    {"Sidebar", "0"},
    {"SilentMode", "0"},
    {"SmoothBraking", "1"},
    {"SpeedLimitController", "1"},
    {"StandardFollow", "15"},
    {"StandardJerk", "10"},
    {"StoppingDistance", FrogsGoMoo ? "6" : "1"},
    {"TSS2Tune", "1"},
    {"TurnAggressiveness", FrogsGoMoo ? "85" : "150"},
    {"TurnDesires", "1"},
    {"UnlimitedLength", "1"},
    {"VisionTurnControl", "1"},
    {"WheelIcon", "0"},
    //Fan///////////////////
    {"AutoACC", "1"},
    {"AutoACCspeed", "10"},
    {"AutoACCCarAway", "1"},
    {"AutoACCGreenLight", "1"},
    {"CarApproachingReminder", "1"},
    {"CarAway", "1"},
    {"CarAwayspeed", "5"},
    {"CarAwaydistance", "5"},
    {"CarAwayReminder", "1"},
    {"ChangeLaneReminder", "1"},
    {"GreenLightReminder", "1"},    
    {"Laneblindspotdetection", "1"},
    {"Navspeed", "1"},
    {"NavReminder", "1"},
    {"Emergencycontrol", "1"},
    {"Roadtype", "1"},
    {"RoadtypeProfile", "1"},
    {"speedoverreminder", "1"},
    {"Speeddistance", "1"},
    {"Voicereminder", "1"}    
    ////////////////////////
  };

  bool rebootRequired = false;
  for (const auto& [key, value] : default_values) {
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

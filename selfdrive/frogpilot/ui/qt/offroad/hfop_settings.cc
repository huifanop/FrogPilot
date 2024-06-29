#include "selfdrive/frogpilot/ui/qt/offroad/hfop_settings.h"

HFOPControlsPanel::HFOPControlsPanel(SettingsWindow *parent) : FrogPilotListWidget(parent) {
  std::string branch = params.get("GitBranch");
  isRelease = branch == "FrogPilot";

  const std::vector<std::tuple<QString, QString, QString, QString>> hfopToggles {
    {"HFOPinf", "  訊息框", "主畫面左下方顯示訊息狀態.", ""},

    {"Faststart", "快速開機", "啟動後開機會加速.", "../frogpilot/assets/toggle_icons/icon_light.png"},
    {"Fuelprice", "油價計算", "啟動後會計算油費.", "../frogpilot/assets/toggle_icons/icon_light.png"},
    {"Fuelcosts", "油價設定", "設定車輛使用油種與價格.", ""},

    {"VagSpeed", "時速差調整", "VAG專用。調整車錶速度與C3定速設定不同步的問題。", "../assets/offroad/icon_openpilot.png"},
    {"VagSpeedFactor", "  時速差調整", "請輸入OP定速為110時儀表板的速度差值.", ""},
    {"Disablestartstop", "取消怠速熄火", "開啟後將強制關閉怠速熄火功能.", "../assets/offroad/icon_warning.png"},

    {"AutoACC", "自動啟動ACC", "啟用後會自動啟動ACC.", "../assets/offroad/icon_conditional.png"},
    {"AutoACCspeed", "  自動啟動ACC時速設定", "設定自動啟動ACC的時速條件設定.", ""},
    {"AutoACCCarAway", "  前車遠離啟動", "啟用後當前方車輛遠離會自動啟動ACC.", ""},
    {"AutoACCGreenLight", "  綠燈啟動", "啟用後當偵測到綠燈時會自動啟動ACC.", ""},
    {"TrafficModespeed", "  塞車模式時速設定", "低於此速度將自動啟動塞車模式.", ""},

    {"Roadtype", "道路種類設定", "開啟後可依道路種類在特定條件下預設時速", "../assets/offroad/icon_road.png"},
    {"RoadtypeProfile", "道路種類設定", "開啟後可依道路種類在特定條件下預設時速", "../assets/offroad/icon_road.png"},

    {"Speeddistance", "車速調控跟車距離", "開啟後可依行車路線自動切換跟車距離， 1格 60公里 2格90公里 3格120公里.", "../assets/offroad/icon_distance.png"},

    {"Navspeed", "圖資速限", "開啟後可依當下所在道路的圖資速限自動更新.", "../assets/offroad/icon_map.png"},
    {"NavReminder", "  導航語音", "開啟後若使用道路導航時會播報轉彎語音訊息.", ""},
    {"speedoverreminder", "  超速提醒", "開啟後若當下速度高於圖資速限會發出提醒.", ""},
    {"speedreminderreset", "  超速重設速限", "開啟後若當下速度高於圖資速限會強制重設速限.", ""},

    {"Dooropen", "  車門開啟", "開啟後在引擎啟動狀態下駕駛車門開啟或後車箱未關閉時會發出提醒.", "../assets/offroad/icon_warning.png"},
    {"DriverdoorOpen", "  駕駛車門", "開啟後在引擎啟動狀態下駕駛車門開啟時會發出提醒.", ""},
    {"CodriverdoorOpen", "  副駕駛車門", "開啟後在引擎啟動狀態下副駕駛車門開啟時會發出提醒.", ""},
    {"LpassengerdoorOpen", "  左乘客車門", "開啟後在引擎啟動狀態下左乘客車門開啟時會發出提醒.", ""},
    {"RpassengerdoorOpen", "  右乘客車門", "開啟後在引擎啟動狀態下右乘客車門開啟時會發出提醒.", ""},
    {"LuggagedoorOpen", "  後車門開啟", "開啟後在引擎啟動狀態下候車門開啟時會發出提醒.", ""},

  };

  for (const auto &[param, title, desc, icon] : hfopToggles) {
    AbstractControl *toggle;

    if (param == "Fuelprice") {
      FrogPilotParamManageControl *FuelpriceToggle = new FrogPilotParamManageControl(param, title, desc, icon, this);
      QObject::connect(FuelpriceToggle, &FrogPilotParamManageControl::manageButtonClicked, this, [this]() {
        openParentToggle();
        for (auto &[key, toggle] : toggles) {
          toggle->setVisible(FuelpriceKeys.find(key.c_str()) != FuelpriceKeys.end());
        }
      });
      toggle = FuelpriceToggle;

    } else if (param == "Fuelcosts") {
      toggle = new FrogPilotParamValueControl(param, title, desc, icon, 300, 350, std::map<int, QString>(), this, false, "元", 10);

    } else if (param == "VagSpeed") {
      FrogPilotParamManageControl *VagSpeedToggle = new FrogPilotParamManageControl(param, title, desc, icon, this);
      QObject::connect(VagSpeedToggle, &FrogPilotParamManageControl::manageButtonClicked, this, [this]() {
        openParentToggle();
        for (auto &[key, toggle] : toggles) {
          toggle->setVisible(VagSpeedKeys.find(key.c_str()) != VagSpeedKeys.end());
        }
      });
      toggle = VagSpeedToggle;
    } else if (param == "VagSpeedFactor") {
      toggle = new FrogPilotParamValueControl(param, title, desc, icon, 0, 20, std::map<int, QString>(), this, false, "公里", 2);

    } else if (param == "AutoACC") {
      FrogPilotParamManageControl *AutoACCToggle = new FrogPilotParamManageControl(param, title, desc, icon, this);
      QObject::connect(AutoACCToggle, &FrogPilotParamManageControl::manageButtonClicked, this, [this]() {
        openParentToggle();
        for (auto &[key, toggle] : toggles) {
          toggle->setVisible(AutoACCKeys.find(key.c_str()) != AutoACCKeys.end());
        }
      });
      toggle = AutoACCToggle;

    } else if (param == "AutoACCspeed") {
      toggle = new FrogPilotParamValueControl(param, title, desc, icon, 1, 50, std::map<int, QString>(), this, false, "公里");

    } else if (param == "TrafficModespeed") {
      toggle = new FrogPilotParamValueControl(param, title, desc, icon, 0, 60, std::map<int, QString>(), this, false, "公里");

    } else if (param == "Roadtype") {
      FrogPilotParamManageControl *RoadToggle = new FrogPilotParamManageControl(param, title, desc, icon, this);
      QObject::connect(RoadToggle, &FrogPilotParamManageControl::manageButtonClicked, this, [this]() {
        openParentToggle();
        for (auto &[key, toggle] : toggles) {
          toggle->setVisible(RoadKeys.find(key.c_str()) != RoadKeys.end());
        }
      });
      toggle = RoadToggle;
    } else if (param == "RoadtypeProfile") {
      std::vector<QString> profileOptions{tr("自動"), tr("平面"), tr("快速"), tr("高速"), tr("關閉")};
      FrogPilotButtonParamControl *profileSelection = new FrogPilotButtonParamControl(param, title, desc, icon, profileOptions);
      toggle = profileSelection;

    } else if (param == "Navspeed") {
      FrogPilotParamManageControl *NavspeedToggle = new FrogPilotParamManageControl(param, title, desc, icon, this);
      QObject::connect(NavspeedToggle, &FrogPilotParamManageControl::manageButtonClicked, this, [this]() {
        openParentToggle();
        for (auto &[key, toggle] : toggles) {
          toggle->setVisible(NavspeedKeys.find(key.c_str()) != NavspeedKeys.end());
        }
      });
      toggle = NavspeedToggle;

    } else if(param == "Dooropen") {
      FrogPilotParamManageControl *DooropenToggle = new FrogPilotParamManageControl(param, title, desc, icon, this);
      QObject::connect(DooropenToggle, &FrogPilotParamManageControl::manageButtonClicked, this, [this]() {
        openParentToggle();
        for (auto &[key, toggle] : toggles) {
          toggle->setVisible(DooropenKeys.find(key.c_str()) != DooropenKeys.end());
        }
      });
      toggle = DooropenToggle;

    } else if(param == "Dooropentype") {
      std::vector<QString> adjustablePersonalitiesToggles{"DriverdoorOpen", "CodriverdoorOpen", "LpassengerdoorOpen", "RpassengerdoorOpen", "LuggagedoorOpen"};
      std::vector<QString> adjustablePersonalitiesNames{tr("駕駛"), tr("副駕"), tr("左乘客"), tr("右乘客"), tr("行李")};
      toggle = new FrogPilotParamToggleControl(param, title, desc, icon, adjustablePersonalitiesToggles, adjustablePersonalitiesNames);

    } else {
      toggle = new ParamControl(param, title, desc, icon, this);
    }

    addItem(toggle);
    toggles[param.toStdString()] = toggle;

    QObject::connect(static_cast<ToggleControl*>(toggle), &ToggleControl::toggleFlipped, &updateFrogPilotToggles);


    QObject::connect(static_cast<FrogPilotParamValueControl*>(toggle), &FrogPilotParamValueControl::valueChanged, &updateFrogPilotToggles);

    QObject::connect(toggle, &AbstractControl::showDescriptionEvent, [this]() {
      update();
    });

    QObject::connect(static_cast<FrogPilotParamManageControl*>(toggle), &FrogPilotParamManageControl::manageButtonClicked, this, [this]() {
      update();
    });
  }

  QObject::connect(parent, &SettingsWindow::closeParentToggle, this, &HFOPControlsPanel::hideToggles);
  QObject::connect(uiState(), &UIState::uiUpdate, this, &HFOPControlsPanel::updateState);
  // hideToggles();
}

void HFOPControlsPanel::showEvent(QShowEvent *event, const UIState &s) {
  // hasOpenpilotLongitudinal = hasOpenpilotLongitudinal && !params.getBool("DisableOpenpilotLongitudinal");
}

void HFOPControlsPanel::updateState(const UIState &s) {
  if (!isVisible()) return;

  started = s.scene.started;
}

void HFOPControlsPanel::hideToggles() {

  for (auto &[key, toggle] : toggles) {
    bool subToggles = FuelpriceKeys.find(key.c_str()) != FuelpriceKeys.end() ||
                      // ScreenKeys.find(key.c_str()) != ScreenKeys.end() ||
                      VagSpeedKeys.find(key.c_str()) != VagSpeedKeys.end() ||
                      AutoACCKeys.find(key.c_str()) != AutoACCKeys.end() ||
                      RoadKeys.find(key.c_str()) != RoadKeys.end() ||
                      NavspeedKeys.find(key.c_str()) != NavspeedKeys.end() ||
                      // VoiceKeys.find(key.c_str()) != VoiceKeys.end() ||
                      DooropenKeys.find(key.c_str()) != DooropenKeys.end() ;
    toggle->setVisible(!subToggles);
  }

  update();
}
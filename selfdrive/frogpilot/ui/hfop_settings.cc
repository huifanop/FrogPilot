#include "selfdrive/frogpilot/ui/hfop_settings.h"
#include "selfdrive/ui/ui.h"

HFOPControlsPanel::HFOPControlsPanel(SettingsWindow *parent) : FrogPilotListWidget(parent) {
  const std::vector<std::tuple<QString, QString, QString, QString>> hfopToggles {
    {"Screen", "  螢幕設定", "設定車輛起動後自動關閉螢幕的方式.", "../frogpilot/assets/toggle_icons/icon_light.png"},
    {"OffScreen", "  啟動關閉螢幕", "車輛起動後自動關閉螢幕，熄火後恢復螢幕.", ""},
    {"AutoOffScreen", "  自動關閉螢幕", "設定是否自動關閉螢幕.", ""},
    {"AutoOffScreentime", "  自動關閉螢幕時間", "自動關閉螢幕計時器.", ""},
    {"VagSpeed", "時速差調整", "VAG專用。調整車錶速度與C3定速設定不同步的問題。", "../assets/offroad/icon_openpilot.png"},
    {"VagSpeedFactor", "  時速差調整", "請輸入OP定速為110時儀表板的速度差值.", ""},
    {"Disablestartstop", "  取消怠速熄火", "開啟後將強制關閉怠速熄火功能.", "../assets/offroad/icon_warning.png"},
    
    {"AutoACC", "自動啟動ACC", "啟用後會自動啟動ACC.", "../assets/offroad/icon_conditional.png"},
    {"AutoACCspeed", "  自動啟動ACC時速設定", "設定自動啟動ACC的時速條件設定.", ""},
    {"AutoACCCarAway", "  前車遠離啟動", "啟用後當前方車輛遠離會自動啟動ACC.", ""},
    {"AutoACCGreenLight", "  綠燈啟動", "啟用後當偵測到綠燈時會自動啟動ACC.", ""},

    {"CarAway", "前車遠離偵測", "啟用後當前車速度改變時會有提醒.", "../assets/offroad/icon_warning.png"},
    {"CarAwayspeed", "  前車速度差設定", "設定前車時速大於多少公里時提醒..", ""},
    {"CarAwaydistance", "  前車距離差設定", "設定前車距離大於多少公尺時提醒..", ""},
    
    {"Roadtype", "道路種類設定", "開啟後可依道路種類在特定條件下預設時速", "../assets/offroad/icon_road.png"},
    {"RoadtypeProfile", "道路種類設定", "開啟後可依道路種類在特定條件下預設時速", "../assets/offroad/icon_road.png"},

    {"Speeddistance", "車速調控跟車距離", "開啟後可依行車路線自動切換跟車距離， 1格 60公里 2格90公里 3格120公里.", "../assets/offroad/icon_distance.png"},
    
    {"Navspeed", "圖資速限", "開啟後可依當下所在道路的圖資速限自動更新.", "../assets/offroad/icon_map.png"},
    {"NavReminder", "  導航語音", "開啟後若使用道路導航時會播報轉彎語音訊息.", ""},
    {"speedoverreminder", "  超速提醒", "開啟後若當下速度高於圖資速限會發出提醒.", ""},
    {"speedreminderreset", "  超速重設速限", "開啟後若當下速度高於圖資速限會強制重設速限.", ""},    
    {"SpeedlimituReminder", "  速限變更提醒", "開啟後若因圖資變更當下最高定速設定時會跳提醒.", ""},
    
    {"Emergencycontrol", "緊急調控車速", "開啟後可當前方車輛快速減速時，會自動調降速限.", "../assets/offroad/icon_warning.png"},
    
    {"Voicereminder", "中文語音提醒", "開啟後可在特定條件下發出語音提醒，不同模型偵測結果會有差異.", "../assets/offroad/icon_custom.png"},
    {"GreenLightReminder", "  綠燈語音", "開啟後在紅燈轉綠燈時會發出語音提醒.", ""},
    {"ChangeLaneReminder", "  變換車道", "開啟後在變換車道時會發出語音提醒.", ""},
    {"Laneblindspotdetection", "  換道盲點", "開啟後在變換車道時若盲點偵測到車輛會發出語音提醒.", ""},
    {"CarApproachingReminder", "  前車急煞", "開啟後在前車急煞時會發出語音提醒.", ""},
    {"CarAwayReminder", "  前車遠離", "開啟後在前車遠離時會發出語音提醒.", ""},
    // {"Autowifi", "  自動啟動網路分享", "開機後自動啟動網路分享.", ""},
  };

  for (const auto &[param, title, desc, icon] : hfopToggles) {
    ParamControl *toggle;

    if (param == "Screen") {
      FrogPilotParamManageControl *ScreenToggle = new FrogPilotParamManageControl(param, title, desc, icon, this);
      QObject::connect(ScreenToggle, &FrogPilotParamManageControl::manageButtonClicked, this, [this]() {
        parentToggleClicked();
        for (auto &[key, toggle] : toggles) {
          toggle->setVisible(ScreenKeys.find(key.c_str()) != ScreenKeys.end());
        }
      });
      toggle = ScreenToggle;    
    } else if (param == "AutoOffScreentime") {
      toggle = new FrogPilotParamValueControl(param, title, desc, icon, 0, 60, std::map<int, QString>(), this, false, "秒");

    } else if (param == "VagSpeed") {
      FrogPilotParamManageControl *VagSpeedToggle = new FrogPilotParamManageControl(param, title, desc, icon, this);
      QObject::connect(VagSpeedToggle, &FrogPilotParamManageControl::manageButtonClicked, this, [this]() {
        parentToggleClicked();
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
        parentToggleClicked();
        for (auto &[key, toggle] : toggles) {
          toggle->setVisible(AutoACCKeys.find(key.c_str()) != AutoACCKeys.end());
        }
      });
      toggle = AutoACCToggle;      
    } else if (param == "AutoACCspeed") {
      toggle = new FrogPilotParamValueControl(param, title, desc, icon, 1, 50, std::map<int, QString>(), this, false, "公里");

    } else if (param == "CarAway") {
      FrogPilotParamManageControl *CarAwayToggle = new FrogPilotParamManageControl(param, title, desc, icon, this);
      QObject::connect(CarAwayToggle, &FrogPilotParamManageControl::manageButtonClicked, this, [this]() {
        parentToggleClicked();
        for (auto &[key, toggle] : toggles) {
          toggle->setVisible(CarAwayKeys.find(key.c_str()) != CarAwayKeys.end());
        }
      });
      toggle = CarAwayToggle;    

    } else if (param == "CarAwayspeed") {
      toggle = new FrogPilotParamValueControl(param, title, desc, icon, 1, 10, std::map<int, QString>(), this, false, "公里");

    } else if (param == "CarAwaydistance") {
      toggle = new FrogPilotParamValueControl(param, title, desc, icon, 1, 10, std::map<int, QString>(), this, false, "公尺");

    } else if (param == "Roadtype") {
      FrogPilotParamManageControl *RoadToggle = new FrogPilotParamManageControl(param, title, desc, icon, this);
      QObject::connect(RoadToggle, &FrogPilotParamManageControl::manageButtonClicked, this, [this]() {
        parentToggleClicked();
        for (auto &[key, toggle] : toggles) {
          toggle->setVisible(RoadKeys.find(key.c_str()) != RoadKeys.end());
        }
      });
      toggle = RoadToggle;
    } else if (param == "RoadtypeProfile") {
      // std::map<int, QString> RoadtypeProfile{{"關閉"}, {"平面"}, {"快速"}, {"高速"}};
      // toggle = new FrogPilotParamValueControl(param, title, desc, icon, 0, 3, RoadtypeProfile, this, true);
      
      std::vector<QString> RoadtypeOptions{{"關閉"}, {"平面"}, {"快速"}, {"高速"}};
      FrogPilotButtonParamControl *preferredRoadtype = new FrogPilotButtonParamControl(param, title, desc, icon, RoadtypeOptions);
      toggle = preferredRoadtype;

    } else if (param == "Navspeed") {
      FrogPilotParamManageControl *NavspeedToggle = new FrogPilotParamManageControl(param, title, desc, icon, this);
      QObject::connect(NavspeedToggle, &FrogPilotParamManageControl::manageButtonClicked, this, [this]() {
        parentToggleClicked();
        for (auto &[key, toggle] : toggles) {
          toggle->setVisible(NavspeedKeys.find(key.c_str()) != NavspeedKeys.end());
        }
      });
      toggle = NavspeedToggle;
    } else if (param == "Voicereminder") {
      FrogPilotParamManageControl *VoiceToggle = new FrogPilotParamManageControl(param, title, desc, icon, this);
      QObject::connect(VoiceToggle, &FrogPilotParamManageControl::manageButtonClicked, this, [this]() {
        parentToggleClicked();
        for (auto &[key, toggle] : toggles) {
          toggle->setVisible(VoiceKeys.find(key.c_str()) != VoiceKeys.end());
        }
      });
      toggle = VoiceToggle;
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

  ScreenKeys = {"OffScreen", "AutoOffScreen", "AutoOffScreentime"};
  VagSpeedKeys = {"VagSpeedFactor"};  
  AutoACCKeys = {"AutoACCspeed", "AutoACCCarAway", "AutoACCGreenLight"};
  CarAwayKeys = {"CarAwayspeed", "CarAwaydistance"};
  RoadKeys = {"RoadtypeProfile"};
  NavspeedKeys = {"NavReminder", "speedoverreminder", "SpeedlimituReminder", "speedreminderreset"};
  VoiceKeys = {"GreenLightReminder", "ChangeLaneReminder","Laneblindspotdetection","CarApproachingReminder","CarAwayReminder"};


  // QObject::connect(uiState(), &UIState::offroadTransition, this, [this](bool offroad) {
  //   if (!offroad) {
  //     updateCarToggles();
  //   }
  // });

  QObject::connect(device(), &Device::interactiveTimeout, this, &HFOPControlsPanel::hideSubToggles);
  QObject::connect(parent, &SettingsWindow::closeParentToggle, this, &HFOPControlsPanel::hideSubToggles);

  hideSubToggles();
}

void HFOPControlsPanel::updateToggles() {
  std::thread([this]() {
    paramsMemory.putBool("FrogPilotTogglesUpdated", true);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    paramsMemory.putBool("FrogPilotTogglesUpdated", false);
  }).detach();
}

void HFOPControlsPanel::parentToggleClicked() {
  this->openParentToggle();
}

void HFOPControlsPanel::hideSubToggles() {

  for (auto &[key, toggle] : toggles) {
    bool subToggles = ScreenKeys.find(key.c_str()) != ScreenKeys.end() ||
                      VagSpeedKeys.find(key.c_str()) != VagSpeedKeys.end() ||
                      AutoACCKeys.find(key.c_str()) != AutoACCKeys.end() ||
                      CarAwayKeys.find(key.c_str()) != CarAwayKeys.end() ||
                      RoadKeys.find(key.c_str()) != RoadKeys.end() ||
                      NavspeedKeys.find(key.c_str()) != NavspeedKeys.end() ||
                      VoiceKeys.find(key.c_str()) != VoiceKeys.end() ;
    toggle->setVisible(!subToggles);
  }

  this->closeParentToggle();
}

void HFOPControlsPanel::hideEvent(QHideEvent *event) {
  hideSubToggles();
}

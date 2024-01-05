#include <cmath>
#include <filesystem>
#include <unordered_set>

#include "selfdrive/frogpilot/ui/hfop_settings.h"
#include "selfdrive/ui/ui.h"

HFOPControlsPanel::HFOPControlsPanel(SettingsWindow *parent) : ListWidget(parent) {
  const std::vector<std::tuple<QString, QString, QString, QString>> hfopToggles {
    {"VagSpeed", "時速差調整", "VAG專用。調整車錶速度與C3定速設定不同步的問題。", "../assets/offroad/icon_openpilot.png"},
    {"VagSpeedFactor", "  時速差調整", "請輸入OP定速為110時儀表板的速度差值.", ""},
    {"Disablestartstop", "  取消怠速熄火", "開啟後將強制關閉怠速熄火功能.", ""},
    
    {"AutoACC", "自動啟動ACC", "啟用後會自動啟動ACC.", "../assets/offroad/icon_conditional.png"},
    {"AutoACCspeed", "  自動啟動ACC時速設定", "設定自動啟動ACC的時速條件設定.", ""},
    {"AutoACCCarAway", "  前車遠離啟動", "啟用後當前方車輛遠離會自動啟動ACC.", ""},
    {"AutoACCGreenLight", "  綠燈啟動", "啟用後當偵測到綠燈時會自動啟動ACC.", ""},

    {"CarAway", "前車遠離偵測", "啟用後當前車速度改變時會有提醒.", "../assets/offroad/icon_warning.png"},
    {"CarAwayspeed", "  前車速度差設定", "設定前車時速大於多少公里時提醒..", ""},
    {"CarAwaydistance", "  前車距離差設定", "設定前車距離大於多少公尺時提醒..", ""},
    
    {"Roadtype", "道路種類設定", "開啟後可依道路種類在特定條件下預設時速", "../assets/offroad/icon_road.png"},
    
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
  };

  for (const auto &[param, title, desc, icon] : hfopToggles) {
    ParamControl *toggle;

    if (param == "VagSpeed") {
      ParamManageControl *VagSpeedToggle = new ParamManageControl(param, title, desc, icon, this);
      QObject::connect(VagSpeedToggle, &ParamManageControl::manageButtonClicked, this, [this]() {
        parentToggleClicked();
        for (auto &[key, toggle] : toggles) {
          toggle->setVisible(VagSpeedKeys.find(key.c_str()) != VagSpeedKeys.end());
        }
      });
      toggle = VagSpeedToggle;      
    } else if (param == "VagSpeedFactor") {
      toggle = new ParamValueControl(param, title, desc, icon, 0, 20, std::map<int, QString>(), this, false, "公里", 2);

    } else if (param == "AutoACC") {
      ParamManageControl *AutoACCToggle = new ParamManageControl(param, title, desc, icon, this);
      QObject::connect(AutoACCToggle, &ParamManageControl::manageButtonClicked, this, [this]() {
        parentToggleClicked();
        for (auto &[key, toggle] : toggles) {
          toggle->setVisible(AutoACCKeys.find(key.c_str()) != AutoACCKeys.end());
        }
      });
      toggle = AutoACCToggle;      
    } else if (param == "AutoACCspeed") {
      toggle = new ParamValueControl(param, title, desc, icon, 1, 50, std::map<int, QString>(), this, false, "公里");

    } else if (param == "CarAway") {
      ParamManageControl *CarAwayToggle = new ParamManageControl(param, title, desc, icon, this);
      QObject::connect(CarAwayToggle, &ParamManageControl::manageButtonClicked, this, [this]() {
        parentToggleClicked();
        for (auto &[key, toggle] : toggles) {
          toggle->setVisible(CarAwayKeys.find(key.c_str()) != CarAwayKeys.end());
        }
      });
      toggle = CarAwayToggle;    

    } else if (param == "CarAwayspeed") {
      toggle = new ParamValueControl(param, title, desc, icon, 1, 10, std::map<int, QString>(), this, false, "公里");

    } else if (param == "CarAwaydistance") {
      toggle = new ParamValueControl(param, title, desc, icon, 1, 10, std::map<int, QString>(), this, false, "公尺");

    } else if (param == "Roadtype") {
      std::map<int, QString> themeLabels = {{0, "關閉"}, {1, "平面"}, {2, "快速"}, {3, "高速"}};
      toggle = new ParamValueControl(param, title, desc, icon, 0, 3, themeLabels, this, true);

    } else if (param == "Navspeed") {
      ParamManageControl *NavspeedToggle = new ParamManageControl(param, title, desc, icon, this);
      QObject::connect(NavspeedToggle, &ParamManageControl::manageButtonClicked, this, [this]() {
        parentToggleClicked();
        for (auto &[key, toggle] : toggles) {
          toggle->setVisible(NavspeedKeys.find(key.c_str()) != NavspeedKeys.end());
        }
      });
      toggle = NavspeedToggle;
    } else if (param == "Voicereminder") {
      ParamManageControl *VoiceToggle = new ParamManageControl(param, title, desc, icon, this);
      QObject::connect(VoiceToggle, &ParamManageControl::manageButtonClicked, this, [this]() {
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
      paramsMemory.putBool("FrogPilotTogglesUpdated", true);
    });

    QObject::connect(dynamic_cast<ParamValueControl*>(toggle), &ParamValueControl::buttonPressed, [this]() {
      paramsMemory.putBool("FrogPilotTogglesUpdated", true);
    });
  }

  VagSpeedKeys = {"VagSpeedFactor"};  
  AutoACCKeys = {"AutoACCspeed", "AutoACCCarAway", "AutoACCGreenLight"};
  CarAwayKeys = {"CarAwayspeed", "CarAwaydistance"};
  NavspeedKeys = {"NavReminder", "speedoverreminder", "SpeedlimituReminder", "speedreminderreset"};
  VoiceKeys = {"GreenLightReminder", "ChangeLaneReminder","Laneblindspotdetection","CarApproachingReminder","CarAwayReminder"};

  QObject::connect(uiState(), &UIState::uiUpdate, this, &HFOPControlsPanel::updateState);

  hideSubToggles();
  setDefaults();
}

void HFOPControlsPanel::updateState() {
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

    previousIsMetric = isMetric;
  }).detach();
}
void HFOPControlsPanel::parentToggleClicked() {
  paramsMemory.putInt("FrogPilotTogglesOpen", 1);
}

void HFOPControlsPanel::hideSubToggles() {
  for (auto &[key, toggle] : toggles) {
    const bool subToggles = VagSpeedKeys.find(key.c_str()) != VagSpeedKeys.end() ||
                            AutoACCKeys.find(key.c_str()) != AutoACCKeys.end() ||
                            CarAwayKeys.find(key.c_str()) != CarAwayKeys.end() ||
                            NavspeedKeys.find(key.c_str()) != NavspeedKeys.end() ||
                            VoiceKeys.find(key.c_str()) != VoiceKeys.end();
    toggle->setVisible(!subToggles);
  }
}

void HFOPControlsPanel::hideEvent(QHideEvent *event) {
  hideSubToggles();
}

void HFOPControlsPanel::setDefaults() {
  // const bool FrogsGoMoo = params.get("DongleId").substr(0, 3) == "be6";

  const std::map<std::string, std::string> defaultValues {
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
    {"Disablestartstop", "1"},
    {"GreenLightReminder", "1"},    
    {"Laneblindspotdetection", "1"},
    {"Navspeed", "1"},
    {"NavReminder", "1"},
    {"Emergencycontrol", "1"},
    {"Roadtype", "1"},
    {"RoadtypeProfile", "1"},
    {"speedoverreminder", "1"},    
    {"speedreminderreset", "1"},
    {"Speeddistance", "1"},
    {"VagSpeed", "1"},
    {"VagSpeedFactor", "13"},
    {"Voicereminder", "1"},
    {"CarAwayReminderstatus", "0"},
    {"speedoverreminderstatus", "0"},
    {"NavReminderstatus", "0"},
    {"GreenLightReminderstatus", "0"},
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

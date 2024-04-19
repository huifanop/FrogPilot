#pragma once

#include <set>
// #include <QStringList>
#include "selfdrive/frogpilot/ui/frogpilot_ui_functions.h"
#include "selfdrive/ui/qt/offroad/settings.h"
#include "selfdrive/ui/ui.h"

class HFOPControlsPanel : public FrogPilotListWidget {
  Q_OBJECT

public:
  explicit HFOPControlsPanel(SettingsWindow *parent);

signals:
  void closeParentToggle();
  void closeSubParentToggle();
  void openParentToggle();
  void openSubParentToggle();

private:
  void hideEvent(QHideEvent *event);
  void hideSubToggles();
  void hideSubSubToggles();
  void parentToggleClicked();
  void subParentToggleClicked();
  void updateState(const UIState &s);
  void updateToggles();

  std::set<QString> FuelpriceKeys = {"Fuelcosts"};
  // std::set<QString> ScreenKeys = {"GooffScreen", "AutoOffScreen", "AutoOffScreentime"};
  std::set<QString> VagSpeedKeys = {"VagSpeedFactor"};  
  std::set<QString> AutoACCKeys = {"AutoACCspeed", "AutoACCCarAway", "AutoACCGreenLight", "TrafficModespeed"};
  // std::set<QString> CarAwayKeys = {"CarAwayspeed", "CarAwaydistance"};  
  std::set<QString> RoadKeys = {"RoadtypeProfile"};
  std::set<QString> NavspeedKeys = {"NavReminder", "speedoverreminder", "SpeedlimituReminder", "speedreminderreset"};
  std::set<QString> VoiceKeys = {"ChangeLaneReminder","Laneblindspotdetection","CarApproachingReminder"};
  std::set<QString> DooropenKeys= {"DriverdoorOpen", "CodriverdoorOpen","LpassengerdoorOpen","RpassengerdoorOpen","LuggagedoorOpen"};

  std::map<std::string, ParamControl*> toggles;

  Params params;
  Params paramsMemory{"/dev/shm/params"};
  bool started = false;
};

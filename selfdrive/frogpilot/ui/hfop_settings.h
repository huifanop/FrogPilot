#pragma once

#include <set>
#include <QStringList>
#include "selfdrive/frogpilot/ui/frogpilot_ui_functions.h"
#include "selfdrive/ui/qt/offroad/settings.h"
#include "selfdrive/ui/ui.h"

class HFOPControlsPanel : public FrogPilotListWidget {
  Q_OBJECT

public:
  explicit HFOPControlsPanel(SettingsWindow *parent);

signals:
  void closeParentToggle();
  void openParentToggle();

private:
  void hideEvent(QHideEvent *event);
  void hideSubToggles();
  void parentToggleClicked();
  // void updateCarToggles();
  void updateToggles();

  std::set<QString> ScreenKeys = {"OffScreen", "AutoOffScreen", "AutoOffScreentime"};
  std::set<QString> VagSpeedKeys = {"VagSpeedFactor"};  
  std::set<QString> AutoACCKeys = {"AutoACCspeed", "AutoACCCarAway", "AutoACCGreenLight"};
  std::set<QString> CarAwayKeys = {"CarAwayspeed", "CarAwaydistance"};  
  std::set<QString> RoadKeys = {"RoadtypeProfile"};
  std::set<QString> NavspeedKeys = {"NavReminder", "speedoverreminder", "SpeedlimituReminder", "speedreminderreset"};
  std::set<QString> VoiceKeys = {"GreenLightReminder", "ChangeLaneReminder","Laneblindspotdetection","CarApproachingReminder","CarAwayReminder"};
  std::set<QString> DooropenKeys= {"DriverdoorOpen", "CodriverdoorOpen","LpassengerdoorOpen","RpassengerdoorOpen","LuggagedoorOpen"};

  std::map<std::string, ParamControl*> toggles;

  Params params;
  Params paramsMemory{"/dev/shm/params"};

};

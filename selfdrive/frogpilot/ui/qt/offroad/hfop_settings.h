#pragma once

#include <set>

#include "selfdrive/frogpilot/ui/qt/offroad/frogpilot_settings.h"

class FrogPilotHFOPPanel : public FrogPilotListWidget {
  Q_OBJECT

public:
  explicit FrogPilotHFOPPanel(FrogPilotSettingsWindow *parent);

signals:
  void openParentToggle();

private:
  // FrogPilotSettingsWindow *parent;

  std::set<QString> FuelpriceKeys = {"Fuelcosts"};
  std::set<QString> TrafficModeKeys = {"TrafficModespeed"};
  std::set<QString> VagSpeedKeys = {"VagSpeedFactor"};
  std::set<QString> AutoACCKeys = {"AutoACCspeed", "AutoACCCarAway", "AutoACCGreenLight"};
  std::set<QString> RoadKeys = {"AutoRoadtype","RoadtypeProfile"};
  std::set<QString> NavspeedKeys = {"NavReminder", "speedoverreminder", "speedreminderreset"};
  std::set<QString> DooropenKeys= {"DriverdoorOpen", "CodriverdoorOpen","LpassengerdoorOpen","RpassengerdoorOpen","LuggagedoorOpen"};

  std::map<QString, AbstractControl*> toggles;

  Params params;
  Params paramsMemory{"/dev/shm/params"};
  bool started ;

  void hideToggles();
  void showToggles(const std::set<QString> &keys);
  void updateState(const UIState &s);

};

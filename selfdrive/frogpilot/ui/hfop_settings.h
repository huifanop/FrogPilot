#pragma once

#include <set>

#include "selfdrive/ui/qt/offroad/settings.h"

class HFOPControlsPanel : public ListWidget {
  Q_OBJECT

public:
  explicit HFOPControlsPanel(SettingsWindow *parent);

private:
  void hideEvent(QHideEvent *event);
  void hideSubToggles();
  void parentToggleClicked();
  void setDefaults();
  void updateState();

  std::set<QString> VagSpeedKeys;
  std::set<QString> AutoACCKeys;
  std::set<QString> CarAwayKeys;  
  std::set<QString> NavspeedKeys;
  std::set<QString> VoiceKeys;

  std::map<std::string, ParamControl*> toggles;

  Params params;
  Params paramsMemory{"/dev/shm/params"};

  bool isMetric = params.getBool("IsMetric");
};

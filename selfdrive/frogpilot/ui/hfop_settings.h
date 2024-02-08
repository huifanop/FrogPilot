#pragma once

#include <set>
#include <QStringList>
#include "selfdrive/frogpilot/ui/frogpilot_ui_functions.h"
#include "selfdrive/ui/qt/offroad/settings.h"

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

  std::set<QString> ScreenKeys;
  std::set<QString> VagSpeedKeys;
  std::set<QString> AutoACCKeys;
  std::set<QString> CarAwayKeys;  
  std::set<QString> RoadKeys;
  std::set<QString> NavspeedKeys;
  std::set<QString> VoiceKeys;

  std::map<std::string, ParamControl*> toggles;

  Params params;
  Params paramsMemory{"/dev/shm/params"};

};

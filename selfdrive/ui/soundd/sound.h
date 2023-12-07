#pragma once

#include <tuple>

#include <QMap>
#include <QSoundEffect>
#include <QString>

#include "system/hardware/hw.h"
#include "selfdrive/ui/ui.h"


const float MAX_VOLUME = 1.0;

const std::tuple<AudibleAlert, QString, int, float> sound_list[] = {
  // AudibleAlert, file name, loop count
  {AudibleAlert::ENGAGE, "engage.wav", 0, MAX_VOLUME},
  {AudibleAlert::DISENGAGE, "disengage.wav", 0, MAX_VOLUME},
  {AudibleAlert::REFUSE, "refuse.wav", 0, MAX_VOLUME},

  {AudibleAlert::PROMPT, "prompt.wav", 0, MAX_VOLUME},
  {AudibleAlert::PROMPT_REPEAT, "prompt.wav", QSoundEffect::Infinite, MAX_VOLUME},
  {AudibleAlert::PROMPT_DISTRACTED, "prompt_distracted.wav", QSoundEffect::Infinite, MAX_VOLUME},
  ///////////////////////////////////////////////////
  {AudibleAlert::CAR_AWAYED, "carawayed.wav", QSoundEffect::Infinite, MAX_VOLUME},
  {AudibleAlert::GREEN_LIGHT, "greenlighted.wav", QSoundEffect::Infinite, MAX_VOLUME},
  {AudibleAlert::LANECHANGE_BLOCKEDSOUND, "lanechangeBlockedsound.wav", QSoundEffect::Infinite, MAX_VOLUME},
  {AudibleAlert::LANECHANGE_SOUND, "lanechange.wav", 0, MAX_VOLUME},
  {AudibleAlert::CAR_APPROACHING, "carapproaching.wav", 0, MAX_VOLUME},
  {AudibleAlert::DETECT_SPEEDU, "dspeedu.wav", 0, MAX_VOLUME},
  {AudibleAlert::DETECT_SPEEDD, "dspeedd.wav", 0, MAX_VOLUME},
  ////////////NAV語音////////////////////////
  {AudibleAlert::NAV_TURN, "navTurn.wav", 0, MAX_VOLUME},
  {AudibleAlert::NAVTURN_LEFT, "navturnLeft.wav", 0, MAX_VOLUME},
  {AudibleAlert::NAVTURN_RIGHT, "navturnRight.wav", 0, MAX_VOLUME},
  {AudibleAlert::NAV_UTURN, "navUturn.wav", 0, MAX_VOLUME},
  {AudibleAlert::NAV_OFFRAMP, "navOfframp.wav", 0, MAX_VOLUME},
  {AudibleAlert::NAV_OFFRAMP, "navSharpright.wav", 0, MAX_VOLUME},
  {AudibleAlert::NAV_OFFRAMP, "navSharpleft.wav", 0, MAX_VOLUME},
  ///////////////////////////////////////////////////
  {AudibleAlert::WARNING_SOFT, "warning_soft.wav", QSoundEffect::Infinite, MAX_VOLUME},
  {AudibleAlert::WARNING_IMMEDIATE, "warning_immediate.wav", QSoundEffect::Infinite, MAX_VOLUME},
};

class Sound : public QObject {
public:
  explicit Sound(QObject *parent = 0);

public slots:
  // FrogPilot slots
  void updateFrogPilotParams();

protected:
  void update();
  void setAlert(const Alert &alert);

  SubMaster sm;
  Alert current_alert = {};
  QMap<AudibleAlert, QPair<QSoundEffect *, int>> sounds;
  int current_volume = -1;

  // FrogPilot variables
  Params params;
  bool isCustomTheme;
  bool isSilentMode;
  int customSounds;
  std::unordered_map<int, QString> soundPaths;
};

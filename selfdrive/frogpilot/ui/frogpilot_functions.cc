#include <filesystem>

#include "selfdrive/frogpilot/ui/frogpilot_functions.h"
#include "selfdrive/ui/ui.h"

bool FrogPilotConfirmationDialog::toggle(const QString &prompt_text, const QString &confirm_text, QWidget *parent) {
  ConfirmationDialog d = ConfirmationDialog(prompt_text, confirm_text, tr("Reboot Later"), false, parent);
  return d.exec();
}

bool FrogPilotConfirmationDialog::toggleAlert(const QString &prompt_text, const QString &button_text, QWidget *parent) {
  ConfirmationDialog d = ConfirmationDialog(prompt_text, button_text, "", false, parent);
  return d.exec();
}

bool FrogPilotConfirmationDialog::yesorno(const QString &prompt_text, QWidget *parent) {
  ConfirmationDialog d = ConfirmationDialog(prompt_text, tr("Yes"), tr("No"), false, parent);
  return d.exec();
}

FrogPilotButtonIconControl::FrogPilotButtonIconControl(const QString &title, const QString &text, const QString &desc, const QString &icon, QWidget *parent) : AbstractControl(title, desc, icon, parent) {
  btn.setText(text);
  btn.setStyleSheet(R"(
    QPushButton {
      padding: 0;
      border-radius: 50px;
      font-size: 35px;
      font-weight: 500;
      color: #E4E4E4;
      background-color: #393939;
    }
    QPushButton:pressed {
      background-color: #4a4a4a;
    }
    QPushButton:disabled {
      color: #33E4E4E4;
    }
  )");
  btn.setFixedSize(250, 100);
  QObject::connect(&btn, &QPushButton::clicked, this, &FrogPilotButtonIconControl::clicked);
  hlayout->addWidget(&btn);
}

void setDefaultParams() {
  Params params = Params();
  bool FrogsGoMoo = params.get("DongleId").substr(0, 3) == "be6";

  std::map<std::string, std::string> defaultValues {
//////////////////////////////////////////////////////
    {"Screen", "0"},
    {"OffScreen", "0"},
    {"AutoOffScreen", "0"},
    {"AutoOffScreenpre", "0"},
    {"AutoOffScreentime", "5"},
    // {"Autowifi", "1"},
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
//////////////////////////////////////////////////////
    {"AccelerationPath", FrogsGoMoo ? "1" : "1"},
    {"AccelerationProfile", FrogsGoMoo ? "3" : "1"},
    {"AdjacentPath", FrogsGoMoo ? "1" : "1"},
    {"AdjustablePersonalities", "3"},
    {"AggressiveAcceleration", "1"},
    {"AggressiveFollow", FrogsGoMoo ? "10" : "12"},
    {"AggressiveJerk", FrogsGoMoo ? "6" : "5"},
    {"AlwaysOnLateral", "1"},
    {"AlwaysOnLateralMain", FrogsGoMoo ? "1" : "1"},
    {"AverageCurvature", FrogsGoMoo ? "1" : "1"},
    {"BlindSpotPath", "1"},
    {"CameraView", FrogsGoMoo ? "1" : "2"},
    {"CECurves", "1"},
    {"CECurvesLead", "1"},
    {"CENavigation", "1"},
    {"CESignal", "1"},
    {"CESlowerLead", "1"},
    {"CESpeed", "0"},
    {"CESpeedLead", "0"},
    {"CEStopLights", "1"},
    {"CEStopLightsLead", FrogsGoMoo ? "0" : "1"},
    {"Compass", FrogsGoMoo ? "1" : "0"},
    {"ConditionalExperimental", "1"},
    {"CurveSensitivity", FrogsGoMoo ? "125" : "125"},
    {"CustomColors", "0"},
    {"CustomIcons", "0"},
    {"CustomPersonalities", "0"},
    {"CustomSignals", "0"},
    {"CustomSounds", "0"},
    {"CustomTheme", "0"},
    {"CustomUI", "1"},
    {"DeviceShutdown", "2"},
    {"DriverCamera", "0"},
    {"DriveStats", "1"},
    {"EVTable", FrogsGoMoo ? "0" : "1"},
    {"ExperimentalModeActivation", "1"},
    {"ExperimentalModeViaLKAS", "1"},
    {"ExperimentalModeViaScreen", FrogsGoMoo ? "0" : "1"},
    {"Fahrenheit", "0"},
    {"FireTheBabysitter", FrogsGoMoo ? "1" : "1"},
    {"FullMap", "0"},
    {"GasRegenCmd", "0"},
    {"GoatScream", "1"},
    {"GreenLightAlert", "1"},
    {"HideSpeed", "0"},
    {"HigherBitrate", "0"},
    {"LaneChangeTime", "1"},
    {"LaneDetection", "1"},
    {"LaneLinesWidth", "4"},
    {"LateralTune", "1"},
    {"LeadInfo", FrogsGoMoo ? "1" : "1"},
    {"LockDoors", "0"},
    {"LongitudinalTune", "1"},
    {"LongPitch", FrogsGoMoo ? "0" : "1"},
    {"LowerVolt", FrogsGoMoo ? "0" : "1"},
    {"Model", "0"},
    {"ModelUI", "1"},
    {"MTSCEnabled", "1"},
    {"MuteDM", FrogsGoMoo ? "1" : "1"},
    {"MuteDoor", FrogsGoMoo ? "1" : "1"},
    {"MuteOverheated", FrogsGoMoo ? "1" : "1"},
    {"MuteSeatbelt", FrogsGoMoo ? "1" : "1"},
    {"NNFF", FrogsGoMoo ? "1" : "1"},
    {"NoLogging", "0"},
    {"NudgelessLaneChange", "1"},
    {"NumericalTemp", FrogsGoMoo ? "1" : "1"},
    {"Offset1", "0"},
    {"Offset2", FrogsGoMoo ? "7" : "0"},
    {"Offset3", "0"},
    {"Offset4", FrogsGoMoo ? "20" : "0"},
    {"OneLaneChange", "1"},
    {"PathEdgeWidth", "20"},
    {"PathWidth", "20"},
    {"PauseLateralOnSignal", "0"},
    {"PreferredSchedule", "0"},
    {"QOLControls", "1"},
    {"QOLVisuals", "1"},
    {"RandomEvents", FrogsGoMoo ? "1" : "0"},
    {"RelaxedFollow", "30"},
    {"RelaxedJerk", "50"},
    {"ReverseCruise", "0"},
    {"RoadEdgesWidth", "2"},
    {"RoadNameUI", "1"},
    {"RotatingWheel", "0"},
    {"ScreenBrightness", "101"},
    {"SearchInput", "0"},
    {"ShowCPU", FrogsGoMoo ? "1" : "0"},
    {"ShowFPS", FrogsGoMoo ? "1" : "0"},
    {"ShowGPU", "0"},
    {"ShowMemoryUsage", FrogsGoMoo ? "1" : "0"},
    {"Sidebar", FrogsGoMoo ? "1" : "0"},
    {"SilentMode", "0"},
    {"SLCFallback", "0"},
    {"SLCOverride", FrogsGoMoo ? "2" : "1"},
    {"SLCPriority", "1"},
    {"SmoothBraking", "1"},
    {"SNGHack", FrogsGoMoo ? "0" : "1"},
    {"SpeedLimitController", "1"},
    {"StandardFollow", "15"},
    {"StandardJerk", "10"},
    {"StoppingDistance", FrogsGoMoo ? "6" : "1"},
    {"TSS2Tune", "1"},
    {"TurnAggressiveness", FrogsGoMoo ? "150" : "150"},
    {"TurnDesires", "1"},
    {"UnlimitedLength", "1"},
    {"UseSI", FrogsGoMoo ? "1" : "0"},
    {"UseVienna", "0"},
    {"VisionTurnControl", "1"},
    {"WheelIcon", FrogsGoMoo ? "1" : "0"}
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

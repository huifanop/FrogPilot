#include <filesystem>
#include <iostream>

#include "selfdrive/ui/qt/widgets/scrollview.h"

#include "selfdrive/frogpilot/navigation/ui/maps_settings.h"
#include "selfdrive/frogpilot/navigation/ui/primeless_settings.h"
#include "selfdrive/frogpilot/ui/qt/offroad/advanced_driving_settings.h"
#include "selfdrive/frogpilot/ui/qt/offroad/advanced_visual_settings.h"
#include "selfdrive/frogpilot/ui/qt/offroad/data_settings.h"
#include "selfdrive/frogpilot/ui/qt/offroad/device_settings.h"
#include "selfdrive/frogpilot/ui/qt/offroad/frogpilot_settings.h"
#include "selfdrive/frogpilot/ui/qt/offroad/lateral_settings.h"
#include "selfdrive/frogpilot/ui/qt/offroad/longitudinal_settings.h"
#include "selfdrive/frogpilot/ui/qt/offroad/sounds_settings.h"
#include "selfdrive/frogpilot/ui/qt/offroad/theme_settings.h"
#include "selfdrive/frogpilot/ui/qt/offroad/utilities.h"
#include "selfdrive/frogpilot/ui/qt/offroad/vehicle_settings.h"
#include "selfdrive/frogpilot/ui/qt/offroad/visual_settings.h"
#include "selfdrive/frogpilot/ui/qt/offroad/hfop_settings.h"

bool checkNNFFLogFileExists(const std::string &carFingerprint) {
  const std::filesystem::path latModelsPath("../car/torque_data/lat_models");

  if (!std::filesystem::exists(latModelsPath)) {
    std::cerr << "Lat models directory does not exist." << std::endl;
    return false;
  }

  for (const std::filesystem::directory_entry &entry : std::filesystem::directory_iterator(latModelsPath)) {
    if (entry.path().filename().string().rfind(carFingerprint, 0) == 0) {
      std::cout << "NNFF supports fingerprint: " << entry.path().filename() << std::endl;
      return true;
    }
  }

  return false;
}

FrogPilotSettingsWindow::FrogPilotSettingsWindow(SettingsWindow *parent) : QFrame(parent) {
  mainLayout = new QStackedLayout(this);

  frogpilotSettingsWidget = new QWidget(this);
  QVBoxLayout *frogpilotSettingsLayout = new QVBoxLayout(frogpilotSettingsWidget);
  frogpilotSettingsLayout->setContentsMargins(50, 25, 50, 25);

  FrogPilotListWidget *list = new FrogPilotListWidget(frogpilotSettingsWidget);

  FrogPilotAdvancedDrivingPanel *frogpilotAdvancedDrivingPanel = new FrogPilotAdvancedDrivingPanel(this);
  QObject::connect(frogpilotAdvancedDrivingPanel, &FrogPilotAdvancedDrivingPanel::openParentToggle, this, &FrogPilotSettingsWindow::openParentToggle);
  QObject::connect(frogpilotAdvancedDrivingPanel, &FrogPilotAdvancedDrivingPanel::openSubParentToggle, this, &FrogPilotSettingsWindow::openSubParentToggle);
  QObject::connect(frogpilotAdvancedDrivingPanel, &FrogPilotAdvancedDrivingPanel::openSubSubParentToggle, this, &FrogPilotSettingsWindow::openSubSubParentToggle);

  FrogPilotAdvancedVisualsPanel *frogpilotAdvancedVisualsPanel = new FrogPilotAdvancedVisualsPanel(this);
  QObject::connect(frogpilotAdvancedVisualsPanel, &FrogPilotAdvancedVisualsPanel::openParentToggle, this, &FrogPilotSettingsWindow::openParentToggle);

  FrogPilotDevicePanel *frogpilotDevicePanel = new FrogPilotDevicePanel(this);
  QObject::connect(frogpilotDevicePanel, &FrogPilotDevicePanel::openParentToggle, this, &FrogPilotSettingsWindow::openParentToggle);

  FrogPilotLateralPanel *frogpilotLateralPanel = new FrogPilotLateralPanel(this);
  QObject::connect(frogpilotLateralPanel, &FrogPilotLateralPanel::openParentToggle, this, &FrogPilotSettingsWindow::openParentToggle);

  FrogPilotLongitudinalPanel *frogpilotLongitudinalPanel = new FrogPilotLongitudinalPanel(this);
  QObject::connect(frogpilotLongitudinalPanel, &FrogPilotLongitudinalPanel::openParentToggle, this, &FrogPilotSettingsWindow::openParentToggle);
  QObject::connect(frogpilotLongitudinalPanel, &FrogPilotLongitudinalPanel::openSubParentToggle, this, &FrogPilotSettingsWindow::openSubParentToggle);

  FrogPilotMapsPanel *frogpilotMapsPanel = new FrogPilotMapsPanel(this);
  QObject::connect(frogpilotMapsPanel, &FrogPilotMapsPanel::openMapSelection, this, &FrogPilotSettingsWindow::openMapSelection);

  FrogPilotPrimelessPanel *frogpilotPrimelessPanel = new FrogPilotPrimelessPanel(this);
  QObject::connect(frogpilotPrimelessPanel, &FrogPilotPrimelessPanel::openMapBoxInstructions, this, &FrogPilotSettingsWindow::openMapBoxInstructions);

  FrogPilotSoundsPanel *frogpilotSoundsPanel = new FrogPilotSoundsPanel(this);
  QObject::connect(frogpilotSoundsPanel, &FrogPilotSoundsPanel::openParentToggle, this, &FrogPilotSettingsWindow::openParentToggle);

  FrogPilotThemesPanel *frogpilotThemesPanel = new FrogPilotThemesPanel(this);
  QObject::connect(frogpilotThemesPanel, &FrogPilotThemesPanel::openParentToggle, this, &FrogPilotSettingsWindow::openParentToggle);

  FrogPilotVisualsPanel *frogpilotVisualsPanel = new FrogPilotVisualsPanel(this);
  QObject::connect(frogpilotVisualsPanel, &FrogPilotVisualsPanel::openParentToggle, this, &FrogPilotSettingsWindow::openParentToggle);

/////////////////////////////////////////////////////
  FrogPilotHFOPPanel *frogpilotHFOPPanel = new FrogPilotHFOPPanel(this);
  QObject::connect(frogpilotHFOPPanel, &FrogPilotHFOPPanel::openParentToggle, this, &FrogPilotSettingsWindow::openParentToggle);
/////////////////////////////////////////////////////

  std::vector<std::pair<QString, std::vector<QWidget*>>> panels = {
    {tr("進階設定"), {frogpilotAdvancedDrivingPanel, frogpilotAdvancedVisualsPanel}},
    {tr("警報和聲音"), {frogpilotSoundsPanel}},
    {tr("駕駛控制"), {frogpilotLongitudinalPanel, frogpilotLateralPanel}},
    {tr("導航"), {frogpilotMapsPanel, frogpilotPrimelessPanel}},
    {tr("系統管理"), {new FrogPilotDataPanel(this), frogpilotDevicePanel, new UtilitiesPanel(this)}},
    {tr("主題與外觀"), {frogpilotVisualsPanel, frogpilotThemesPanel}},
    {tr("車輛控制"), {new FrogPilotVehiclesPanel(this)}},
    /////////////////////////////////////////////////////
    {tr("H F O P"), {frogpilotHFOPPanel}}
/////////////////////////////////////////////////////
  };

  std::vector<QString> icons = {
    "../frogpilot/assets/toggle_icons/icon_advanced.png",
    "../frogpilot/assets/toggle_icons/icon_sound.png",
    "../frogpilot/assets/toggle_icons/icon_steering.png",
    "../frogpilot/assets/toggle_icons/icon_map.png",
    "../frogpilot/assets/toggle_icons/icon_system.png",
    "../frogpilot/assets/toggle_icons/icon_display.png",
    "../frogpilot/assets/toggle_icons/icon_vehicle.png",
    "../frogpilot/assets/toggle_icons/icon_system.png",
  };

  std::vector<QString> descriptions = {
    tr("適合更有經驗的使用者的高級 FrogPilot 功能."),
    tr("自訂 FrogPilot 聲音警報和通知的選項."),
    tr("FrogPilot 的功能不僅僅是衝擊加速、煞車和轉向."),
    tr("離線地圖下載器和“Navigate On openpilot (NOO)”設置."),
    tr("用於維護和排除 FrogPilot 故障的工具和系統實用程式."),
    tr("用於自訂 FrogPilot 主題、UI 外觀和道路小部件的選項."),
    tr("支援的品牌和型號的車輛特定設定和配置."),
    tr("HFOP 的功能.")
  };

  std::vector<std::vector<QString>> buttonLabels = {
    {tr("個人化駕駛"), tr("介面")},
    {tr("管理")},
    {tr("油門/煞車"), tr("轉向")},
    {tr("管理")},
    {tr("數據"), tr("裝置"), tr("公用")},
    {tr("外觀"), tr("主題")},
    {tr("管理")},
    {tr("管理")}
  };

  for (size_t i = 0; i < panels.size(); ++i) {
    addPanelControl(list, panels[i].first, descriptions[i], buttonLabels[i], icons[i], panels[i].second, panels[i].first == tr("駕駛控制"), panels[i].first == tr("導航"));
  }

  frogpilotSettingsLayout->addWidget(new ScrollView(list, frogpilotSettingsWidget));
  frogpilotSettingsLayout->addStretch(1);

  mainLayout->addWidget(frogpilotSettingsWidget);
  mainLayout->setCurrentWidget(frogpilotSettingsWidget);

  QObject::connect(parent, &SettingsWindow::closeMapBoxInstructions, this, &FrogPilotSettingsWindow::closeMapBoxInstructions);
  QObject::connect(parent, &SettingsWindow::closeMapSelection, this, &FrogPilotSettingsWindow::closeMapSelection);
  QObject::connect(parent, &SettingsWindow::closePanel, this, &FrogPilotSettingsWindow::closePanel);
  QObject::connect(parent, &SettingsWindow::closeParentToggle, this, &FrogPilotSettingsWindow::closeParentToggle);
  QObject::connect(parent, &SettingsWindow::closeSubParentToggle, this, &FrogPilotSettingsWindow::closeSubParentToggle);
  QObject::connect(parent, &SettingsWindow::closeSubSubParentToggle, this, &FrogPilotSettingsWindow::closeSubSubParentToggle);
  QObject::connect(parent, &SettingsWindow::updateMetric, this, &FrogPilotSettingsWindow::updateMetric);
  QObject::connect(uiState(), &UIState::offroadTransition, this, &FrogPilotSettingsWindow::updateCarVariables);

  updateCarVariables();
}

void FrogPilotSettingsWindow::showEvent(QShowEvent *event) {
  updatePanelVisibility();
}

void FrogPilotSettingsWindow::closePanel() {
  QWidget *currentWidget = mainLayout->currentWidget();
  if (currentWidget != frogpilotSettingsWidget) {
    mainLayout->removeWidget(currentWidget);
  }
}

void FrogPilotSettingsWindow::updatePanelVisibility() {
  disableOpenpilotLongitudinal = params.getBool("DisableOpenpilotLongitudinal");

  drivingButton->setVisibleButton(0, hasOpenpilotLongitudinal && !disableOpenpilotLongitudinal);
  navigationButton->setVisibleButton(1, !uiState()->hasPrime());

  mainLayout->setCurrentWidget(frogpilotSettingsWidget);
}

void FrogPilotSettingsWindow::updateCarVariables() {
  float currentFrictionStock = params.getFloat("SteerFrictionStock");
  float currentKPStock = params.getFloat("SteerKPStock");
  float currentLatAccelStock = params.getFloat("SteerLatAccelStock");
  float currentRatioStock = params.getFloat("SteerRatioStock");

  std::string carParams = params.get("CarParamsPersistent");
  if (!carParams.empty()) {
    AlignedBuffer aligned_buf;
    capnp::FlatArrayMessageReader cmsg(aligned_buf.align(carParams.data(), carParams.size()));
    cereal::CarParams::Reader CP = cmsg.getRoot<cereal::CarParams>();
    cereal::CarParams::SafetyModel safetyModel = CP.getSafetyConfigs()[0].getSafetyModel();

    std::string carFingerprint = CP.getCarFingerprint();
    std::string carName = CP.getCarName();

    hasAutoTune = (carName == "hyundai" || carName == "toyota") && CP.getLateralTuning().which() == cereal::CarParams::LateralTuning::TORQUE;
    hasBSM = CP.getEnableBsm();
    hasDashSpeedLimits = carName == "hyundai" || carName == "toyota";
    hasExperimentalOpenpilotLongitudinal = CP.getExperimentalLongitudinalAvailable();
    hasNNFFLog = checkNNFFLogFileExists(carFingerprint);
    hasOpenpilotLongitudinal = hasLongitudinalControl(CP);
    hasPCMCruise = CP.getPcmCruise();
    hasSNG = CP.getMinEnableSpeed() <= 0;
    isGM = carName == "gm";
    isGMPCMCruise = CP.getCarName() == "gm" && CP.getPcmCruise();
    isHKGCanFd = carName == "hyundai" && safetyModel == cereal::CarParams::SafetyModel::HYUNDAI_CANFD;
    isImpreza = carFingerprint == "SUBARU_IMPREZA";
    isSubaru = carName == "subaru";
    isToyota = carName == "toyota";
    isToyotaTuneSupported = carFingerprint == "LEXUS_ES_TSS2";
    isVolt = carFingerprint == "CHEVROLET_VOLT";
    forcingAutoTune = params.getBool("LateralTune") && params.getBool("ForceAutoTune");
    steerFrictionStock = CP.getLateralTuning().getTorque().getFriction();
    steerKPStock = CP.getLateralTuning().getTorque().getKp();
    steerLatAccelStock = CP.getLateralTuning().getTorque().getLatAccelFactor();
    steerRatioStock = CP.getSteerRatio();

    if (currentFrictionStock != steerFrictionStock && steerFrictionStock != 0) {
      if (params.getFloat("SteerFriction") == currentFrictionStock) {
        params.putFloatNonBlocking("SteerFriction", steerFrictionStock);
      }
      params.putFloatNonBlocking("SteerFrictionStock", steerFrictionStock);
    }

    if (currentKPStock != steerKPStock && currentKPStock != 0) {
      if (params.getFloat("SteerKP") == currentKPStock) {
        params.putFloatNonBlocking("SteerKP", steerKPStock);
      }
      params.putFloatNonBlocking("SteerKPStock", steerKPStock);
    }

    if (currentLatAccelStock != steerLatAccelStock && steerLatAccelStock != 0) {
      if (params.getFloat("SteerLatAccel") == steerLatAccelStock) {
        params.putFloatNonBlocking("SteerLatAccel", steerLatAccelStock);
      }
      params.putFloatNonBlocking("SteerLatAccelStock", steerLatAccelStock);
    }

    if (currentRatioStock != steerRatioStock && steerRatioStock != 0) {
      if (params.getFloat("SteerRatio") == steerRatioStock) {
        params.putFloatNonBlocking("SteerRatio", steerRatioStock);
      }
      params.putFloatNonBlocking("SteerRatioStock", steerRatioStock);
    }

    uiState()->scene.has_auto_tune = hasAutoTune || forcingAutoTune;
  } else {
    hasAutoTune = true;
    hasBSM = true;
    hasDashSpeedLimits = true;
    hasExperimentalOpenpilotLongitudinal = false;
    hasNNFFLog = true;
    hasOpenpilotLongitudinal = true;
    hasPCMCruise = true;
    hasSNG = false;
    isGMPCMCruise = false;
    isHKGCanFd = true;
    isImpreza = true;
    isSubaru = false;
    isToyota = true;
    isVolt = true;
  }

  std::string liveTorqueParamsKey;
  if (params.getBool("ModelManagement")) {
    QString model = QString::fromStdString(params.get("ModelName"));
    QString part_model_param = processModelName(model);
    liveTorqueParamsKey = part_model_param.toStdString() + "LiveTorqueParameters";
  } else {
    liveTorqueParamsKey = "LiveTorqueParameters";
  }

  if (params.checkKey(liveTorqueParamsKey)) {
    auto torqueParams = params.get(liveTorqueParamsKey);
    if (!torqueParams.empty()) {
      AlignedBuffer aligned_buf;
      capnp::FlatArrayMessageReader cmsg(aligned_buf.align(torqueParams.data(), torqueParams.size()));
      cereal::Event::Reader LTP = cmsg.getRoot<cereal::Event>();

      auto liveTorqueParams = LTP.getLiveTorqueParameters();

      liveValid = liveTorqueParams.getLiveValid();
    } else {
      liveValid = false;
    }
  } else {
    liveValid = false;
  }

  emit updateCarToggles();
}

void FrogPilotSettingsWindow::addPanelControl(FrogPilotListWidget *list, QString &title, QString &desc, std::vector<QString> &button_labels, QString &icon, std::vector<QWidget*> &panels, bool isDrivingPanel, bool isNavigationPanel) {
  std::vector<QWidget*> panelContainers;
  panelContainers.reserve(panels.size());

  for (QWidget *panel : panels) {
    QWidget *panelContainer = new QWidget(this);
    QVBoxLayout *panelLayout = new QVBoxLayout(panelContainer);
    panelLayout->setContentsMargins(50, 25, 50, 25);
    panelLayout->addWidget(panel);
    panelContainers.push_back(panelContainer);
  }

  FrogPilotButtonsControl *button;
  if (isDrivingPanel) {
    drivingButton = new FrogPilotButtonsControl(title, desc, button_labels, false, true, icon);
    button = drivingButton;
  } else if (isNavigationPanel) {
    navigationButton = new FrogPilotButtonsControl(title, desc, button_labels, false, true, icon);
    button = navigationButton;
  } else {
    button = new FrogPilotButtonsControl(title, desc, button_labels, false, true, icon);
  }

  QObject::connect(button, &FrogPilotButtonsControl::buttonClicked, [this, panelContainers](int buttonId) {
    if (buttonId < panelContainers.size()) {
      QWidget *selectedPanel = panelContainers[buttonId];
      if (mainLayout->indexOf(selectedPanel) == -1) {
        mainLayout->addWidget(selectedPanel);
      }
      mainLayout->setCurrentWidget(selectedPanel);
    }
    openPanel();
  });

  list->addItem(button);
}

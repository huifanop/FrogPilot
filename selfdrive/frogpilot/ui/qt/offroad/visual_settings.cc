#include "selfdrive/frogpilot/ui/qt/offroad/visual_settings.h"

FrogPilotVisualsPanel::FrogPilotVisualsPanel(FrogPilotSettingsWindow *parent) : FrogPilotListWidget(parent), parent(parent) {
  const std::vector<std::tuple<QString, QString, QString, QString>> visualToggles {
    {"CustomUI", tr("自設道路畫面"), tr("定義自己喜歡的道路畫面."), "../assets/offroad/icon_road.png"},
    {"Compass", tr("  羅盤"), tr("將指南針加入道路畫面."), ""},
    {"DynamicPathWidth", tr("  動態路徑寬度"), tr("根據目前接合狀態自動調整行駛路徑顯示的寬度:\n\nFully engaged = 100%\nAlways On Lateral Active = 75%\nFully disengaged = 50%"), ""},
    {"PedalsOnUI", tr("  踏板"), tr("公路畫面中顯示踏板指示器可根據施加的壓力改變不透明度."), ""},
    {"CustomPaths", tr("  路徑"), tr("預計的加速路徑、偵測到的車道以及盲點中的車輛."), ""},
    {"RoadNameUI", tr("  道路名稱"), tr("將道路名稱顯示在螢幕底部'."), ""},
    {"RotatingWheel", tr("  旋轉方向盤"), tr("行駛畫面中的方向盤，會隨著方向盤的移動而旋轉."), ""},

    {"QOLVisuals", tr("進階設定"), tr("整合各種視覺功能可改善您的駕駛體驗."), "../frogpilot/assets/toggle_icons/quality_of_life.png"},
    {"BigMap", tr("全螢幕地圖顯示"), tr("行駛畫面中的導航地圖尺寸更大."), ""},
    {"MapStyle", tr("地圖樣式"), tr("導航地圖使用的地圖樣式."), ""},
    {"StandbyMode", tr("螢幕待機模式"), tr("行駛後螢幕會關閉，但如果狀態發生變化或發生重要警報，螢幕會自動喚醒."), ""},
    {"DriverCamera", tr("倒車時顯示駕駛員攝影機"), tr("車輛倒車時顯示駕駛攝影機畫面."), ""},
    {"StoppedTimer", tr("停止計時器"), tr("行駛畫面的計時器可顯示車輛停止了多長時間."), ""}
  };

  for (const auto &[param, title, desc, icon] : visualToggles) {
    AbstractControl *visualToggle;

    if (param == "CustomUI") {
      FrogPilotParamManageControl *customUIToggle = new FrogPilotParamManageControl(param, title, desc, icon);
      QObject::connect(customUIToggle, &FrogPilotParamManageControl::manageButtonClicked, [this]() {
        customPathsBtn->setVisibleButton(0, hasBSM);

        std::set<QString> modifiedCustomOnroadUIKeys = customOnroadUIKeys;

        showToggles(modifiedCustomOnroadUIKeys);
      });
      visualToggle = customUIToggle;
    } else if (param == "CustomPaths") {
      std::vector<QString> pathToggles{"AccelerationPath", "AdjacentPath", "BlindSpotPath"};
      std::vector<QString> pathToggleNames{tr("加速"), tr("左右車道"), tr("盲區")};
      customPathsBtn = new FrogPilotButtonToggleControl(param, title, desc, pathToggles, pathToggleNames);
      visualToggle = customPathsBtn;
    } else if (param == "PedalsOnUI") {
      std::vector<QString> pedalsToggles{"DynamicPedalsOnUI", "StaticPedalsOnUI"};
      std::vector<QString> pedalsToggleNames{tr("動態的"), tr("靜止的")};
      FrogPilotButtonToggleControl *pedalsToggle = new FrogPilotButtonToggleControl(param, title, desc, pedalsToggles, pedalsToggleNames, true);
      QObject::connect(pedalsToggle, &FrogPilotButtonToggleControl::buttonClicked, [this](int index) {
        if (index == 0) {
          params.putBool("StaticPedalsOnUI", false);
        } else if (index == 1) {
          params.putBool("DynamicPedalsOnUI", false);
        }
      });
      visualToggle = pedalsToggle;

    } else if (param == "QOLVisuals") {
      FrogPilotParamManageControl *qolToggle = new FrogPilotParamManageControl(param, title, desc, icon);
      QObject::connect(qolToggle, &FrogPilotParamManageControl::manageButtonClicked, [this]() {
        showToggles(qolKeys);
      });
      visualToggle = qolToggle;
    } else if (param == "BigMap") {
      std::vector<QString> mapToggles{"FullMap"};
      std::vector<QString> mapToggleNames{tr("全螢幕")};
      visualToggle = new FrogPilotButtonToggleControl(param, title, desc, mapToggles, mapToggleNames);
    } else if (param == "MapStyle") {
      QMap<int, QString> styleMap = {
        {0, tr("原始 openpilot")},
        {1, tr("Mapbox 街道")},
        {2, tr("Mapbox 戶外活動")},
        {3, tr("Mapbox 白天")},
        {4, tr("Mapbox 夜晚")},
        {5, tr("Mapbox 衛星")},
        {6, tr("Mapbox 衛星街道")},
        {7, tr("Mapbox 導航(白天)")},
        {8, tr("Mapbox 導航(夜晚)")},
        {9, tr("Mapbox 交通(夜晚)")},
        {10, tr("mike854's (衛星混合)")},
        {11, tr("huifan's (街道)")},
        {12, tr("huifan's 導航(白天)")},
        {13, tr("huifan's 導航(夜晚)")},
      };

      QStringList styles = styleMap.values();
      ButtonControl *mapStyleButton = new ButtonControl(title, tr("選擇"), desc);
      QObject::connect(mapStyleButton, &ButtonControl::clicked, [=]() {
        QStringList styles = styleMap.values();
        QString selection = MultiOptionDialog::getSelection(tr("選擇地圖樣式"), styles, "", this);
        if (!selection.isEmpty()) {
          int selectedStyle = styleMap.key(selection);
          params.putIntNonBlocking("MapStyle", selectedStyle);
          mapStyleButton->setValue(selection);
          updateFrogPilotToggles();
        }
      });

      int currentStyle = params.getInt("MapStyle");
      mapStyleButton->setValue(styleMap[currentStyle]);

      visualToggle = mapStyleButton;

    } else {
      visualToggle = new ParamControl(param, title, desc, icon);
    }

    addItem(visualToggle);
    toggles[param] = visualToggle;

    makeConnections(visualToggle);

    if (FrogPilotParamManageControl *frogPilotManageToggle = qobject_cast<FrogPilotParamManageControl*>(visualToggle)) {
      QObject::connect(frogPilotManageToggle, &FrogPilotParamManageControl::manageButtonClicked, this, &FrogPilotVisualsPanel::openParentToggle);
    }

    QObject::connect(visualToggle, &AbstractControl::showDescriptionEvent, [this]() {
      update();
    });
  }

  QObject::connect(parent, &FrogPilotSettingsWindow::closeParentToggle, this, &FrogPilotVisualsPanel::hideToggles);
  QObject::connect(parent, &FrogPilotSettingsWindow::updateCarToggles, this, &FrogPilotVisualsPanel::updateCarToggles);
}

void FrogPilotVisualsPanel::updateCarToggles() {
  hasBSM = parent->hasBSM;

  hideToggles();
}

void FrogPilotVisualsPanel::showToggles(const std::set<QString> &keys) {
  setUpdatesEnabled(false);

  for (auto &[key, toggle] : toggles) {
    toggle->setVisible(keys.find(key) != keys.end());
  }

  setUpdatesEnabled(true);
  update();
}

void FrogPilotVisualsPanel::hideToggles() {
  setUpdatesEnabled(false);

  for (auto &[key, toggle] : toggles) {
    bool subToggles = customOnroadUIKeys.find(key) != customOnroadUIKeys.end() ||
                      qolKeys.find(key) != qolKeys.end();

    toggle->setVisible(!subToggles);
  }

  setUpdatesEnabled(true);
  update();
}

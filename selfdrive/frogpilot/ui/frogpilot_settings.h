#pragma once

#include "common/params.h"
#include "selfdrive/ui/qt/widgets/controls.h"
#include "selfdrive/ui/qt/widgets/input.h"
#include "selfdrive/ui/ui.h"

static const QString buttonStyle = R"(
  QPushButton {
    border-radius: 50px;
    font-size: 40px;
    font-weight: 500;
    height: 100px;
    padding: 0 20 0 20;
    margin: 15px;
    color: #E4E4E4;
    background-color: #393939;
  }
  QPushButton:pressed {
    background-color: #4a4a4a;
  }
  QPushButton:checked:enabled {
    background-color: #33Ab4C;
  }
  QPushButton:disabled {
    color: #33E4E4E4;
  }
)";

class FrogPilotButtonParamControl : public QPushButton {
  Q_OBJECT

public:
  FrogPilotButtonParamControl(const QString &param, const QString &label, const int minimumButtonWidth = 225)
    : QPushButton(), key(param.toStdString()), params(), 
      value(params.getBool(key)) {
    setCheckable(true);
    setChecked(value);
    setStyleSheet(buttonStyle);
    setMinimumWidth(minimumButtonWidth);
    setText(label);

    QObject::connect(this, &QPushButton::toggled, this, [this](bool checked) {
      params.putBoolNonBlocking(key, checked);
      paramsMemory.putBoolNonBlocking("FrogPilotTogglesUpdated", true);
      if (key == "MuteDM" || key == "RoadNameUI") {
        if (ConfirmationDialog::toggle("Reboot required to take effect.", "Reboot Now", this)) {
          Hardware::reboot();
        }
      }
    });
  }

private:
  const std::string key;
  Params params;
  Params paramsMemory{"/dev/shm/params"};
  bool value;
};

class ParamValueControl : public AbstractControl {
protected:
  ParamValueControl(const QString &name, const QString &description, const QString &iconPath)
    : AbstractControl(name, description, iconPath) {
    label.setAlignment(Qt::AlignVCenter | Qt::AlignRight);
    label.setStyleSheet("color: #e0e879");
    label.setFixedWidth(170);

    setupButton(btnMinus, "-", -1);
    setupButton(btnPlus, "+", 1);

    hlayout->addWidget(&label);
    hlayout->addWidget(&btnMinus);
    hlayout->addWidget(&btnPlus);
  }

  void setupButton(QPushButton &btn, const QString &text, int delta) {
    btn.setStyleSheet(R"(
      QPushButton {
        background-color: #393939;
        color: #E4E4E4;
        border-radius: 50px;
        font-size: 50px;
        font-weight: 500;
        padding: 0;
      }
      QPushButton:pressed {
        background-color: #4a4a4a;
        color: #E4E4E4;
      }
    )");
    btn.setText(text);
    btn.setFixedSize(110, 100);
    btn.setAutoRepeat(true);
    btn.setAutoRepeatInterval(150);
    connect(&btn, &QPushButton::clicked, [this, delta]() { updateValue(delta); });
  }

  QPushButton btnMinus, btnPlus;
  QLabel label;
  Params params;
  Params paramsMemory{"/dev/shm/params"};
  bool isMetric = params.getBool("IsMetric");

  virtual void updateValue(int delta) = 0;
  virtual void refresh() = 0;
};

class FrogPilotPanel : public QWidget {
  Q_OBJECT

public:
  explicit FrogPilotPanel(QWidget *parent = nullptr) : QWidget(parent) {}
  QFrame *horizontalLine(QWidget *parent = nullptr) const;
  QFrame *whiteHorizontalLine(QWidget *parent = nullptr) const;

  Params params;
  Params paramsMemory{"/dev/shm/params"};
  bool isMetric = params.getBool("IsMetric");

protected:
  QVBoxLayout *mainLayout;
  std::map<std::string, std::vector<QWidget*>> childControls;

  ParamControl *createParamControl(const QString &key, const QString &label, const QString &desc, const QString &icon, QWidget *parent);
  QWidget *addSubControls(const QString &parentKey, QVBoxLayout *layout, const std::vector<std::tuple<QString, QString, QString>> &controls);
  QWidget *createDualParamControl(ParamValueControl *control1, ParamValueControl *control2);
  void addControl(const QString &key, const QString &label, const QString &desc, QVBoxLayout *layout, const QString &icon = "../assets/offroad/icon_blank.png");
  void createSubControl(const QString &key, const QString &label, const QString &desc, const QString &icon, const std::vector<QWidget*> &subControls, const std::vector<std::tuple<QString, QString, QString>> &additionalControls = {});
  void createSubButtonControl(const QString &parentKey, const std::vector<QPair<QString, QString>> &buttonKeys, QVBoxLayout *subControlLayout);
  void setInitialToggleStates();
  void setParams();
};

class FrogPilotControlsPanel : public FrogPilotPanel {
  Q_OBJECT

public:
  explicit FrogPilotControlsPanel(QWidget *parent = nullptr);
};

class FrogPilotVisualsPanel : public FrogPilotPanel {
  Q_OBJECT

public:
  explicit FrogPilotVisualsPanel(QWidget *parent = nullptr);
};
/////////////////////////////////////////////////////////////
class HFOPControlsPanel : public FrogPilotPanel {
  Q_OBJECT

public:
  explicit HFOPControlsPanel(QWidget *parent = nullptr);
};
/////////////////////////////////////////////////////////////
#define ParamController(className, paramName, labelText, descText, iconPath, getValueStrFunc, newValueFunc) \
class className : public ParamValueControl { \
  Q_OBJECT \
public: \
  className() : ParamValueControl(labelText, descText, iconPath) { \
    if (std::string(#className) == "SLCFallback" || std::string(#className) == "SLCPriority") { \
      label.setFixedWidth(750); \
    } \
    if (std::string(#className) == "AdjustablePersonalities") { \
      label.setFixedWidth(300); \
    } \
    if (std::string(#className) == "CameraView" || std::string(#className) == "DeviceShutdown" || std::string(#className) == "RouteInput" || std::string(#className) == "StoppingDistance" || std::string(#className) == "WheelIcon") { \
      label.setFixedWidth(225); \
    } \
    if (std::string(#className) == "CESpeed" || std::string(#className) == "CESpeedLead" || std::string(#className) == "Offset1" || std::string(#className) == "Offset2" || std::string(#className) == "Offset3" || std::string(#className) == "Offset4") { \
      label.setFixedWidth(180); \
    } \
    refresh(); \
  } \
private: \
  void refresh() override { \
    label.setText(getValueStr()); \
  } \
  void updateValue(int delta) override { \
    int value = params.getInt(paramName); \
    value = newValue(value + delta); \
    params.putIntNonBlocking(paramName, value); \
    paramsMemory.putBoolNonBlocking("FrogPilotTogglesUpdated", true); \
    if (std::string(#className) == "Model") { \
      params.remove("CalibrationParams"); \
      params.remove("LiveTorqueParameters"); \
    } \
    refresh(); \
  } \
  QString getValueStr() { getValueStrFunc; } \
  int newValue(int v) { newValueFunc; } \
};

ParamController(AccelerationProfile, "AccelerationProfile", "加速模式", "通過運動型或更節能的配置更改 openpilot 的加速速度.", "../assets/offroad/icon_blank.png",
  const int profile = params.getInt("AccelerationProfile");
  return profile == 1 ? "節能" : profile == 2 ? "正常" : "運動";,
  return std::clamp(v, 1, 3);
)

ParamController(AdjustablePersonalities, "AdjustablePersonalities", "跟車控制", "使用方向盤上的「距離」按鈕或透過其他品牌的道路使用者介面切換個性。.\n\n1 格 = 接近\n2 格 = 標準\n3 格 = 遠離", "../assets/offroad/icon_distance.png",
  const int selection = params.getInt("AdjustablePersonalities");
  return selection == 0 ? "None" : selection == 1 ? "Wheel" : selection == 2 ? "UI" : "Wheel + UI";,
  return v >= 0 ? v % 4 : 3;
)

ParamController(AggressiveJerk, "AggressiveJerk", "Jerk 值", "設定積極模式中的Jerk 值\n\n數值代表剎車/油門踏板的反應能力.\n\n更高的數值代表更少的力道/更“放鬆”\n\n預設值為0.5.", "../assets/offroad/icon_blank.png",
  return QString::number(params.getInt("AggressiveJerk") / 10.0);,
  return std::clamp(v, 1, 50);
)

ParamController(AggressiveFollow, "AggressiveFollow", "時間", "設定積極模式跟車距離\n\n數值反應與前車跟隨距離的秒數設定\n\n預設值為 1.25.", "../assets/aggressive.png",
  return QString::number(params.getInt("AggressiveFollow") / 10.0) + " sec";,
  return std::clamp(v, 10, 50);
)

ParamController(CameraView, "CameraView", "相機視角（僅限外觀）", "為 UI 設定您首選的相機視圖。此切換純粹是裝飾性的，不會影響openpilot 的使用.", "../assets/offroad/icon_camera.png",
  const int camera = params.getInt("CameraView");
  return camera == 0 ? "自動" : camera == 1 ? "標準" : camera == 2 ? "廣角" : "Driver";,
  return v >= 0 ? v % 4 : 3;
)

ParamController(CESpeed, "CESpeed", "時速設定", "當沒有前方車輛且低於此速度時切換到“實驗模式”.", "../assets/offroad/icon_blank.png",
  const int speed = params.getInt("CESpeed");
  return speed == 0 ? "關閉" : QString::number(speed) + (isMetric ? " 公里" : " mph");,
  return std::clamp(v, 0, isMetric ? 150 : 99);
)

ParamController(CESpeedLead, "CESpeedLead", "有前車", "當有前車且低於此速度時切換至實驗模式.", "../assets/offroad/icon_blank.png",
  const int speedLead = params.getInt("CESpeedLead");
  return speedLead == 0 ? "關閉" : QString::number(speedLead) + (isMetric ? " 公里" : " mph");,
  return std::clamp(v, 0, isMetric ? 150 : 99);
)

ParamController(CurveSensitivity, "CurveSensitivity", "曲線檢測靈敏度", "改變汽車對道路彎道的敏感度。 較高的值使汽車對彎道的反應更早，而較低的值可能會導致更平滑但反應更晚.", "../assets/offroad/icon_blank.png",
  return QString::number(params.getInt("CurveSensitivity")) + "%";,
  return std::clamp(v, 1, 200);
)

ParamController(CustomColors, "CustomColors", "顏色 ", "使用自訂配色方案替換庫存 openpilot 顏色.", "../assets/offroad/icon_blank.png",
  const int colors = params.getInt("CustomColors");
  return colors == 0 ? "Stock" : colors == 1 ? "Frog" : colors == 2 ? "Tesla" : "Stalin";,
  return v >= 0 ? v % 4 : 3;
)

ParamController(CustomIcons, "CustomIcons", "圖示", "用自訂圖標包替換庫存 openpilot 圖標.", "../assets/offroad/icon_blank.png",
  const int icons = params.getInt("CustomIcons");
  return icons == 0 ? "Stock" : icons == 1 ? "Frog" : icons == 2 ? "Tesla" : "Stalin";,
  return v >= 0 ? v % 4 : 3;
)

ParamController(CustomSignals, "CustomSignals", "訊號", "啟用自訂方向燈動畫.", "../assets/offroad/icon_blank.png",
  const int turnSignals = params.getInt("CustomSignals");
  return turnSignals == 0 ? "Stock" : turnSignals == 1 ? "Frog" : "Stalin";,
  return v >= 0 ? v % 4 : 3;
)

ParamController(CustomSounds, "CustomSounds", "聲音", "用自訂聲音包替換庫存 openpilot 聲音.", "../assets/offroad/icon_blank.png",
  const int sounds = params.getInt("CustomSounds");
  return sounds == 0 ? "Stock" : sounds == 1 ? "Frog" : sounds == 2 ? "Tesla" : "Stalin";,
  return v >= 0 ? v % 4 : 3;
)

ParamController(DeviceShutdown, "DeviceShutdown", "設備自動關機設定", "設置設備在熄火後自動關閉的計時器，以減少能源浪費並防止電池耗盡.", "../assets/offroad/icon_time.png",
  const int time = params.getInt("DeviceShutdown");
  return time == 0 ? "立刻" : (time > 0 && time <= 3) ? QString::number(time * 15) + " mins" : QString::number(time - 3) + (time == 4 ? " hour" : " hours");,
  return std::clamp(v, 0, 33);
)

ParamController(LaneChangeTime, "LaneChangeTime", "自動變換車道延遲", "設定自動變換車道延遲時間.", "../assets/offroad/icon_blank.png",
  const int delay = params.getInt("LaneChangeTime");
  return delay == 0 ? "立刻" : QString::number(static_cast<double>(delay) / 2.0) + " sec";,
  return std::clamp(v, 0, 10);
)

ParamController(LaneLinesWidth, "LaneLinesWidth", "車道寬", "自定義車道線寬度。默認匹配 MUTCD 平均值 4 英寸.", "../assets/offroad/icon_blank.png",
  return QString::number(params.getInt("LaneLinesWidth")) + (isMetric ? " cm" : " in");,
  return std::clamp(v, 0, isMetric ? 60 : 24);
)

ParamController(Model, "Model", "模型選擇 (須重啟)", "選擇您喜歡的 openpilot 模型.\n\nFV = Farmville(Default)\nNLP = New Lemon Pie\nBD = Blue Diamond", "../assets/offroad/icon_calibration.png",
  const int model = params.getInt("Model");
  return model == 0 ? "FV" : model == 1 ? "NLP" : "BD";,
  return v >= 0 ? v % 3 : 2;
)

ParamController(Offset1, "Offset1", "0-34", "Set the speed limit offset when the speed limit is between 0 and 34 mph.", "../assets/icon_blank.png",
  return QString::number(params.getInt("Offset1")) + " mph";,
  return std::clamp(v, 0, 99);
)

ParamController(Offset1Metric, "Offset1", "0-54", "Set the speed limit offset when the speed limit is between 0 and 34 mph.", "../assets/icon_blank.png",
  return QString::number(params.getInt("Offset1")) + " 公里";,
  return std::clamp(v, 0, 99);
)

ParamController(Offset2, "Offset2", "35-54", "Set the speed limit offset when the speed limit is between 35 and 54 mph.", "../assets/icon_blank.png",
  return QString::number(params.getInt("Offset2")) + " mph";,
  return std::clamp(v, 0, 99);
)

ParamController(Offset2Metric, "Offset2", "55-89", "Set the speed limit offset when the speed limit is between 35 and 54 mph.", "../assets/icon_blank.png",
  return QString::number(params.getInt("Offset2")) + " 公里";,
  return std::clamp(v, 0, 99);
)

ParamController(Offset3, "Offset3", "55-64", "Set the speed limit offset when the speed limit is between 55 and 64 mph.", "../assets/icon_blank.png",
  return QString::number(params.getInt("Offset3")) + " mph";,
  return std::clamp(v, 0, 99);
)

ParamController(Offset3Metric, "Offset3", "90-104", "Set the speed limit offset when the speed limit is between 55 and 64 mph.", "../assets/icon_blank.png",
  return QString::number(params.getInt("Offset3")) + " 公里";,
  return std::clamp(v, 0, 99);
)

ParamController(Offset4, "Offset4", "65-99", "Set the speed limit offset when the speed limit is between 65 and 99 mph.", "../assets/icon_blank.png",
  return QString::number(params.getInt("Offset4")) + " mph";,
  return std::clamp(v, 0, 99);
)

ParamController(Offset4Metric, "Offset4", "105-159", "Set the speed limit offset when the speed limit is between 65 and 99 mph.", "../assets/icon_blank.png",
  return QString::number(params.getInt("Offset4")) + " 公里";,
  return std::clamp(v, 0, 99);
)

ParamController(PathEdgeWidth, "PathEdgeWidth", "路徑邊寬", "自定義顯示當前駕駛狀態的路徑邊緣寬度。預設為總路徑的 20%。\n\n藍色 =導航\n\n淺藍色 =全時置中\n綠色 = 默認使用“FrogPilot 顏色”\n淺綠色 = 預設使用原始顏色\n橙色 = 實驗模式啟動 \n黃色 = 條件模式", "../assets/offroad/icon_blank.png",
  return QString::number(params.getInt("PathEdgeWidth")) + "%";,
  return std::clamp(v, 0, 100);
)

ParamController(PathWidth, "PathWidth", "路徑寬", "自定義路徑寬度。\n\n預設為 skoda kodiaq 的寬度.", "../assets/offroad/icon_blank.png",
  return QString::number(params.getInt("PathWidth") / 10.0) + (isMetric ? " m" : " ft");,
  return std::clamp(v, 0, isMetric ? 30 : 100);
)

ParamController(RelaxedJerk, "RelaxedJerk", "Jerk 值", "設置“輕鬆模式”的Jerk值。\n\n數值代表剎車/油門踏板的反應能力.\n\n更高的數值代表更少的力道/更“放鬆”\n\n預設值為1.0.", "../assets/offroad/icon_blank.png",
  return QString::number(params.getInt("RelaxedJerk") / 10.0);,
  return std::clamp(v, 1, 50);
)

ParamController(RelaxedFollow, "RelaxedFollow", "時間", "設定輕鬆模式跟車距離。\n\n數值反應與前車跟隨距離的秒數設定\n\n預設值為 1.75.", "../assets/relaxed.png",
  return QString::number(params.getInt("RelaxedFollow") / 10.0) + " sec";,
  return std::clamp(v, 10, 50);
)

ParamController(RoadEdgesWidth, "RoadEdgesWidth", "道路邊寬", "自定義道路邊緣寬度。\n\n預設值為 MUTCD 平均車道線寬度 4 英寸.", "../assets/offroad/icon_blank.png",
  return QString::number(params.getInt("RoadEdgesWidth")) + (isMetric ? " cm" : " in");,
  return std::clamp(v, 0, isMetric ? 60 : 24);
)

ParamController(ScreenBrightness, "ScreenBrightness", "螢幕亮度", "設定螢幕亮度或使用預設的“自動”亮度設定.", "../assets/offroad/icon_light.png",
  const int brightness = params.getInt("ScreenBrightness");
  uiState()->scene.screen_brightness = brightness;
  return brightness == 101 ? "自動" : brightness == 0 ? "Off" : QString::number(brightness) + "%";,
  return std::clamp(v, 0, 101);
)

ParamController(SLCFallback, "SLCFallback", "SLC 備援方式", "當導航、OSM 或汽車儀表板中沒有速度限制時，設定您的首選後備方法.", "../assets/offroad/icon_blank.png",
  const int fallback = params.getInt("SLCFallback");
  return fallback == 0 ? "無" : fallback == 1 ? "實驗模式" : "之前的限速";,
  return v >= 0 ? v % 3 : 2;
)

ParamController(SLCPriority, "SLCPriority", "SLC 優先選項", "在決定限速控制器所使用的限速時設定您的首選優先順序.", "../assets/offroad/icon_blank.png",
  const int priority = params.getInt("SLCPriority");
  return priority == 0 ? "導航, 儀表, OSM" : 
         priority == 1 ? "導航, OSM, 儀表" : 
         priority == 2 ? "導航, OSM" : 
         priority == 3 ? "導航, 儀表" : 
         priority == 4 ? "導航" : 
         priority == 5 ? "OSM, 儀表, 導航" : 
         priority == 6 ? "OSM, 導航, 儀表" : 
         priority == 7 ? "OSM, 導航" : 
         priority == 8 ? "OSM, 儀表" : 
         priority == 9 ? "OSM" : 
         priority == 10 ? "儀表, 導航, OSM" : 
         priority == 11 ? "儀表, OSM, 導航" : 
         priority == 12 ? "儀表, OSM" : 
         priority == 13 ? "儀表, 導航" : 
         priority == 14 ? "儀表" : 
         priority == 15 ? "最高" : 
         "最低";,
  return v >= 0 ? v % 17 : 16;
)

ParamController(StandardJerk, "StandardJerk", "Jerk 值", "設置“標準模式”的Jerk值。\n\n數值代表剎車/油門踏板的反應能力.\n\n更高的數值代表更少的力道/更“放鬆”\n\n預設值為1.0.", "../assets/offroad/icon_blank.png",
  return QString::number(params.getInt("StandardJerk") / 10.0);,
  return std::clamp(v, 1, 50);
)

ParamController(StandardFollow, "StandardFollow", "時間", "設定標準模式跟車距離。\n\n數值反應與前車跟隨距離的秒數設定\n\n預設值為 1.45.", "../assets/standard.png",
  return QString::number(params.getInt("StandardFollow") / 10.0) + " sec";,
  return std::clamp(v, 10, 50);
)

ParamController(StoppingDistance, "StoppingDistance", "增加停止距離", "增加停車距離以獲得更舒適的停車體驗.", "../assets/offroad/icon_blank.png",
  const int distance = params.getInt("StoppingDistance");
  return distance == 0 ? "關閉" : QString::number(distance) + (isMetric ? " meters" : " feet");,
  return std::clamp(v, 0, isMetric ? 5 : 15);
)

ParamController(WheelIcon, "WheelIcon", "方向盤圖示", "用自定義圖標替換官方方向盤圖示", "../assets/offroad/icon_openpilot.png",
  const int wheel = params.getInt("WheelIcon");
  return wheel == 0 ? "Stock" : wheel == 1 ? "Lexus" : wheel == 2 ? "Toyota" : wheel == 3 ? "Frog" : wheel == 4 ? "Rocket" : wheel == 5 ? "Hyundai" : "Stalin";,
  return v >= 0 ? v % 7 : 6;
)

ParamController(TurnAggressiveness, "TurnAggressiveness", "轉彎速度積極性", "調整汽車速度。 較高的值意味著轉彎更快，而較低的值意味著轉彎更平緩.", "../assets/offroad/icon_blank.png",
  return QString::number(params.getInt("TurnAggressiveness")) + "%";,
  return std::clamp(v, 1, 200);
)
////////////////////////////////////////////////////////////////////////////////////////////////////
ParamController(AutoACCspeed, "AutoACCspeed", "自動啟動ACC設定", "設定自動啟動ACC的時速條件設定.", "../assets/offroad/icon_blank.png",
  const int speed = params.getInt("AutoACCspeed");
  return speed == 0 ? "關閉" : QString::number(speed) + (isMetric ? " 公里" : " feet");,
  return std::clamp(v, 0, isMetric ? 50 : 60);
)

ParamController(CarAwayspeed, "CarAwayspeed", "前車速度差設定", "設定前車時速大於多少公里時提醒.", "../assets/offroad/icon_blank.png",
  const int caspeed = params.getInt("CarAwayspeed");
  return caspeed == 0 ? "關閉" : QString::number(caspeed) + (isMetric ? " 公里" : " feet");,
  return std::clamp(v, 0, isMetric ? 10 : 30);
)

ParamController(CarAwaydistance, "CarAwaydistance", "前車距離差設定", "設定前車距離大於多少公里時提醒.", "../assets/offroad/icon_blank.png",
  const int caspeed = params.getInt("CarAwaydistance");
  return caspeed == 0 ? "關閉" : QString::number(caspeed) + (isMetric ? " 公尺" : " feet");,
  return std::clamp(v, 0, isMetric ? 10 : 30);
)

ParamController(RoadtypeProfile, "RoadtypeProfile", "選擇行駛的道路種類", "關閉或選擇目前行駛的路段可依特定條件改變最高時速設定.", "../assets/offroad/icon_blank.png",
  const int roadtype = params.getInt("RoadtypeProfile");
  return roadtype == 0 ? "關閉" :roadtype == 1 ? "平面" : roadtype == 2 ? "快速" : "高速";,
  return std::clamp(v, 0, 3);
)
////////////////////////////////////////////////////////////////////////////////////////////////////

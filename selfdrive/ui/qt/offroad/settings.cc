#include "selfdrive/ui/qt/offroad/settings.h"

#include <cassert>
#include <cmath>
#include <string>
#include <tuple>
#include <vector>

#include <QDebug>

#include "selfdrive/ui/qt/network/networking.h"

#include "common/params.h"
#include "common/watchdog.h"
#include "common/util.h"
#include "system/hardware/hw.h"
#include "selfdrive/ui/qt/widgets/controls.h"
#include "selfdrive/ui/qt/widgets/input.h"
#include "selfdrive/ui/qt/widgets/scrollview.h"
#include "selfdrive/ui/qt/widgets/ssh_keys.h"
#include "selfdrive/ui/qt/widgets/toggle.h"
#include "selfdrive/ui/ui.h"
#include "selfdrive/ui/qt/util.h"
#include "selfdrive/ui/qt/qt_window.h"

#include "selfdrive/frogpilot/ui/control_settings.h"
#include "selfdrive/frogpilot/ui/vehicle_settings.h"
#include "selfdrive/frogpilot/ui/visual_settings.h"
#include "selfdrive/frogpilot/navigation/ui/navigation_settings.h"
/////////////////////////////////////////////////////
#include "selfdrive/frogpilot/ui/hfop_settings.h"
/////////////////////////////////////////////////////

TogglesPanel::TogglesPanel(SettingsWindow *parent) : ListWidget(parent) {
  // param, title, desc, icon
  std::vector<std::tuple<QString, QString, QString, QString>> toggle_defs{
    {
      "OpenpilotEnabledToggle",
      tr("啟用 openpilot"),
      tr("使用 openpilot 的主動式巡航和車道保持功能，開啟後您需要持續集中注意力，設定變更在重新啟動車輛後生效。"),
      "../assets/offroad/icon_openpilot.png",
    },
    {
      "ExperimentalLongitudinalEnabled",
      tr("openpilot 縱向控制 (Alpha 版)"),
      QString("<b>%1</b><br><br>%2")
      .arg(tr("警告：此車輛的 Openpilot 縱向控制功能目前處於 Alpha 版本，使用此功能將會停用自動緊急制動（AEB）功能。"))
      .arg(tr("在這輛車上，Openpilot 預設使用車輛內建的主動巡航控制（ACC），而非 Openpilot 的縱向控制。啟用此項功能可切換至 Openpilot 的縱向控制。當啟用 Openpilot 縱向控制 Alpha 版本時，建議同時啟用實驗性模式（Experimental mode）。")),
      "../assets/offroad/icon_speed_limit.png",
    },
    {
      "ExperimentalMode",
      tr("實驗模式"),
      "",
      "../assets/img_experimental_white.svg",
    },
    {
      "DisengageOnAccelerator",
      tr("油門取消控車"),
      tr("啟用後，踩踏油門將會取消 openpilot 控制。"),
      "../assets/offroad/icon_disengage_on_accelerator.svg",
    },
    {
      "IsLdwEnabled",
      tr("啟用車道偏離警告"),
      tr("車速在時速 50 公里 (31 英里) 以上且未打方向燈的情況下，如果偵測到車輛駛出目前車道線時，發出車道偏離警告。"),
      "../assets/offroad/icon_warning.png",
    },
    {
      "RecordFront",
      tr("記錄並上傳駕駛監控影像"),
      tr("上傳駕駛監控的錄像來協助我們提升駕駛監控的準確率。"),
      "../assets/offroad/icon_monitoring.png",
    },
    {
      "IsMetric",
      tr("使用公制單位"),
      tr("啟用後，速度單位顯示將從 mp/h 改為 km/h。"),
      "../assets/offroad/icon_metric.png",
    },
#ifdef ENABLE_MAPS
    {
      "NavSettingTime24h",
      tr("預計到達時間單位改用 24 小時制"),
      tr("使用 24 小時制。(預設值為 12 小時制)"),
      "../assets/offroad/icon_metric.png",
    },
    {
      "NavSettingLeftSide",
      tr("將地圖顯示在畫面的左側"),
      tr("進入分割畫面後，地圖將會顯示在畫面的左側。"),
      "../assets/offroad/icon_road.png",
    },
#endif
  };


  std::vector<QString> longi_button_texts{tr("積極"), tr("標準"), tr("舒適")};
  long_personality_setting = new ButtonParamControl("LongitudinalPersonality", tr("駕駛模式"),
                                          tr("推薦使用標準模式。在積極模式中，openpilot 會更靠近前車並在加速和剎車方面更積極。在舒適模式中，openpilot 會與前車保持較遠的距離。"),
                                          "../assets/offroad/icon_speed_limit.png",
                                          longi_button_texts);
  for (auto &[param, title, desc, icon] : toggle_defs) {
    auto toggle = new ParamControl(param, title, desc, icon, this);

    bool locked = params.getBool((param + "Lock").toStdString());
    toggle->setEnabled(!locked);

    addItem(toggle);
    toggles[param.toStdString()] = toggle;

    // insert longitudinal personality after NDOG toggle
    if (param == "DisengageOnAccelerator" && !params.getInt("AdjustablePersonalities")) {
      addItem(long_personality_setting);
    }
  }

  // Toggles with confirmation dialogs
  toggles["ExperimentalMode"]->setActiveIcon("../assets/img_experimental.svg");
  toggles["ExperimentalMode"]->setConfirmation(true, true);
  toggles["ExperimentalLongitudinalEnabled"]->setConfirmation(true, false);

  connect(toggles["ExperimentalLongitudinalEnabled"], &ToggleControl::toggleFlipped, [=]() {
    updateToggles();
  });
}

void TogglesPanel::expandToggleDescription(const QString &param) {
  toggles[param.toStdString()]->showDescription();
}

void TogglesPanel::showEvent(QShowEvent *event) {
  updateToggles();
}

void TogglesPanel::updateToggles() {
  auto experimental_mode_toggle = toggles["ExperimentalMode"];
  auto op_long_toggle = toggles["ExperimentalLongitudinalEnabled"];
  const QString e2e_description = QString("%1<br>"
                                          "<h4>%2</h4><br>"
                                          "%3<br>"
                                          "<h4>%4</h4><br>"
                                          "%5<br>"
                                          "<h4>%6</h4><br>"
                                          "%7")
                                  .arg(tr("openpilot 預設以 <b>輕鬆模式</b>駕駛. 實驗模式啟用了尚未準備好進入輕鬆模式的 <b>alpha 級功能</b> 。實驗功能如下：:"))
                                  .arg(tr("點對點的縱向控制"))
                                  .arg(tr("讓駕駛模型來控制油門及煞車。openpilot將會模擬人類的駕駛行為，包含在看見紅燈及停止標示時停車。"
                                          "由於車速將由駕駛模型決定，因此您設定的時速將成為速度上限。本功能仍在早期實驗階段，請預期模型有犯錯的可能性。"))
                                  .arg(tr("在openpilot上使用導航"))
                                  .arg(tr("當導航有目的地時openpilot會將地圖資訊輸入到模型中。這為模型提供了有用訊息 並允許 openpilot 在分叉/出口處適當地保持左或右。"
                                          "變道行為仍需由駕駛啟動。這是一個alpha品質功能;在出口和分叉處容易有誤判。"
                                          "設置導航目的地並且駕駛模型將其用作輸入時，地圖上的行駛路徑將變為綠色。"))
                                  .arg(tr("新的駕駛視覺介面"))
                                  .arg(tr("行駛畫面將在低速時切換至道路朝向的廣角鏡頭，以更好地顯示一些轉彎。實驗模式圖標也將顯示在右上角。當設定了導航目的地並且行駛模型正在將其作為輸入時，地圖上的行駛路徑將變為綠色。"));

  const bool is_release = params.getBool("IsReleaseBranch");
  auto cp_bytes = params.get("CarParamsPersistent");
  if (!cp_bytes.empty()) {
    AlignedBuffer aligned_buf;
    capnp::FlatArrayMessageReader cmsg(aligned_buf.align(cp_bytes.data(), cp_bytes.size()));
    cereal::CarParams::Reader CP = cmsg.getRoot<cereal::CarParams>();

    if (!CP.getExperimentalLongitudinalAvailable() || is_release) {
      params.remove("ExperimentalLongitudinalEnabled");
    }
    op_long_toggle->setVisible(CP.getExperimentalLongitudinalAvailable() && !is_release);
    if (hasLongitudinalControl(CP)) {
      // normal description and toggle
      experimental_mode_toggle->setEnabled(!params.getBool("ConditionalExperimental"));
      experimental_mode_toggle->setDescription(e2e_description);
      long_personality_setting->setEnabled(true);
    } else {
      // no long for now
      experimental_mode_toggle->setEnabled(false);
      long_personality_setting->setEnabled(false);
      params.remove("ExperimentalMode");

      const QString unavailable = tr("因車輛使用內建ACC系統，無法在本車輛上啟動實驗模式。");

      QString long_desc = unavailable + " " + \
                          tr("openpilot 縱向控制可能會在未來的更新中提供。");
      if (CP.getExperimentalLongitudinalAvailable()) {
        if (is_release) {
          long_desc = unavailable + " " + tr("在正式 (release) 版以外的分支上可以測試 openpilot 縱向控制的 Alpha 版本，以及實驗模式。");
        } else {
          long_desc = tr("啟用 openpilot 縱向控制 (alpha) 切換以允許實驗模式。");
        }
      }
      experimental_mode_toggle->setDescription("<b>" + long_desc + "</b><br><br>" + e2e_description);
    }

    experimental_mode_toggle->refresh();
  } else {
    experimental_mode_toggle->setDescription(e2e_description);
    op_long_toggle->setVisible(false);
  }
}

DevicePanel::DevicePanel(SettingsWindow *parent) : ListWidget(parent) {
  setSpacing(50);
  addItem(new LabelControl(tr("Dongle ID"), getDongleId().value_or(tr("無法使用"))));
  addItem(new LabelControl(tr("序號"), params.get("HardwareSerial").c_str()));

  // offroad-only buttons

  auto dcamBtn = new ButtonControl(tr("駕駛監控鏡頭"), tr("預覽"),
                                   tr("預覽駕駛員監控鏡頭畫面，以確保其具有良好視野。（僅在熄火時可用）"));
  connect(dcamBtn, &ButtonControl::clicked, [=]() { emit showDriverView(); });
  addItem(dcamBtn);

  auto resetCalibBtn = new ButtonControl(tr("重設校正"), tr("重設"), "");
  connect(resetCalibBtn, &ButtonControl::showDescriptionEvent, this, &DevicePanel::updateCalibDescription);
  connect(resetCalibBtn, &ButtonControl::clicked, [&]() {
    if (ConfirmationDialog::confirm(tr("是否確定要重設校正?"), tr("重設"), this)) {
      params.remove("CalibrationParams");
      params.remove("LiveTorqueParameters");
    }
  });
  addItem(resetCalibBtn);

  if (!params.getBool("Passive")) {
    auto retrainingBtn = new ButtonControl(tr("觀看使用教學"), tr("觀看"), tr("觀看 openpilot 的使用規則、功能和限制"));
    connect(retrainingBtn, &ButtonControl::clicked, [=]() {
      if (ConfirmationDialog::confirm(tr("是否確定要觀看使用教學?"), tr("觀看"), this)) {
        emit reviewTrainingGuide();
      }
    });
    addItem(retrainingBtn);
  }

  if (Hardware::TICI()) {
    auto regulatoryBtn = new ButtonControl(tr("法規/監管"), tr("觀看"), "");
    connect(regulatoryBtn, &ButtonControl::clicked, [=]() {
      const std::string txt = util::read_file("../assets/offroad/fcc.html");
      ConfirmationDialog::rich(QString::fromStdString(txt), this);
    });
    addItem(regulatoryBtn);
  }

  auto translateBtn = new ButtonControl(tr("更改語言"), tr("更改"), "");
  connect(translateBtn, &ButtonControl::clicked, [=]() {
    QMap<QString, QString> langs = getSupportedLanguages();
    QString selection = MultiOptionDialog::getSelection(tr("選擇語系"), langs.keys(), langs.key(uiState()->language), this);
    if (!selection.isEmpty()) {
      // put language setting, exit Qt UI, and trigger fast restart
      params.put("LanguageSetting", langs[selection].toStdString());
      qApp->exit(18);
      watchdog_kick(0);
    }
  });
  addItem(translateBtn);

  // Delete driving footage button
  const auto deleteFootageBtn = new ButtonControl(tr("刪除駕駛數據"), tr("刪除"), tr("此按鈕提供了一種快速且安全的方式來永久刪除所有"
    "裝置中儲存的駕駛錄影和資料。非常適合維護隱私或釋放空間.")
  );
  connect(deleteFootageBtn, &ButtonControl::clicked, [this]() {
    if (!ConfirmationDialog::confirm(tr("您確定要永久刪除所有駕駛錄影和資料嗎?"), tr("刪除"), this)) return;
    std::thread([&] {
      std::system("rm -rf /data/media/0/realdata");
    }).detach();
  });
  addItem(deleteFootageBtn);

  // Panda flashing button
  const auto flashPandaBtn = new ButtonControl(tr("刷新 Panda"), tr("刷新"), "");
  connect(flashPandaBtn, &ButtonControl::clicked, [this]() {
    if (!ConfirmationDialog::confirm(tr("是否確定要刷新 Panda?"), tr("刷新"), this)) return;
    QProcess process;
    // Get Panda type
    const SubMaster &sm = *(uiState()->sm);
    const auto pandaStates = sm["pandaStates"].getPandaStates();
    // Choose recovery script based on Panda type
    if (pandaStates.size() != 0) {
      const auto pandaType = pandaStates[0].getPandaType();
      const bool isRedPanda = (pandaType == cereal::PandaState::PandaType::RED_PANDA ||
                               pandaType == cereal::PandaState::PandaType::RED_PANDA_V2);
      const QString recoveryScript = isRedPanda ? "./recover.sh" : "./recover.py";
      // Run recovery script and flash Panda
      process.setWorkingDirectory("/data/openpilot/panda/board");
      process.start("/bin/sh", QStringList{"-c", recoveryScript});
      process.waitForFinished();
    }
    // Run the killall script as a redundancy
    process.setWorkingDirectory("/data/openpilot/panda");
    process.start("/bin/sh", QStringList{"-c", "pkill -f boardd; PYTHONPATH=.. python -c \"from panda import Panda; Panda().flash()\""});
    process.waitForFinished();
    Hardware::reboot();
  });
  addItem(flashPandaBtn);

  QObject::connect(uiState(), &UIState::offroadTransition, [=](bool offroad) {
    for (auto btn : findChildren<ButtonControl *>()) {
      btn->setEnabled(offroad);
    }
  });

  // power buttons
  QHBoxLayout *power_layout = new QHBoxLayout();
  power_layout->setSpacing(30);

  QPushButton *reboot_btn = new QPushButton(tr("重啟"));
  reboot_btn->setObjectName("reboot_btn");
  power_layout->addWidget(reboot_btn);
  QObject::connect(reboot_btn, &QPushButton::clicked, this, &DevicePanel::reboot);

  QPushButton *poweroff_btn = new QPushButton(tr("關機"));
  poweroff_btn->setObjectName("poweroff_btn");
  power_layout->addWidget(poweroff_btn);
  QObject::connect(poweroff_btn, &QPushButton::clicked, this, &DevicePanel::poweroff);

  if (!Hardware::PC()) {
    connect(uiState(), &UIState::offroadTransition, poweroff_btn, &QPushButton::setVisible);
  }

  setStyleSheet(R"(
    #reboot_btn { height: 120px; border-radius: 15px; background-color: #393939; }
    #reboot_btn:pressed { background-color: #4a4a4a; }
    #poweroff_btn { height: 120px; border-radius: 15px; background-color: #E22C2C; }
    #poweroff_btn:pressed { background-color: #FF2424; }
  )");
  addItem(power_layout);
}

void DevicePanel::updateCalibDescription() {
  QString desc =
      tr("openpilot 需要將設備固定在左右偏差 4° 以內，朝上偏差 5° 以内或朝下偏差 8° 以内。 "
         "鏡頭在後台會持續自動校準，很少有需要重置的情况。");
  std::string calib_bytes = params.get("CalibrationParams");
  if (!calib_bytes.empty()) {
    try {
      AlignedBuffer aligned_buf;
      capnp::FlatArrayMessageReader cmsg(aligned_buf.align(calib_bytes.data(), calib_bytes.size()));
      auto calib = cmsg.getRoot<cereal::Event>().getLiveCalibration();
      if (calib.getCalStatus() != cereal::LiveCalibrationData::Status::UNCALIBRATED) {
        double pitch = calib.getRpyCalib()[1] * (180 / M_PI);
        double yaw = calib.getRpyCalib()[2] * (180 / M_PI);
        desc += tr("你的設備目前朝 %1° %2 以及朝 %3° %4.")
                    .arg(QString::number(std::abs(pitch), 'g', 1), pitch > 0 ? tr("下") : tr("上"),
                         QString::number(std::abs(yaw), 'g', 1), yaw > 0 ? tr("左") : tr("右"));
      }
    } catch (kj::Exception) {
      qInfo() << "invalid CalibrationParams";
    }
  }
  qobject_cast<ButtonControl *>(sender())->setDescription(desc);
}

void DevicePanel::reboot() {
  if (!uiState()->engaged()) {
    if (ConfirmationDialog::confirm(tr("是否確定要重新啟動?"), tr("重新啟動"), this)) {
      // Check engaged again in case it changed while the dialog was open
      if (!uiState()->engaged()) {
        params.putBool("DoReboot", true);
      }
    }
  } else {
    ConfirmationDialog::alert(tr("請先取消控車才能重新啟動"), this);
  }
}

void DevicePanel::poweroff() {
  if (!uiState()->engaged()) {
    if (ConfirmationDialog::confirm(tr("是否確定要關機?"), tr("關機"), this)) {
      // Check engaged again in case it changed while the dialog was open
      if (!uiState()->engaged()) {
        params.putBool("DoShutdown", true);
      }
    }
  } else {
    ConfirmationDialog::alert(tr("請先取消控車才能關機"), this);
  }
}

void SettingsWindow::showEvent(QShowEvent *event) {
  setCurrentPanel(0);
}

void SettingsWindow::setCurrentPanel(int index, const QString &param) {
  panel_widget->setCurrentIndex(index);
  nav_btns->buttons()[index]->setChecked(true);
  if (!param.isEmpty()) {
    emit expandToggleDescription(param);
  }
}

SettingsWindow::SettingsWindow(QWidget *parent) : QFrame(parent) {

  // setup two main layouts
  sidebar_widget = new QWidget;
  QVBoxLayout *sidebar_layout = new QVBoxLayout(sidebar_widget);
  sidebar_layout->setMargin(0);
  panel_widget = new QStackedWidget();

  // close button
  QPushButton *close_btn = new QPushButton(tr("← 返回"));
  close_btn->setStyleSheet(R"(
    QPushButton {
      font-size: 50px;
      padding-bottom: 0px;
      border 1px grey solid;
      border-radius: 25px;
      background-color: #292929;
      font-weight: 500;
    }
    QPushButton:pressed {
      background-color: #3B3B3B;
    }
  )");
  close_btn->setFixedSize(300, 125);
  sidebar_layout->addSpacing(10);
  sidebar_layout->addWidget(close_btn, 0, Qt::AlignRight);
  QObject::connect(close_btn, &QPushButton::clicked, [this]() {
    if (paramsMemory.getInt("FrogPilotTogglesOpen") == 1) {
      paramsMemory.putInt("FrogPilotTogglesOpen", 2);
    } else {
      this->closeSettings();
    }
  });

  // setup panels
  DevicePanel *device = new DevicePanel(this);
  QObject::connect(device, &DevicePanel::reviewTrainingGuide, this, &SettingsWindow::reviewTrainingGuide);
  QObject::connect(device, &DevicePanel::showDriverView, this, &SettingsWindow::showDriverView);

  TogglesPanel *toggles = new TogglesPanel(this);
  QObject::connect(this, &SettingsWindow::expandToggleDescription, toggles, &TogglesPanel::expandToggleDescription);

  QList<QPair<QString, QWidget *>> panels = {
    {tr("設備資訊"), device},
    {tr("網路設定"), new Networking(this)},
    {tr("官方設定"), toggles},
    {tr("軟體資訊"), new SoftwarePanel(this)},
    {tr("控制設定"), new FrogPilotControlsPanel(this)},
    {tr("導航設定"), new FrogPilotNavigationPanel(this)},
    {tr("車輛設定"), new FrogPilotVehiclesPanel(this)},
    {tr("介面設定"), new FrogPilotVisualsPanel(this)},
/////////////////////////////////////////////////////
    {tr("H F O P"), new HFOPControlsPanel(this)},
/////////////////////////////////////////////////////
  };

  nav_btns = new QButtonGroup(this);
  for (auto &[name, panel] : panels) {
    QPushButton *btn = new QPushButton(name);
    btn->setCheckable(true);
    btn->setChecked(nav_btns->buttons().size() == 0);
    btn->setStyleSheet(R"(
      QPushButton {
        color: grey;
        border: none;
        background: none;
        font-size: 55px;
        font-weight: 500;
      }
      QPushButton:checked {
        color: white;
      }
      QPushButton:pressed {
        color: #ADADAD;
      }
    )");
    btn->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    nav_btns->addButton(btn);
    sidebar_layout->addWidget(btn, 0, Qt::AlignHCenter);

    const int lr_margin = name != tr("網路") ? 50 : 0;  // Network panel handles its own margins
    panel->setContentsMargins(lr_margin, 25, lr_margin, 25);

    ScrollView *panel_frame = new ScrollView(panel, this);
    panel_widget->addWidget(panel_frame);

    QObject::connect(btn, &QPushButton::clicked, [=, w = panel_frame]() {
      btn->setChecked(true);
      panel_widget->setCurrentWidget(w);
    });
  }
  sidebar_layout->setContentsMargins(50, 50, 100, 50);

  // main settings layout, sidebar + main panel
  QHBoxLayout *main_layout = new QHBoxLayout(this);

  sidebar_widget->setFixedWidth(500);
  main_layout->addWidget(sidebar_widget);
  main_layout->addWidget(panel_widget);

  setStyleSheet(R"(
    * {
      color: white;
      font-size: 50px;
    }
    SettingsWindow {
      background-color: black;
    }
    QStackedWidget, ScrollView {
      background-color: #292929;
      border-radius: 30px;
    }
  )");
}

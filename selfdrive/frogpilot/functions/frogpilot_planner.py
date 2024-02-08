import cereal.messaging as messaging
import numpy as np

from openpilot.common.conversions import Conversions as CV
from openpilot.common.numpy_fast import clip
from openpilot.selfdrive.controls.lib.desire_helper import LANE_CHANGE_SPEED_MIN
from openpilot.selfdrive.controls.lib.longitudinal_planner import A_CRUISE_MIN, A_CRUISE_MAX_BP, get_max_accel

from openpilot.selfdrive.frogpilot.functions.frogpilot_functions import FrogPilotFunctions

from openpilot.selfdrive.frogpilot.functions.conditional_experimental_mode import ConditionalExperimentalMode
from openpilot.selfdrive.frogpilot.functions.map_turn_speed_controller import MapTurnSpeedController
from openpilot.selfdrive.frogpilot.functions.speed_limit_controller import SpeedLimitController

# VTSC variables
MIN_TARGET_V = 5    # m/s
TARGET_LAT_A = 1.9  # m/s^2

class FrogPilotPlanner:
  def __init__(self, params, params_memory):
    self.cem = ConditionalExperimentalMode()
    self.mtsc = MapTurnSpeedController()

    self.override_slc = False

    self.overridden_speed = 0
    self.mtsc_target = 0
    self.slc_target = 0
    self.v_cruise = 0
    self.vtsc_target = 0

    self.accel_limits = [A_CRUISE_MIN, get_max_accel(0)]

    ##############################
    self.carawayck = False
    self.detect_speed_prev = 0
    self.slchangedu = False
    self.slchangedd = False
    self.speedover = False
    self.AutoACCCarAway = False
    self.AutoACCGreenLight = False
    #########################################
    self.update_frogpilot_params(params, params_memory)
    self.params_memory = params_memory
    self.params = params
  def update(self, carState, controlsState, modelData, radarState, mpc, sm, v_cruise, v_ego):
#########################################
    # self.params = Params
    # self.params_memory = Params("/dev/shm/params")
    carState, controlsState, modelData, radarState = sm['carState'], sm['controlsState'], sm['modelV2'], sm['radarState']
#########################################
    enabled = controlsState.enabled

    road_curvature = FrogPilotFunctions.road_curvature(modelData, v_ego)

    # Acceleration profiles
    v_cruise_changed = (self.mtsc_target or self.vtsc_target) + 1 < v_cruise  # Use stock acceleration profiles to handle MTSC/VTSC more precisely
    if v_cruise_changed:
      self.accel_limits = [A_CRUISE_MIN, get_max_accel(v_ego)]
    elif self.acceleration_profile == 1:
      self.accel_limits = [FrogPilotFunctions.get_min_accel_eco(v_ego), FrogPilotFunctions.get_max_accel_eco(v_ego)]
    elif self.acceleration_profile in (2, 3):
      self.accel_limits = [FrogPilotFunctions.get_min_accel_sport(v_ego), FrogPilotFunctions.get_max_accel_sport(v_ego)]
    else:
      self.accel_limits = [A_CRUISE_MIN, get_max_accel(v_ego)]

    # Conditional Experimental Mode
    if self.conditional_experimental_mode and enabled:
      self.cem.update(carState, sm['frogpilotNavigation'], modelData, mpc, sm['radarState'], road_curvature, carState.standstill, v_ego)

    if v_ego > MIN_TARGET_V:
      self.v_cruise = self.update_v_cruise(carState, controlsState, modelData, enabled, v_cruise, v_ego)
    else:
      self.mtsc_target = v_cruise
      self.vtsc_target = v_cruise
      self.v_cruise = v_cruise

    # Lane detection
    check_lane_width = self.adjacent_lanes or self.blind_spot_path or self.lane_detection
    if check_lane_width and v_ego >= LANE_CHANGE_SPEED_MIN:
      self.lane_width_left = float(FrogPilotFunctions.calculate_lane_width(modelData.laneLines[0], modelData.laneLines[1], modelData.roadEdges[0]))
      self.lane_width_right = float(FrogPilotFunctions.calculate_lane_width(modelData.laneLines[3], modelData.laneLines[2], modelData.roadEdges[1]))
    else:
      self.lane_width_left = 0
      self.lane_width_right = 0
################################################################################
    #定義
    have_lead = radarState.leadOne.status
    v_ego_kph = v_ego * 3.6
    detect_sl = SpeedLimitController.desired_speed_limit * 3.6
    speedlimit = int(self.params_memory.get_int('DetectSpeedLimit')*1.1)

    Auto_ACC = self.params.get_bool("AutoACC")
    AutoACCspeed = self.params.get_int("AutoACCspeed")
    Auto_ACC_pass = v_ego_kph > AutoACCspeed
    AutoACCCarAway = self.AutoACCCarAway
    AutoACCGreenLight = self.AutoACCGreenLight

    AutoOffScreen = self.params.get_bool("AutoOffScreen")
    currentIsEngaged = self.params.get_bool("IsEngaged")
    ScreenBrightness = self.params.get_int("ScreenBrightness")
    ScreenBrightnesspre = self.params.get_int("ScreenBrightnesspre")

    CarAway = self.params.get_bool("CarAway")
    CarAway_speed = self.params.get_int("CarAwayspeed")
    CarAway_distance = self.params.get_int("CarAwaydistance")
    aheadspeed = self.params.get_int("leadspeeddiffProfile")
    aheaddis = self.params.get_int("leaddisProfile")
    CarAway_speedpass = (CarAway_speed == aheadspeed) or ((CarAway_speed+1 or +2) == aheadspeed) 
    CarAway_distancepass = (CarAway_distance == aheaddis) or ((CarAway_distance+1 or +2) == aheaddis)
    
    Roadtype = self.params.get_bool("Roadtype")
    Roadtype_Profile = self.params.get_int("RoadtypeProfile")
    
    Navspeed = self.params.get_bool("Navspeed")
    
    current_setspeed = self.params_memory.get_int("KeySetSpeed")

    #自動啟動ACC並帶入最高速限
    if AutoOffScreen:
      # if not currentIsEngaged:
      if ScreenBrightness != 0 :
        self.params.put_int("ScreenBrightnesspre",ScreenBrightness)
        self.params.put_int("ScreenBrightness", 0)
        self.params.put_bool("FrogPilotTogglesUpdated", True)
    else:
      if (ScreenBrightness == 0  and ScreenBrightnesspre !=0) :
        self.params.put_int("ScreenBrightness", ScreenBrightnesspre)
        self.params.put_bool("FrogPilotTogglesUpdated", True)
      
    if Auto_ACC :
      if not currentIsEngaged and (Auto_ACC_pass or AutoACCCarAway or AutoACCGreenLight) :
        self.params_memory.put_bool("KeyResume", True)
        self.params_memory.put_bool("KeyChanged", True)
        if self.params_memory.get_int("DetectSpeedLimit") != 0 and Roadtype_Profile != 0:
          if Navspeed  :
              self.params_memory.put_bool("SpeedLimitChanged", True)
        else:
          if Roadtype  :  
            if Roadtype_Profile == 1 and current_setspeed <60:
              self.params_memory.put_int("KeySetSpeed", 60)
              self.params_memory.put_bool("KeyChanged", True)
              self.params_memory.put_int("SpeedPrev",0)
            elif Roadtype_Profile == 2 and current_setspeed <90:
              self.params_memory.put_int("KeySetSpeed", 90)
              self.params_memory.put_bool("KeyChanged", True)
              self.params_memory.put_int("SpeedPrev",0)
            elif Roadtype_Profile == 3 and current_setspeed <120:
              self.params_memory.put_int("KeySetSpeed", 120)
              self.params_memory.put_bool("KeyChanged", True)
              self.params_memory.put_int("SpeedPrev",0)

    #前車遠離後自動帶入速度控制
    if CarAway:
      if v_ego_kph < 1 and have_lead and (CarAway_speedpass or CarAway_distancepass) :
        self.carawayck = True
        if self.params.get_bool("AutoACCCarAway"):
          self.AutoACCCarAway = True
        if self.params_memory.get_int("DetectSpeedLimit") !=0 :
          if Navspeed :
            self.params_memory.put_bool("SpeedLimitChanged", True)
        else:
          if Roadtype :
            if Roadtype_Profile == 1 and current_setspeed <60:
              self.params_memory.put_int("KeySetSpeed", 60)
              self.params_memory.put_bool("KeyChanged", True)
              self.params_memory.put_int("SpeedPrev",0)
            elif Roadtype_Profile == 2 and current_setspeed <90:
              self.params_memory.put_int("KeySetSpeed", 90)
              self.params_memory.put_bool("KeyChanged", True)
              self.params_memory.put_int("SpeedPrev",0)
            elif Roadtype_Profile == 3 and current_setspeed <120:
              self.params_memory.put_int("KeySetSpeed", 120)
              self.params_memory.put_bool("KeyChanged", True)
              self.params_memory.put_int("SpeedPrev",0)
      else:
        self.carawayck = False
        if self.params.get_bool("AutoACCCarAway"):
          self.AutoACCCarAway = False

    #綠燈帶入提醒與時速控制
    if self.params_memory.get_int("GreenLightReminderstatus") ==1:
      if self.params.get_bool("AutoACCGreenLight"):
        self.AutoACCGreenLight = True
      if self.params_memory.get_int("DetectSpeedLimit") != 0:
        if Navspeed  :
          self.params_memory.put_bool("SpeedLimitChanged", True)
      else:
        if Roadtype  :
          if Roadtype_Profile == 1 and current_setspeed < 60:
            self.params_memory.put_int("KeySetSpeed", 60)
            self.params_memory.put_bool("KeyChanged", True)
            self.params_memory.put_int("SpeedPrev", 0)
    else:
      if self.params.get_bool("AutoACCGreenLight"):
        self.AutoACCGreenLight = False
    #################################################################
    # 速限變更偵測
    if self.params.get_bool("Navspeed") :
      if detect_sl != self.detect_speed_prev and v_ego_kph > 5:    
        if detect_sl > 0:
          self.params_memory.put_int("DetectSpeedLimit", detect_sl)
          self.params_memory.put_bool("SpeedLimitChanged", True)
          self.slchangedu = True
          self.slchangedd = False        
          self.detect_speed_prev = detect_sl
        else:
          self.detect_speed_prev = 0
          self.params_memory.put_int("DetectSpeedLimit", 0 )
          self.slchangedd = True
          self.slchangedu = False
      else:
        self.params_memory.put_bool("SpeedLimitChanged", False)
        self.slchangedu = False
        self.slchangedd = False 
    #超速偵測
    if v_ego_kph >=40 and speedlimit >= 40 :
      if (v_ego_kph - speedlimit) >= 1:
        self.speedover = True
        if self.params_memory.get_int("DetectSpeedLimit") !=0 :
          if self.params.get_bool("speedreminderreset") :
            if self.params_memory.get_int("DetectSpeedLimit") <40:
              self.params_memory.put_int("DetectSpeedLimit",40)
            else:
              self.params_memory.put_bool("SpeedLimitChanged", True)
      else:
        self.speedover = False
    elif v_ego_kph <40 :
      self.speedover = False
      
####################################################################################
  def update_v_cruise(self, carState, controlsState, modelData, enabled, v_cruise, v_ego):
    # Pfeiferj's Map Turn Speed Controller
    if self.map_turn_speed_controller:
      self.mtsc_target = np.clip(self.mtsc.target_speed(v_ego, carState.aEgo), MIN_TARGET_V, v_cruise)
    else:
      self.mtsc_target = v_cruise

    # Pfeiferj's Speed Limit Controller
    if self.speed_limit_controller:
      SpeedLimitController.update_current_max_velocity(v_cruise)
      self.slc_target = SpeedLimitController.desired_speed_limit

      # Override SLC upon gas pedal press and reset upon brake/cancel button
      if self.speed_limit_controller_override:
        self.override_slc |= carState.gasPressed
        self.override_slc &= enabled
        self.override_slc &= v_ego > self.slc_target
      else:
        self.override_slc = False

      self.overridden_speed *= enabled

      # Use the override speed if SLC is being overridden
      if self.override_slc:
        if self.speed_limit_controller_override == 1:
          # Set the max speed to the manual set speed
          if carState.gasPressed:
#########################################
            # self.overridden_speed = np.clip(v_ego, self.slc_target, v_cruise)
            self.overridden_speed = v_cruise
#########################################
          self.slc_target = self.overridden_speed
        elif self.speed_limit_controller_override == 2:
          self.overridden_speed = v_cruise
          self.slc_target = v_cruise
      if self.slc_target == 0:
        self.slc_target = v_cruise
    else:
      self.overriden_speed = 0
      self.slc_target = v_cruise

    # Pfeiferj's Vision Turn Controller
    if self.vision_turn_controller:
      # Set the curve sensitivity
      orientation_rate = np.array(np.abs(modelData.orientationRate.z)) * self.curve_sensitivity
      velocity = np.array(modelData.velocity.x)

      # Get the maximum lat accel from the model
      max_pred_lat_acc = np.amax(orientation_rate * velocity)

      # Get the maximum curve based on the current velocity
      max_curve = max_pred_lat_acc / (v_ego**2)

      # Set the target lateral acceleration
      adjusted_target_lat_a = TARGET_LAT_A * self.turn_aggressiveness

      # Get the target velocity for the maximum curve
      self.vtsc_target = (adjusted_target_lat_a / max_curve)**0.5
      self.vtsc_target = np.clip(self.vtsc_target, MIN_TARGET_V, v_cruise)
    else:
      self.vtsc_target = v_cruise

    v_ego_diff = max(carState.vEgoRaw - carState.vEgoCluster, 0)
    return min(v_cruise, self.mtsc_target, self.slc_target, self.vtsc_target) - v_ego_diff

  def publish(self, sm, pm, mpc):
    frogpilot_plan_send = messaging.new_message('frogpilotPlan')
    frogpilot_plan_send.valid = sm.all_checks(service_list=['carState', 'controlsState'])
    frogpilotPlan = frogpilot_plan_send.frogpilotPlan

    frogpilotPlan.adjustedCruise = float(min(self.mtsc_target, self.vtsc_target) * (CV.MS_TO_KPH if self.is_metric else CV.MS_TO_MPH))
    frogpilotPlan.conditionalExperimental = self.cem.experimental_mode

    frogpilotPlan.desiredFollowDistance = mpc.safe_obstacle_distance - mpc.stopped_equivalence_factor
    frogpilotPlan.safeObstacleDistance = mpc.safe_obstacle_distance
    frogpilotPlan.safeObstacleDistanceStock = mpc.safe_obstacle_distance_stock
    frogpilotPlan.stoppedEquivalenceFactor = mpc.stopped_equivalence_factor

    frogpilotPlan.laneWidthLeft = self.lane_width_left
    frogpilotPlan.laneWidthRight = self.lane_width_right

    frogpilotPlan.redLight = self.cem.red_light_detected

    frogpilotPlan.slcOverridden = self.override_slc
    frogpilotPlan.slcOverriddenSpeed = float(self.overridden_speed)
    frogpilotPlan.slcSpeedLimit = float(self.slc_target)
    frogpilotPlan.slcSpeedLimitOffset = SpeedLimitController.offset

    frogpilotPlan.vtscControllingCurve = bool(self.mtsc_target > self.vtsc_target)
    #######################################################
    frogpilotPlan.dspeedlimitu = self.slchangedu
    frogpilotPlan.dspeedlimitd = self.slchangedd
    frogpilotPlan.speedover = self.speedover 
    frogpilotPlan.carawayck = self.carawayck
    ########################################################
    pm.send('frogpilotPlan', frogpilot_plan_send)

  def update_frogpilot_params(self, params, params_memory):
    self.is_metric = params.get_bool("IsMetric")

    self.conditional_experimental_mode = params.get_bool("ConditionalExperimental")
    if self.conditional_experimental_mode:
      self.cem.update_frogpilot_params(self.is_metric, params)
      params.put_bool("ExperimentalMode", True)

    self.custom_personalities = params.get_bool("CustomPersonalities")
    self.aggressive_follow = params.get_int("AggressiveFollow") / 10
    self.standard_follow = params.get_int("StandardFollow") / 10
    self.relaxed_follow = params.get_int("RelaxedFollow") / 10
    self.aggressive_jerk = params.get_int("AggressiveJerk") / 10
    self.standard_jerk = params.get_int("StandardJerk") / 10
    self.relaxed_jerk = params.get_int("RelaxedJerk") / 10

    custom_ui = params.get_bool("CustomUI")
    self.adjacent_lanes = params.get_bool("AdjacentPath") and custom_ui
    self.blind_spot_path = params.get_bool("BlindSpotPath") and custom_ui

    lateral_tune = params.get_bool("LateralTune")

    self.lane_detection = params.get_bool("LaneDetection") and params.get_bool("NudgelessLaneChange")

    longitudinal_tune = params.get_bool("LongitudinalTune")
    self.acceleration_profile = params.get_int("AccelerationProfile") if longitudinal_tune else 0
    self.aggressive_acceleration = params.get_bool("AggressiveAcceleration") and longitudinal_tune
    self.increased_stopping_distance = params.get_int("StoppingDistance") * (1 if self.is_metric else CV.FOOT_TO_METER) if longitudinal_tune else 0
    self.smoother_braking = params.get_bool("SmoothBraking") and longitudinal_tune

    self.map_turn_speed_controller = params.get_bool("MTSCEnabled")
    if self.map_turn_speed_controller:
      params_memory.put_float("MapTargetLatA", 2 * (params.get_int("MTSCAggressiveness") / 100))

    self.speed_limit_controller = params.get_bool("SpeedLimitController")
    if self.speed_limit_controller:
      self.speed_limit_controller_override = params.get_int("SLCOverride")
      SpeedLimitController.update_frogpilot_params()

    self.vision_turn_controller = params.get_bool("VisionTurnControl")
    if self.vision_turn_controller:
      self.curve_sensitivity = params.get_int("CurveSensitivity") / 100
      self.turn_aggressiveness = params.get_int("TurnAggressiveness") / 100

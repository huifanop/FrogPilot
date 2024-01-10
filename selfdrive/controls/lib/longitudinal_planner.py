#!/usr/bin/env python3
import math
import numpy as np
from openpilot.common.numpy_fast import clip, interp
from openpilot.common.params import Params
from cereal import log

import cereal.messaging as messaging
from openpilot.common.conversions import Conversions as CV
from openpilot.common.filter_simple import FirstOrderFilter
from openpilot.common.realtime import DT_MDL
from openpilot.selfdrive.modeld.constants import ModelConstants
from openpilot.selfdrive.car.interfaces import ACCEL_MIN, ACCEL_MAX
from openpilot.selfdrive.controls.lib.longcontrol import LongCtrlState
from openpilot.selfdrive.controls.lib.longitudinal_mpc_lib.long_mpc import LongitudinalMpc
from openpilot.selfdrive.controls.lib.longitudinal_mpc_lib.long_mpc import T_IDXS as T_IDXS_MPC
from openpilot.selfdrive.controls.lib.drive_helpers import V_CRUISE_MAX, CONTROL_N, get_speed_error
from openpilot.common.swaglog import cloudlog

from openpilot.selfdrive.frogpilot.functions.conditional_experimental_mode import ConditionalExperimentalMode
from openpilot.selfdrive.frogpilot.functions.map_turn_speed_controller import MapTurnSpeedController
from openpilot.selfdrive.frogpilot.functions.speed_limit_controller import SpeedLimitController

LON_MPC_STEP = 0.2  # first step is 0.2s
A_CRUISE_MIN = -1.2
A_CRUISE_MAX_VALS = [1.6, 1.2, 0.8, 0.6]
A_CRUISE_MAX_BP = [0., 10.0, 25., 40.]

# Acceleration profiles - Credit goes to the DragonPilot team!
                 # MPH = [0.,  35,   35,  40,    40,  45,    45,  67,    67,   67, 123]
A_CRUISE_MIN_BP_CUSTOM = [0., 2.0, 2.01, 11., 11.01, 18., 18.01, 28., 28.01,  33., 55.]
                 # MPH = [0., 6.71, 13.4, 17.9, 24.6, 33.6, 44.7, 55.9, 67.1, 123]
A_CRUISE_MAX_BP_CUSTOM = [0.,    3,   6.,   8.,  11.,  15.,  20.,  25.,  30., 55.]

A_CRUISE_MIN_VALS_ECO_TUNE = [-0.480, -0.480, -0.40, -0.40, -0.40, -0.36, -0.32, -0.28, -0.28, -0.25, -0.25]
A_CRUISE_MAX_VALS_ECO_TUNE = [3.5, 3.3, 1.7, 1.1, .76, .62, .47, .36, .28, .09]

A_CRUISE_MIN_VALS_SPORT_TUNE = [-0.500, -0.500, -0.42, -0.42, -0.42, -0.42, -0.40, -0.35, -0.35, -0.30, -0.30]
A_CRUISE_MAX_VALS_SPORT_TUNE = [3.5, 3.5, 3.0, 2.6, 1.4, 1.0, 0.7, 0.6, .38, .2]

# Lookup table for turns
_A_TOTAL_MAX_V = [1.7, 3.2]
_A_TOTAL_MAX_BP = [20., 40.]

# VTSC variables
TARGET_LAT_A = 1.9  # m/s^2


def get_max_accel(v_ego):
  return interp(v_ego, A_CRUISE_MAX_BP, A_CRUISE_MAX_VALS)

def get_min_accel_eco_tune(v_ego):
  return interp(v_ego, A_CRUISE_MIN_BP_CUSTOM, A_CRUISE_MIN_VALS_ECO_TUNE)

def get_max_accel_eco_tune(v_ego):
  return interp(v_ego, A_CRUISE_MAX_BP_CUSTOM, A_CRUISE_MAX_VALS_ECO_TUNE)

def get_min_accel_sport_tune(v_ego):
  return interp(v_ego, A_CRUISE_MIN_BP_CUSTOM, A_CRUISE_MIN_VALS_SPORT_TUNE)

def get_max_accel_sport_tune(v_ego):
  return interp(v_ego, A_CRUISE_MAX_BP_CUSTOM, A_CRUISE_MAX_VALS_SPORT_TUNE)

def limit_accel_in_turns(v_ego, angle_steers, a_target, CP):
  """
  This function returns a limited long acceleration allowed, depending on the existing lateral acceleration
  this should avoid accelerating when losing the target in turns
  """

  # FIXME: This function to calculate lateral accel is incorrect and should use the VehicleModel
  # The lookup table for turns should also be updated if we do this
  a_total_max = interp(v_ego, _A_TOTAL_MAX_BP, _A_TOTAL_MAX_V)
  a_y = v_ego ** 2 * angle_steers * CV.DEG_TO_RAD / (CP.steerRatio * CP.wheelbase)
  a_x_allowed = math.sqrt(max(a_total_max ** 2 - a_y ** 2, 0.))

  return [a_target[0], min(a_target[1], a_x_allowed)]


class LongitudinalPlanner:
  def __init__(self, CP, init_v=0.0, init_a=0.0):
    self.CP = CP
    self.mpc = LongitudinalMpc()
    self.fcw = False

    self.a_desired = init_a
    self.v_desired_filter = FirstOrderFilter(init_v, 2.0, DT_MDL)
    self.v_model_error = 0.0

    self.x_desired_trajectory = np.zeros(CONTROL_N)
    self.v_desired_trajectory = np.zeros(CONTROL_N)
    self.a_desired_trajectory = np.zeros(CONTROL_N)
    self.j_desired_trajectory = np.zeros(CONTROL_N)
    self.solverExecutionTime = 0.0
    self.params = Params()
    ##############################
    self.params_memory = Params("/dev/shm/params")
    #############################
    self.param_read_counter = 0
    self.read_param()
    self.personality = log.LongitudinalPersonality.standard
    #########################################
    self.carawayck = False
    self.detect_speed_prev = 0
    self.slchangedu = False
    self.slchangedd = False
    self.lead_emer_count = 0
    self.lead_emeroff_count = 0
    self.detect_drel_count =0
    self.previous_lead_distance = 0
    self.speedover = False
    self.AutoACCspeed = False
    self.AutoACCCarAway = False
    self.AutoACCGreenLight = False
    #########################################

    # FrogPilot variables
    self.params_memory = Params("/dev/shm/params")

    self.green_light = False
    self.override_slc = False
    self.previously_driving = False
    self.stopped_for_light_previously = False

    self.overridden_speed = 0
    self.mtsc_target = 0
    self.slc_target = 0
    self.vtsc_target = 0

  def read_param(self):
    try:
      self.personality = int(self.params.get('LongitudinalPersonality'))
    except (ValueError, TypeError):
      self.personality = log.LongitudinalPersonality.standard

  @staticmethod
  def parse_model(model_msg, model_error):
    if (len(model_msg.position.x) == 33 and
       len(model_msg.velocity.x) == 33 and
       len(model_msg.acceleration.x) == 33):
      x = np.interp(T_IDXS_MPC, ModelConstants.T_IDXS, model_msg.position.x) - model_error * T_IDXS_MPC
      v = np.interp(T_IDXS_MPC, ModelConstants.T_IDXS, model_msg.velocity.x) - model_error
      a = np.interp(T_IDXS_MPC, ModelConstants.T_IDXS, model_msg.acceleration.x)
      j = np.zeros(len(T_IDXS_MPC))
    else:
      x = np.zeros(len(T_IDXS_MPC))
      v = np.zeros(len(T_IDXS_MPC))
      a = np.zeros(len(T_IDXS_MPC))
      j = np.zeros(len(T_IDXS_MPC))
    return x, v, a, j

  def update(self, sm):
    if self.param_read_counter % 50 == 0:
      self.read_param()
    self.param_read_counter += 1
    self.mpc.mode = 'blended' if sm['controlsState'].experimentalMode else 'acc'

    v_ego = sm['carState'].vEgo
    v_cruise_kph = min(sm['controlsState'].vCruise, V_CRUISE_MAX)
    v_cruise = v_cruise_kph * CV.KPH_TO_MS

    long_control_off = sm['controlsState'].longControlState == LongCtrlState.off
    force_slow_decel = sm['controlsState'].forceDecel

    # Reset current state when not engaged, or user is controlling the speed
    reset_state = long_control_off if self.CP.openpilotLongitudinalControl else not sm['controlsState'].enabled

    # No change cost when user is controlling the speed, or when standstill
    prev_accel_constraint = not (reset_state or sm['carState'].standstill)

    if self.mpc.mode == 'acc':
      # Use stock acceleration profiles to handle MTSC/VTSC more precisely
      # v_cruise_changed = (self.mtsc_target or self.vtsc_target) != v_cruise
      # if v_cruise_changed:
        # accel_limits = [A_CRUISE_MIN, get_max_accel(v_ego)]
      if self.acceleration_profile == 1:
        accel_limits = [get_min_accel_eco_tune(v_ego), get_max_accel_eco_tune(v_ego)]
      elif self.acceleration_profile == 3:
        accel_limits = [get_min_accel_sport_tune(v_ego), get_max_accel_sport_tune(v_ego)]
      else:
        accel_limits = [A_CRUISE_MIN, get_max_accel(v_ego)]
      accel_limits_turns = limit_accel_in_turns(v_ego, sm['carState'].steeringAngleDeg, accel_limits, self.CP)
    else:
      accel_limits = [ACCEL_MIN, ACCEL_MAX]
      accel_limits_turns = [ACCEL_MIN, ACCEL_MAX]

    if reset_state:
      self.v_desired_filter.x = v_ego
      # Clip aEgo to cruise limits to prevent large accelerations when becoming active
      self.a_desired = clip(sm['carState'].aEgo, accel_limits[0], accel_limits[1])

    # Prevent divergence, smooth in current v_ego
    self.v_desired_filter.x = max(0.0, self.v_desired_filter.update(v_ego))
    # Compute model v_ego error
    self.v_model_error = get_speed_error(sm['modelV2'], v_ego)

    if force_slow_decel:
      v_cruise = 0.0
    # clip limits, cannot init MPC outside of bounds
    accel_limits_turns[0] = min(accel_limits_turns[0], self.a_desired + 0.05)
    accel_limits_turns[1] = max(accel_limits_turns[1], self.a_desired - 0.05)

    # FrogPilot variables
    carState, controlsState, modelData, radarState = sm['carState'], sm['controlsState'], sm['modelV2'], sm['radarState']
    enabled = controlsState.enabled
    have_lead = radarState.leadOne.status
    standstill = carState.standstill
    v_lead = radarState.leadOne.vLead

    self.previously_driving |= not standstill and enabled
    self.previously_driving &= sm['frogpilotCarControl'].drivingGear

################################################################################
    #定義
    v_ego_kph = v_ego * 3.6
    detect_sl = SpeedLimitController.desired_speed_limit * 3.6
    speedlimit = int(self.params_memory.get_int('DetectSpeedLimit')*1.1)

    Auto_ACC = self.params.get_bool("AutoACC")
    AutoACCspeed = self.params.get_int('AutoACCspeed')
    Auto_ACC_pass = v_ego_kph > AutoACCspeed
    AutoACCCarAway = self.AutoACCCarAway
    AutoACCGreenLight = self.AutoACCGreenLight
    
    CarAway = self.params.get_bool("CarAway")
    CarAway_speed = self.params.get_int('CarAwayspeed')
    CarAway_distance = self.params.get_int('CarAwaydistance')
    aheadspeed = self.params.get_int("leadspeeddiffProfile")
    aheaddis = self.params.get_int("leaddisProfile")
    CarAway_speedpass = (CarAway_speed == aheadspeed) or ((CarAway_speed+1 or +2) == aheadspeed) 
    CarAway_distancepass = (CarAway_distance == aheaddis) or ((CarAway_distance+1 or +2) == aheaddis)
    
    Roadtype = self.params.get_bool('Roadtype')
    Roadtype_Profile = self.params.get_int("RoadtypeProfile")
    
    Speed_distance = self.params.get_bool("Speeddistance")
    Navspeed = self.params.get_bool('Navspeed')
    
    current_setspeed = self.params_memory.get_int('KeySetSpeed')
    SpeedLimitChangedck = self.params_memory.get_bool('SpeedLimitChanged')
    KeyChangedck = self.params_memory.get_bool('KeyChanged')

    #自動啟動ACC並帶入最高速限
    if Auto_ACC :
      if not self.params.get_bool('IsEngaged') and (Auto_ACC_pass or AutoACCCarAway or AutoACCGreenLight) :
        self.params_memory.put_bool('KeyResume', True)
        self.params_memory.put_bool('KeyChanged', True)
        if self.params_memory.get_int('DetectSpeedLimit') != 0 and Roadtype_Profile != 0:
          if Navspeed  :
              self.params_memory.put_bool('SpeedLimitChanged', True)
        else:
          if Roadtype  :  
            if Roadtype_Profile == 1:
              self.params_memory.put_int('KeySetSpeed', 60)
              self.params_memory.put_bool('KeyChanged', True)
              self.params_memory.put_int('SpeedPrev',0)
            elif Roadtype_Profile == 2:
              self.params_memory.put_int('KeySetSpeed', 90)
              self.params_memory.put_bool('KeyChanged', True)
              self.params_memory.put_int('SpeedPrev',0)
            elif Roadtype_Profile == 3:
              self.params_memory.put_int('KeySetSpeed', 120)
              self.params_memory.put_bool('KeyChanged', True)
              self.params_memory.put_int('SpeedPrev',0)

    #前車遠離後自動帶入速度控制
    if CarAway:
      if v_ego_kph < 1 and have_lead and (CarAway_speedpass and CarAway_distancepass) :
        self.carawayck = True
        if self.params.get_bool("AutoACCCarAway"):
          self.AutoACCCarAway = True
        if self.params_memory.get_int('DetectSpeedLimit') !=0 :
          if Navspeed :
            self.params_memory.put_bool('SpeedLimitChanged', True)
        else:
          if Roadtype :
            if Roadtype_Profile == 1:
              self.params_memory.put_int('KeySetSpeed', 60)
              self.params_memory.put_bool('KeyChanged', True)
              self.params_memory.put_int('SpeedPrev',0)
            elif Roadtype_Profile == 2:
              self.params_memory.put_int('KeySetSpeed', 90)
              self.params_memory.put_bool('KeyChanged', True)
              self.params_memory.put_int('SpeedPrev',0)
            elif Roadtype_Profile == 3:
              self.params_memory.put_int('KeySetSpeed', 120)
              self.params_memory.put_bool('KeyChanged', True)
              self.params_memory.put_int('SpeedPrev',0)
      else:
        self.carawayck = False
        if self.params.get_bool("AutoACCCarAway"):
          self.AutoACCCarAway = False

    #速度調控車距
    LongitudinalPersonalityck = self.params.get_int("LongitudinalPersonality")
    if Speed_distance :
      if SpeedLimitChangedck==True or KeyChangedck==True:
        if  v_ego_kph < 60:
          if LongitudinalPersonalityck != 0 :
            self.params.put_int("LongitudinalPersonality", 0)
            self.params_memory.put_bool("FrogPilotTogglesUpdated", True)
        elif v_ego_kph == 60 and v_ego_kph < 90:
          if LongitudinalPersonalityck != 1 :
            self.params.put_int("LongitudinalPersonality",1)
            self.params_memory.put_bool("FrogPilotTogglesUpdated", True)
        elif v_ego_kph == 90 and v_ego_kph < 120:
          if LongitudinalPersonalityck != 2 :
              self.params.put_int("LongitudinalPersonality",2)
              self.params_memory.put_bool("FrogPilotTogglesUpdated", True)
    
    #綠燈帶入提醒與時速控制
    if self.green_light:
      if self.params.get_bool("AutoACCGreenLight"):
        self.AutoACCGreenLight = True
      if self.params_memory.get_int('DetectSpeedLimit') != 0:
        if Navspeed  :
          self.params_memory.put_bool('SpeedLimitChanged', True)
      else:
        if Roadtype  :
          if Roadtype_Profile == 1 and current_setspeed < 60:
            self.params_memory.put_int('KeySetSpeed', 60)
            self.params_memory.put_bool('KeyChanged', True)
            self.params_memory.put_int('SpeedPrev', 0)
    else:
      if self.params.get_bool("AutoACCGreenLight"):
        self.AutoACCGreenLight = False
    #################################################################
    # 速限變更偵測
    if self.params.get_bool("Navspeed") :
      if detect_sl != self.detect_speed_prev and v_ego_kph > 5:    
        if detect_sl > 0:
          self.params_memory.put_int('DetectSpeedLimit', detect_sl)
          self.params_memory.put_bool('SpeedLimitChanged', True)
          self.slchangedu = True
          self.slchangedd = False        
          self.detect_speed_prev = detect_sl
        else:
          self.detect_speed_prev = 0
          self.params_memory.put_int('DetectSpeedLimit', 0 )
          self.slchangedd = True
          self.slchangedu = False
      else:
        self.params_memory.put_bool('SpeedLimitChanged', False)
        self.slchangedu = False
        self.slchangedd = False 
    #超速偵測
    if v_ego_kph >=40 and speedlimit >= 40 :
      if (v_ego_kph - speedlimit) >= 1:
        self.speedover = True
        if self.params_memory.get_int('DetectSpeedLimit') !=0 :
          if self.params.get_bool("speedreminderreset") :
            if self.params_memory.get_int('DetectSpeedLimit') <40:
              self.params_memory.put_int('DetectSpeedLimit',40)
            else:
              self.params_memory.put_bool('SpeedLimitChanged', True)
      else:
        self.speedover = False
    elif v_ego_kph <40 :
      self.speedover = False
      
####################################################################################

    # Conditional Experimental Mode
    if self.conditional_experimental_mode and self.previously_driving:
      ConditionalExperimentalMode.update(carState, sm['frogpilotNavigation'], modelData, radarState, v_ego, v_lead, self.mtsc_target, self.vtsc_target)

    # Green light alert
    if self.green_light_alert and self.previously_driving:
      stopped_for_light = ConditionalExperimentalMode.stop_sign_and_light(carState, False, 0, modelData, v_ego, 0) and carState.standstill

      self.green_light = not stopped_for_light and self.stopped_for_light_previously and not carState.gasPressed

      self.stopped_for_light_previously = stopped_for_light

    # Update v_cruise for speed limiter functions
    v_ego_cluster = carState.vEgoCluster
    v_ego_raw = carState.vEgoRaw
    v_ego_diff = v_ego_raw - v_ego_cluster if v_ego_cluster > 0 else 0
    v_cruise += v_ego_diff
    v_cruise = self.v_cruise_update(carState, enabled, modelData, v_cruise, v_ego)

    self.mpc.set_weights(prev_accel_constraint, self.custom_personalities, self.aggressive_jerk, self.standard_jerk, self.relaxed_jerk, personality=self.personality)
    self.mpc.set_accel_limits(accel_limits_turns[0], accel_limits_turns[1])
    self.mpc.set_cur_state(self.v_desired_filter.x, self.a_desired)
    x, v, a, j = self.parse_model(sm['modelV2'], self.v_model_error)
    self.mpc.update(sm['radarState'], v_cruise, x, v, a, j, have_lead, self.aggressive_acceleration, self.increased_stopping_distance, self.smoother_braking,
                    self.custom_personalities, self.aggressive_follow, self.standard_follow, self.relaxed_follow, personality=self.personality)

    self.x_desired_trajectory_full = np.interp(ModelConstants.T_IDXS, T_IDXS_MPC, self.mpc.x_solution)
    self.v_desired_trajectory_full = np.interp(ModelConstants.T_IDXS, T_IDXS_MPC, self.mpc.v_solution)
    self.a_desired_trajectory_full = np.interp(ModelConstants.T_IDXS, T_IDXS_MPC, self.mpc.a_solution)
    self.x_desired_trajectory = self.x_desired_trajectory_full[:CONTROL_N]
    self.v_desired_trajectory = self.v_desired_trajectory_full[:CONTROL_N]
    self.a_desired_trajectory = self.a_desired_trajectory_full[:CONTROL_N]
    self.j_desired_trajectory = np.interp(ModelConstants.T_IDXS[:CONTROL_N], T_IDXS_MPC[:-1], self.mpc.j_solution)

    # TODO counter is only needed because radar is glitchy, remove once radar is gone
    self.fcw = self.mpc.crash_cnt > 2 and not sm['carState'].standstill
    if self.fcw:
      cloudlog.info("FCW triggered")

    # Interpolate 0.05 seconds and save as starting point for next iteration
    a_prev = self.a_desired
    self.a_desired = float(interp(DT_MDL, ModelConstants.T_IDXS[:CONTROL_N], self.a_desired_trajectory))
    self.v_desired_filter.x = self.v_desired_filter.x + DT_MDL * (self.a_desired + a_prev) / 2.0

  def publish(self, sm, pm):
    plan_send = messaging.new_message('longitudinalPlan')

    plan_send.valid = sm.all_checks(service_list=['carState', 'controlsState'])

    longitudinalPlan = plan_send.longitudinalPlan
    longitudinalPlan.modelMonoTime = sm.logMonoTime['modelV2']
    longitudinalPlan.processingDelay = (plan_send.logMonoTime / 1e9) - sm.logMonoTime['modelV2']

    longitudinalPlan.speeds = self.v_desired_trajectory.tolist()
    longitudinalPlan.accels = self.a_desired_trajectory.tolist()
    longitudinalPlan.jerks = self.j_desired_trajectory.tolist()

    longitudinalPlan.hasLead = sm['radarState'].leadOne.status
    longitudinalPlan.longitudinalPlanSource = self.mpc.source
    longitudinalPlan.fcw = self.fcw

    longitudinalPlan.solverExecutionTime = self.mpc.solve_time
    longitudinalPlan.personality = self.personality

    pm.send('longitudinalPlan', plan_send)

    # FrogPilot longitudinalPlan variables
    frogpilot_plan_send = messaging.new_message('frogpilotLongitudinalPlan')
    frogpilot_plan_send.valid = sm.all_checks(service_list=['carState', 'controlsState'])
    frogpilotLongitudinalPlan = frogpilot_plan_send.frogpilotLongitudinalPlan

    frogpilotLongitudinalPlan.adjustedCruise = float(min(self.mtsc_target, self.vtsc_target)) * (CV.MS_TO_KPH if self.is_metric else CV.MS_TO_MPH)

    frogpilotLongitudinalPlan.conditionalExperimental = ConditionalExperimentalMode.experimental_mode
    frogpilotLongitudinalPlan.distances = self.x_desired_trajectory.tolist()
    frogpilotLongitudinalPlan.greenLight = bool(self.green_light)

    frogpilotLongitudinalPlan.slcOverridden = self.override_slc
    frogpilotLongitudinalPlan.slcOverriddenSpeed = float(self.overridden_speed)
    frogpilotLongitudinalPlan.slcSpeedLimit = float(self.slc_target)
    frogpilotLongitudinalPlan.slcSpeedLimitOffset = SpeedLimitController.offset

    frogpilotLongitudinalPlan.safeObstacleDistance = self.mpc.safe_obstacle_distance
    frogpilotLongitudinalPlan.stoppedEquivalenceFactor = self.mpc.stopped_equivalence_factor
    frogpilotLongitudinalPlan.desiredFollowDistance = self.mpc.safe_obstacle_distance - self.mpc.stopped_equivalence_factor
    frogpilotLongitudinalPlan.safeObstacleDistanceStock = self.mpc.safe_obstacle_distance_stock
    frogpilotLongitudinalPlan.stoppedEquivalenceFactorStock = self.mpc.stopped_equivalence_factor_stock

    #######################################################
    frogpilotLongitudinalPlan.dspeedlimitu = self.slchangedu
    frogpilotLongitudinalPlan.dspeedlimitd = self.slchangedd
    frogpilotLongitudinalPlan.speedover = self.speedover 
    frogpilotLongitudinalPlan.carawayck = self.carawayck
    ########################################################

    pm.send('frogpilotLongitudinalPlan', frogpilot_plan_send)

  def v_cruise_update(self, carState, enabled, modelData, v_cruise, v_ego):
    # Pfeiferj's Map Turn Speed Controller
    if self.map_turn_speed_controller:
      self.mtsc_target = np.clip(MapTurnSpeedController.target_speed(v_ego, carState.aEgo), 0, v_cruise)
      if self.mtsc_target == 0:
        self.mtsc_target = v_cruise
    else:
      self.mtsc_target = v_cruise

    # Pfeiferj's Speed Limit Controller
    if self.speed_limit_controller:
      SpeedLimitController.update_current_max_velocity(v_cruise)
      self.slc_target = SpeedLimitController.desired_speed_limit

      # Override SLC upon gas pedal press and reset upon brake/cancel button
      self.override_slc |= carState.gasPressed
      self.override_slc &= enabled
      self.override_slc &= v_ego > self.slc_target

      # Set the max speed to the manual set speed
      if carState.gasPressed:
        # self.overridden_speed = np.clip(v_ego, self.slc_target, v_cruise)
        self.overridden_speed = v_cruise
      self.overridden_speed *= enabled

      # Use the override speed if SLC is being overridden
      if self.override_slc:
        self.slc_target = self.overridden_speed

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
      self.vtsc_target = (adjusted_target_lat_a / max_curve) ** 0.5
      self.vtsc_target = np.clip(self.vtsc_target, 0, v_cruise)
      if self.vtsc_target == 0:
        self.vtsc_target = v_cruise
    else:
      self.vtsc_target = v_cruise

    return min(v_cruise, self.mtsc_target, self.slc_target, self.vtsc_target)

  def update_frogpilot_params(self):
    self.is_metric = self.params.get_bool("IsMetric")

    self.longitudinal_tune = self.params.get_bool("LongitudinalTune")
    self.acceleration_profile = self.params.get_int("AccelerationProfile") if self.longitudinal_tune else 2
    self.aggressive_acceleration = self.params.get_bool("AggressiveAcceleration") and self.longitudinal_tune
    self.increased_stopping_distance = self.params.get_int("StoppingDistance") * (1 if self.is_metric else CV.FOOT_TO_METER) if self.longitudinal_tune else 0
    self.smoother_braking = self.params.get_bool("SmoothBraking") and self.longitudinal_tune

    self.conditional_experimental_mode = self.params.get_bool("ConditionalExperimental")
    if self.conditional_experimental_mode:
      ConditionalExperimentalMode.update_frogpilot_params(self.is_metric)
      if not self.params.get_bool("ExperimentalMode"):
        self.params.put_bool("ExperimentalMode", True)

    self.custom_personalities = self.params.get_bool("CustomPersonalities")
    self.aggressive_follow = self.params.get_int("AggressiveFollow") / 10
    self.standard_follow = self.params.get_int("StandardFollow") / 10
    self.relaxed_follow = self.params.get_int("RelaxedFollow") / 10
    self.aggressive_jerk = self.params.get_int("AggressiveJerk") / 10
    self.standard_jerk = self.params.get_int("StandardJerk") / 10
    self.relaxed_jerk = self.params.get_int("RelaxedJerk") / 10

    self.green_light_alert = self.params.get_bool("GreenLightAlert")
    self.map_turn_speed_controller = self.params.get_bool("MTSCEnabled")

    self.speed_limit_controller = self.params.get_bool("SpeedLimitController")
    if self.speed_limit_controller:
      SpeedLimitController.update_frogpilot_params()

    self.vision_turn_controller = self.params.get_bool("VisionTurnControl")
    if self.vision_turn_controller:
      self.curve_sensitivity = self.params.get_int("CurveSensitivity") / 100
      self.turn_aggressiveness = self.params.get_int("TurnAggressiveness") / 100

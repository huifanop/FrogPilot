#!/usr/bin/bash

if [ -z "$BASEDIR" ]; then
  BASEDIR="/data/openpilot"
fi

source "$BASEDIR/launch_env.sh"

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null && pwd )"

function agnos_init {
  # wait longer for weston to come up
  if [ -f "$BASEDIR/prebuilt" ]; then
    sleep 5
  fi

  # TODO: move this to agnos
  sudo rm -f /data/etc/NetworkManager/system-connections/*.nmmeta

  # set success flag for current boot slot
  sudo abctl --set_success

  # Check if AGNOS update is required
  if [ $(< /VERSION) != "$AGNOS_VERSION" ]; then
    AGNOS_PY="$DIR/system/hardware/tici/agnos.py"
    MANIFEST="$DIR/system/hardware/tici/agnos.json"
    if $AGNOS_PY --verify $MANIFEST; then
      sudo reboot
    fi
    $DIR/system/hardware/tici/updater $AGNOS_PY $MANIFEST
  fi
}

function launch {
  # Remove orphaned git lock if it exists on boot
  [ -f "$DIR/.git/index.lock" ] && rm -f $DIR/.git/index.lock

  # Pull time from panda
  $DIR/selfdrive/boardd/set_time.py

  # Check to see if there's a valid overlay-based update available. Conditions
  # are as follows:
  #
  # 1. The BASEDIR init file has to exist, with a newer modtime than anything in
  #    the BASEDIR Git repo. This checks for local development work or the user
  #    switching branches/forks, which should not be overwritten.
  # 2. The FINALIZED consistent file has to exist, indicating there's an update
  #    that completed successfully and synced to disk.

  if [ -f "${BASEDIR}/.overlay_init" ]; then
    find ${BASEDIR}/.git -newer ${BASEDIR}/.overlay_init | grep -q '.' 2> /dev/null
    if [ $? -eq 0 ]; then
      echo "${BASEDIR} has been modified, skipping overlay update installation"
    else
      if [ -f "${STAGING_ROOT}/finalized/.overlay_consistent" ]; then
        if [ ! -d /data/safe_staging/old_openpilot ]; then
          echo "Valid overlay update found, installing"
          LAUNCHER_LOCATION="${BASH_SOURCE[0]}"

          mv $BASEDIR /data/safe_staging/old_openpilot
          mv "${STAGING_ROOT}/finalized" $BASEDIR
          cd $BASEDIR

          echo "Restarting launch script ${LAUNCHER_LOCATION}"
          unset AGNOS_VERSION
          exec "${LAUNCHER_LOCATION}"
        else
          echo "openpilot backup found, not updating"
          # TODO: restore backup? This means the updater didn't start after swapping
        fi
      fi
    fi
  fi

  # handle pythonpath
  ln -sfn $(pwd) /data/pythonpath
  export PYTHONPATH="$PWD"

  # hardware specific init
  agnos_init

  # write tmux scrollback to a file
  tmux capture-pane -pq -S-1000 > /tmp/launch_log

  # start manager
  # cd selfdrive/manager
  # ./build.py && ./manager.py
# ###################################################
#   if [ ! -d /data/media/0/log ]; then
#     mkdir -p /data/media/0/log/
#   fi
#   if [ ! -d /data/media/mapd ]; then
#     mkdir -p /data/media/mapd/
#   fi
#   cd selfdrive/manager
#   ./build.py && ./manager.py > /data/media/0/log/launch_log_$(date +"%Y%m%d_%H%M%S").txt
# ###################################################
  cd selfdrive/manager
  if [ ! -d /data/media/0/log ]; then
    mkdir -p /data/media/0/log/
  fi
  if [ ! -d /data/community/crashes ]; then
    mkdir -p /data/community/crashes/
  fi
  if [ ! -d /data/community/build ]; then
    mkdir -p /data/community/build/
  fi
  if [ -d "/data/community/build" ]; then
    if [ -d "/data/community/crashes" ]; then
      ./build.py > /data/community/build/build_log_$(date +"%Y%m%d_%H%M%S").txt && ./manager.py > /data/community/crashes/launch_log_$(date +"%Y%m%d_%H%M%S").txt
    else
      ./build.py && ./manager.py
    fi
  else
    if [ -d "/data/media/0/log" ]; then
      ./manager.py > /data/media/0/log/launch_log_$(date +"%Y%m%d_%H%M%S").txt
    else
      ./manager.py
    fi
  fi
###################################################

  # if broken, keep on screen error
  while true; do sleep 1; done
}

launch

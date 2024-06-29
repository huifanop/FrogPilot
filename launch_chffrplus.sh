#!/usr/bin/bash

if [ -z "$BASEDIR" ]; then
  BASEDIR="/data/openpilot"
fi

source "$BASEDIR/launch_env.sh"

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null && pwd )"

function agnos_init {
  # TODO: move this to agnos
  sudo rm -f /data/etc/NetworkManager/system-connections/*.nmmeta

  # set success flag for current boot slot
  sudo abctl --set_success

  # TODO: do this without udev in AGNOS
  # udev does this, but sometimes we startup faster
  sudo chgrp gpu /dev/adsprpc-smd /dev/ion /dev/kgsl-3d0
  sudo chmod 660 /dev/adsprpc-smd /dev/ion /dev/kgsl-3d0

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
  if [ -f /AGNOS ]; then
    agnos_init
  fi

  # write tmux scrollback to a file
  tmux capture-pane -pq -S-1000 > /tmp/launch_log

  # start manager
#  cd selfdrive/manager
#  if [ ! -f $DIR/prebuilt ]; then
#    ./build.py
#  fi
#  ./manager.py

###################################################
  cd selfdrive/manager
  if [ ! -f $DIR/prebuilt ]; then
    ./build.py
    if [ ! -d /data/community/crashes ]; then
      mkdir -p /data/community/crashes/
    fi
    if [ ! -d /data/community/build ]; then
      mkdir -p /data/community/build/
    fi
    if [ -d "/data/community/build" ]; then
      if [ -d "/data/community/crashes" ]; then
        ./build.py > /data/community/build/build_log_$(date +"%Y%m%d_%H%M%S").txt && ./manager.py > /data/community/crashes/fan$(date +"%m%d%H%M").txt
        
        # ./build.py > /data/community/build/build_log_$(date +"%Y%m%d_%H%M%S").txt && ./manager.py > /data/community/crashes/launch_log_$(date +"%Y%m%d_%H%M%S").txt
      else
        ./build.py && ./manager.py
      fi
    else
      if [ ! -d /data/media/0/log ]; then
        mkdir -p /data/media/0/log/
      fi
      if [ -d "/data/media/0/log" ]; then
        ./manager.py > /data/media/0/log/launch_log_$(date +"%Y%m%d_%H%M%S").txt
      else
        ./manager.py
      fi
    fi
  else
    ./manager.py
  fi
###################################################

  # if broken, keep on screen error
  while true; do sleep 1; done
}

launch

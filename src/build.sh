#!/bin/bash

if [ -f ~/.config/rerobapp/rerobapp.conf ]; then
   source ~/.config/rerobapp/rerobapp.conf
else
   echo "Loading rerobapp.conf local"
   source ../rerobapp.conf
fi

SRC_PATH="."

SSH_USER="$EXOTAO_USER_SSH"

rm -r "$SRC_DOCKER_PATH"

mkdir -p "$USER_DOCKER_PATH"
mkdir -p "$BUILD_DOCKER_PATH"
mkdir -p "$SRC_DOCKER_PATH"

rsync -aruh "$SRC_PATH/" "$SRC_DOCKER_PATH"

docker run -it --rm --user $(id -u):$(id -g) \
    --env="DISPLAY" \
    --env="QT_X11_NO_MITSHM=1" \
    --env="TERM=xterm-256color" \
    --network="host" \
    --ipc="host" \
    --privileged \
    --volume="/etc/group:/etc/group:ro" \
    --volume="/etc/passwd:/etc/passwd:ro" \
    --volume="/etc/shadow:/etc/shadow:ro" \
    --volume="/etc/sudoers.d:/etc/sudoers.d:ro" \
    --volume="/tmp/.X11-unix:/tmp/.X11-unix:rw" \
    --volume="$USER_DOCKER_PATH:/home/$USER:rw" \
    --volume="$BUILD_DOCKER_PATH:/home/$USER/build:rw" \
    --volume="$REROBAPP_PATH/libs:/home/$USER/libs:rw" \
    --name beaglebone_c --workdir="/home/$USER" \
    beaglebone2023:lasted "./src/task.sh"

rsync -aruvh  "$BUILD_DOCKER_PATH" "$SSH_USER:/home/debian/"
rsync -aruvh  exoapp/res/ "$SSH_USER:/home/debian/res"

# read -p "Press enter to continue . . ."


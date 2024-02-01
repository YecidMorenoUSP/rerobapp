#!/bin/bash

if [ -f ~/.config/rerobapp/rerobapp.conf ]; then
   source ~/.config/rerobapp/rerobapp.conf
else
   echo "Loading rerobapp.conf local"
   source ../rerobapp.conf
fi

rsync -aruvh  "$BUILD_DOCKER_PATH" "$EXOTAO_USER_SSH:/home/debian/"
rsync -aruvh  "$REROBAPP_PATH/src/res/" "$EXOTAO_USER_SSH:/home/debian/res"
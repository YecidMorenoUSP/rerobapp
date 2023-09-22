#!/bin/bash
ls
scp -r $BUILD_DOCKER_PATH debian@192.168.7.2:/home/debian/

# ssh debian@192.168.7.2 mkdir /home/debian/rerobapp
# scp -r src res Makefile debian@192.168.7.2:/home/debian/rerobapp

# rsync -aruvh  src res Makefile debian@192.168.7.2:/home/debian/rerobapp

# ssh debian@192.168.7.2 "cd /home/debian/rerobapp  ; make"
# ssh debian@192.168.7.2 "cd /home/debian/rerobapp ; make clean ; make"

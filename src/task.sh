#!/bin/bash

cd ~
cd src/libs_rerob/canlib/
make

cd ~
cd src/exoapp/ 
make

cd ~
cd src/server_udp/
make

cd ~
cd src/server_shm/
make
#!/bin/sh

cp pdk /opt/pdk/pdk_room/pdk_room -f
cd /opt/pdk/pdk_room
. ./restart.sh
cd -


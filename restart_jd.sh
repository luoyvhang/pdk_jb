#!/bin/sh

cp pdk /opt/pdk/pdk_20/pdk -f
cd /opt/pdk/pdk_20
. ./restart.sh
cd -

sleep 5
cp pdk /opt/pdk/pdk_50/pdk -f
cd /opt/pdk/pdk_50
. ./restart.sh
cd -

sleep 5
cp pdk /opt/pdk/pdk_100/pdk -f
cd /opt/pdk/pdk_100
. ./restart.sh
cd -

sleep 5
cp pdk /opt/pdk/pdk_200/pdk -f
cd /opt/pdk/pdk_200
. ./restart.sh
cd -

sleep 5
cp pdk /opt/pdk/pdk_500/pdk -f
cd /opt/pdk/pdk_500
. ./restart.sh
cd -



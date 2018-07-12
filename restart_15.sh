#!/bin/sh

cp pdk /opt/pdk15/pdk15_20/pdk15 -f
cd /opt/pdk15/pdk15_20
. ./restart.sh
cd -

sleep 5
cp pdk /opt/pdk15/pdk15_50/pdk15 -f
cd /opt/pdk15/pdk15_50
. ./restart.sh
cd -

sleep 5
cp pdk /opt/pdk15/pdk15_100/pdk15 -f
cd /opt/pdk15/pdk15_100
. ./restart.sh
cd -

sleep 5
cp pdk /opt/pdk15/pdk15_200/pdk15 -f
cd /opt/pdk15/pdk15_200
. ./restart.sh
cd -

sleep 5
cp pdk /opt/pdk15/pdk15_500/pdk15 -f
cd /opt/pdk15/pdk15_500
. ./restart.sh
cd -


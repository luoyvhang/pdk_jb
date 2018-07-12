#pragma once

#include "Common/logfile.h"
#include "Common/objPool.h"
#include "Common/multpleThread.h"

#include "evTimer.h"
#include "jsonPacket.h"
#include "proto.h"

#include "gamedb.h"
#include "classAward.h"

#include <stdint.h>

//未定义的对象使用下面的这个ID标识
#define OBJECT_ID_UNDEFINED		0
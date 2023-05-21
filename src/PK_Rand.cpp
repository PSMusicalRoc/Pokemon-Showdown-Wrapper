#include "PK_Rand.h"
#include <time.h>

std::mt19937 PK_RAND::mt_rand(time(0));
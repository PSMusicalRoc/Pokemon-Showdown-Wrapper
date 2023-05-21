#ifndef _ROC_RANDOM_GENERATOR_H_
#define _ROC_RANDOM_GENERATOR_H_

#include <random>

struct PK_RAND {
    /** The mersenne twister object for random number generation.*/
    static std::mt19937 mt_rand;
};

#endif
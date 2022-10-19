#include "util.h"

int cvi_bound(int min, int max, int value)
{
    if (value <= min)
        return min;

    if (value >= max)
        return max;
    
    return value;
}
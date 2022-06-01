#include "fast_rand.h"

static unsigned int g_seed;

// used to seed the generator
void fast_srand(int seed)
{
    g_seed = seed;
}

// compute a pseudorandom integer
// output value in range [0, 32767]
int fast_rand(void)
{
    g_seed = 214013 * g_seed + 2531011;
    return (g_seed >> 16) & 0x7FFF;
}

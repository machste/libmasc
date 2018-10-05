#include <stdlib.h>

#include <masc/random.h>
#include <masc/mloop.h>


static bool seed_set = false;
static unsigned int seed;


void random_seed(unsigned int _seed)
{
    seed = _seed;
    srandom(seed);
    seed_set = true;
}

int random_int(int a, int b)
{
    if (!seed_set) {
        random_seed((unsigned int)mloop_time());
    }
    return a + random() % (b - a + 1);
}

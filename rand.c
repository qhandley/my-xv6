// Source: https://www.cs.virginia.edu/~cr4bd/4414/F2018/files/lcg_parkmiller_c.txt

#include "types.h"

static uint random_seed = 1;

#define RANDOM_MAX ((1u << 31u) - 1u)
uint lcg_parkmiller(uint *state)
{
    const uint N = 0x7fffffff;
    const uint G = 48271u;

    /*  
        Indirectly compute state*G%N.

        Let:
          div = state/(N/G)
          rem = state%(N/G)

        Then:
          rem + div*(N/G) == state
          rem*G + div*(N/G)*G == state*G

        Now:
          div*(N/G)*G == div*(N - N%G) === -div*(N%G)  (mod N)

        Therefore:
          rem*G - div*(N%G) === state*G  (mod N)

        Add N if necessary so that the result is between 1 and N-1.
    */
    uint div = *state / (N / G);  /* max : 2,147,483,646 / 44,488 = 48,271 */
    uint rem = *state % (N / G);  /* max : 2,147,483,646 % 44,488 = 44,487 */

    uint a = rem * G;        /* max : 44,487 * 48,271 = 2,147,431,977 */
    uint b = div * (N % G);  /* max : 48,271 * 3,399 = 164,073,129 */

    return *state = (a > b) ? (a - b) : (a + (N - b));
}

uint rand()
{
    return lcg_parkmiller(&random_seed);
}


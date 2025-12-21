// There are 43,252,003,274,489,856,000 unique Rubik's cube positions and it
// has been proven that all of them can be solved in 20 moves or less.
//
// Total unique possible positions after each move increases like so:
//
// 00: 1
// 01: 18
// 02: 243
// 03: 3,240
// 04: 43,239
// 05: 574,908
// 06: 7,618,438
// 07: 100,803,036
// 08: 1,332,343,288
// 09: 17,596,479,795
// 10: 232,248,063,316
// 11: 3,063,288,809,012
// 12: 40,374,425,656,248
// 13: 531,653,418,284,628
// 14: 6,989,320,578,825,350
// 15: 91,365,146,187,124,300
// 16: ~1,100,000,000,000,000,000
// 17: ~12,000,000,000,000,000,000
// 18: ~29,000,000,000,000,000,000
// 19: ~1,500,000,000,000,000,000
// 20: ~490,000,000
//
// Total of these values should equal number of unique Rubik's cube positions.
// These numbers are far too large to bruteforce naively so a solving strategy
// is needed.
//
// ----------------------------------------------------------------------------
//
// Two-Phase-Algorithm (Herbert Kociemba)
// https://kociemba.org/cube.htm
//
// Kociemba's Two Phase Algorithm uses many astute observations to reduce the
// search space and make finding an optimal solve feasible. The main genius is
// making moves until the cube is in a state where it is known to be solveable
// only using a subset of moves G1 = <U,D,R2,L2,F2,B2>. Optimisations such as
// calculating equivalent and symmetric cubes, generating large pruning tables
// to speed up computation, and representing the cube using multiple encodings
// for facelet, cubie and coordinate levels ensure the algorithm can run fast
// in both time and space.
//
// The author reports being able to solve around 7000 cubes optimally per day
// which is ~12.3seconds per solve on average. This was using his efficient
// multi-threaded implementation of the algorithm in C.
//
// ----------------------------------------------------------------------------
//
// From my research there doesn't seem to be a faster method than the
// Two-Phase-Algorithm that can find optimal solves for cubes. Rather than
// trying to re-implement the Two-Phase-Algorithm myself I have instead decided
// to write a solver that implements CFOP, a standard speed-cubing method.
// Whilst this method is far from optimal, I am glad I will be able to write
// the solving code myself without reference. It will also mean that this
// program can be run with very little memory and computation requirements as
// it is a naive solver. I look forward to using this computer program to teach
// myself the CFOP solving method as I only know how to solve a cube using the
// beginner method.


#include "solve.h"


void SolveCube(Arena* arena, Cube* cube) {
    printf("Solve!\n");
}

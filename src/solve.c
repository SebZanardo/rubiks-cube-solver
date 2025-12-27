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
//
// Whilst this method is far from optimal, I am glad I will be able to write
// the solving code myself without reference. It will also mean that this
// program can be run with very little memory and computation requirements as
// it is a naive solver. I look forward to using this computer program to teach
// myself the CFOP solving method as I only know how to solve a cube using the
// beginner method.
//
// For the cross stage I have implemented a search algorithm to ensure the
// solver finds the minimum number of moves. This is beneficial to a human
// using the software as it is the first stage of the solve that is thought
// about whilst inspecting the cube. The cross can always be solved in 8 or
// less moves. This comes out to 18^8 ~= 11 billion states. This is searchable
// already but with one simple optimisation it becomes trivial. Storing seen
// edge states for the four white corners: (12*2)*(11*2)*(10*2)*(9*2) = 190,080
// combinations the search space becomes easy for a simple BFS. BFS is better
// than DFS in this case as all combinations can be stored in memory and we can
// exit the search as soon as we find the cross state often 5-6 moves, not the
// maximum of 8. Using IDDFS is not needed as the hashmap with all states can
// fit into memory easily and BFS is faster.
//
// For F2L, PLL, and OLL I have simply used lookup tables containing
// cases for all possible states with the minimal number of turns to solve
// each.


#include "solve.h"


DEFINE_TYPED_STACK(TurnType, MoveStack)
DEFINE_TYPED_QUEUE(u32, QueueU32)

#define MOVE_STACK_LEN 300


// (12*2)*(11*2)*(10*2)*(9*2) = 190,080. This can be divided by two due to edge
// parity rules that states sum of edge parity must be even
#define CROSS_EDGE_LEN (190080 / 2)

// 2^20 = 1048576 as there are 20bits in cube hash. This ensures no collisions
// but only 95,040 spots can be filled so hashmap is ~1/10 filled...
#define CROSS_HASHMAP_LEN 1048576


static const u8 CROSS_TURN_TABLE[6][4] = {
    { 2, 4, 8, 7 },     // F
    { 1, 5, 9, 4 },     // R
    { 0, 1, 2, 3 },     // U
    { 0, 6, 10, 5 },    // B
    { 3, 7, 11, 6 },    // L
    { 8, 9, 10, 11 }    // D
};


typedef void (*SolveFunction)(Arena* arena, MoveStack* moves, Cube* cube);


static void PerformTurn(MoveStack* moves, Cube* cube, TurnType turn_type) {
    CubeColour face = turn_type % 6;

    if (turn_type < 6) {
        CubeFaceTurnClockwise(cube, face);
    } else if (turn_type < 12) {
        CubeFaceTurnAntiClockwise(cube, face);
    } else {
        CubeFaceTurnDouble(cube, face);
    }

    MoveStack_append(moves, turn_type);
}

static void PrintMoves(MoveStack* moves, int start, int end) {
    printf("Moves: %d\n", end - start);
    for (int i = start; i < end; i++) {
        printf("%s\n", TURN_TYPE_NAMES[moves->items[i]]);
    }
    printf("\n");
}

/*
static void PrintHash(u32 hash) {
    u32 position = (hash << 12) >> 12;
    printf("position: ");

    for (int i = 19; i >= 0; i--) {
        int bit = (position >> i) & 1;
        printf("%d", bit);
        if (i % 5 == 0) {
            printf(" ");
        }
    }
    printf("\n");
}
*/

static u32 HashCrossCube(Cube* cube) {
    // This function searches the cube for the four white edges so that their
    // position and orientation can be stored in a simplified state.
    //
    // The hash stores the four edges in sorted order (GW, RW, WO, BW). The
    // first bit of each set of 5 bits stores the orientation (flipped/not)
    // then the last 4 bits stores the position between 0-12. The top 12 bits
    // are unused for the hash.
    u32 hash = 0;

    for (int i = 0; i < 12; i++) {
        CubeColour edge1 = CUBE_EDGE_COLOUR_TABLE[i * 2];
        CubeColour edge2 = CUBE_EDGE_COLOUR_TABLE[i * 2 + 1];
        u8 pos1 = CUBE_EDGE_POSITION_TABLE[i * 2];
        u8 pos2 = CUBE_EDGE_POSITION_TABLE[i * 2 + 1];

        CubeColour current1 = FaceGetTile(cube->faces[edge1], pos1);
        CubeColour current2 = FaceGetTile(cube->faces[edge2], pos2);

        for (int j = 0; j < 4; j++) {
            CubeColour target1 = CUBE_EDGE_COLOUR_TABLE[j * 2];
            CubeColour target2 = CUBE_EDGE_COLOUR_TABLE[j * 2 + 1];

            if (current1 == target1 && current2 == target2) {
            } else if (current1 == target2 && current2 == target1) {
            } else {
                continue;
            }

            // Write position
            u8 chunk = i;

            // Write flip bit: 0 is correct 1 is flipped
            //
            //         . 0 .
            //         0 U 0
            //         . 0 .
            //
            //  . 1 .  . 1 .  . 1 .  . 1 .
            //  1 L 1  0 F 0  1 R 1  0 B 0
            //  . 1 .  . 1 .  . 1 .  . 1 .
            //
            //         . 0 .
            //         0 D 0
            //         . 0 .
            //
            if (current1 != CUBE_WHITE) {
                chunk |= Bit(4);
            }

            // Write into hashed block at correct position
            hash |= (chunk << (j * 5));
            break;
        }
    }

    return hash;
}

static u32 TurnHashCross(u32 hash, TurnType turn_type) {
    // Extract sections from hash for easy modification
    u8 edge_position[4];
    u8 edge_orientation[4];
    u8 seen[4];
    for (int i = 0; i < 4; i++) {
        int shift = i * 5;
        u8 chunk = (hash >> shift) & 0x1F;
        edge_position[i] = chunk & 0xF;
        edge_orientation[i] = chunk >> 4;
        seen[i] = false;
    }

    u8 face = turn_type % 6;

    // Manipulate
    for (int i = 0; i < 4; i++) {
        u8 target_pos = CROSS_TURN_TABLE[face][i];

        // For each edge position
        for (int j = 0; j < 4; j++) {
            if (seen[j]) continue;

            u8 current_pos = edge_position[j];

            if (target_pos != current_pos) continue;

            seen[j] = true;

            // Orientation. Don't change edge orientation for double turns
            if (turn_type < 12 && (face == CUBE_GREEN || face == CUBE_BLUE)) {
                // Toggle, turn one to zero and zero to one
                edge_orientation[j] = 1 - edge_orientation[j];
            }

            // Position
            if (turn_type < 6) {
                // Clockwise
                edge_position[j] = CROSS_TURN_TABLE[face][ModWrap(i + 1, 4)];
            } else if (turn_type < 12) {
                // AntiClockwise
                edge_position[j] = CROSS_TURN_TABLE[face][ModWrap(i - 1, 4)];
            } else {
                // Double
                edge_position[j] = CROSS_TURN_TABLE[face][ModWrap(i + 2, 4)];
            }
        }
    }

    // Construct hash
    u32 new_hash = 0;
    for (int i = 0; i < 4; i++) {
        int shift = i * 5;
        u8 chunk = edge_position[i] | (edge_orientation[i] << 4);
        new_hash |= (chunk << shift);
    }

    return new_hash;
}

static u32 CrossBFS(
    u32* visited, QueueU32* queue, u32 starting_hash, u32 target_hash
) {
    // Add current hash to queue as first element and mark in visited
    QueueU32_append(queue, starting_hash);
    visited[starting_hash] = starting_hash | (TURN_TYPE_COUNT << 20);

    // Check if cross already solved
    if (starting_hash == target_hash) {
        return target_hash;
    }

    u32 current_hash = starting_hash;

    // BFS
    while (QueueU32_length(queue) > 0) {
        QueueU32_pop(queue, &current_hash);
        for (int turn_type = 0; turn_type < TURN_TYPE_COUNT; turn_type++) {
            // Perform all 18 move types from this position
            u32 new_hash = TurnHashCross(current_hash, turn_type);

            // Check if new hash is in visited
            if (visited[new_hash] != 0) continue;

            // If new record and add to queue
            QueueU32_append(queue, new_hash);
            visited[new_hash] = current_hash | (turn_type << 20);

            // Check if hash is target hash and terminate search
            if (new_hash == target_hash) {
                return target_hash;
            }
        }
    }

    // Failed to solve... this should never be hit
    return UINT32_MAX;
}

static bool IsCrossSolved(Cube* cube) {
    for (int i = 0; i < 8; i++) {
        CubeColour colour = CUBE_EDGE_COLOUR_TABLE[i];
        u8 position = CUBE_EDGE_POSITION_TABLE[i];
        if (FaceGetTile(cube->faces[colour], position) != colour) return false;
    }
    return true;
}

static void SolveStep(
    SolveFunction func, Arena* arena, MoveStack* moves, Cube* cube
) {
    int moves_before = MoveStack_length(moves);

    clock_t start = clock();
    func(arena, moves, cube);
    clock_t end = clock();

    double elapsed = (double) (end - start) / CLOCKS_PER_SEC;
    printf("Time: %f seconds\n", elapsed);

    int moves_after = MoveStack_length(moves);
    PrintMoves(moves, moves_before, moves_after);
}

static void SolveCross(Arena* arena, MoveStack* moves, Cube* cube) {
    printf("CROSS:\n");

    // We need to encode the position of the four edges into an integer to use
    // in the lookup table. Each of the 190,080 configurations needs a spot in
    // the array. To reconstruct the path we need to store the turn type and
    // parent at each u32 index in the array, luckily there can only be one
    // parent as two moves that result in the same hash at the same BFS step
    // either can be picked. Due to the hash side this cannot fit into a u16:
    //
    // |    (TurnType)    |            (parent hash)           |
    // |  0000 0000 0000  |   0 0000  0 0000  0 0000  0 0000   |
    // |      12-bits     |              20-bits               |
    //

    // Compute hash for cube with cross solved
    Cube solved_cube;
    CubeInit(arena, &solved_cube);
    CubeSetSolved(&solved_cube);
    u32 target_hash = HashCrossCube(&solved_cube);

    // Allocate hashtable to store parent hashes and turn types
    u32* visited = ArenaPushArray(arena, CROSS_HASHMAP_LEN, u32);

    // Allocate queue to store hashes that need to be searched
    u32* items = ArenaPushArray(arena, CROSS_EDGE_LEN, u32);
    QueueU32 queue;
    QueueU32_init(&queue, items, CROSS_EDGE_LEN);

    u32 starting_hash = HashCrossCube(cube);

    // Run BFS
    assert(CrossBFS(visited, &queue, starting_hash, target_hash) == target_hash);

    // Reconstruct path. Start at end and traverse backwards. Max length 8
    TurnType path[8];
    int path_length = 0;
    u32 current_hash = target_hash;
    for (int i = 0; i < 8; i++) {
        if (current_hash == starting_hash) break;
        u32 hash_pair = visited[current_hash];
        TurnType move = hash_pair >> 20;
        current_hash = (hash_pair << 12) >> 12;
        path[path_length++] = move;
    }

    // Perform moves writing to movestack
    for (int i = path_length - 1; i >= 0; i--) {
        PerformTurn(moves, cube, path[i]);
    }

    // Sanity check
    assert(IsCrossSolved(cube));
}

static void SolveF2L(Arena* arena, MoveStack* moves, Cube* cube) {
    printf("F2L:\n");
    printf("NOT IMPLEMENTED YET\n");
}

static void SolveOLL(Arena* arena, MoveStack* moves, Cube* cube) {
    printf("OLL:\n");
    printf("NOT IMPLEMENTED YET\n");
}

static void SolvePLL(Arena* arena, MoveStack* moves, Cube* cube) {
    printf("PLL:\n");
    printf("NOT IMPLEMENTED YET\n");
}

MoveStack* SolveCube(Arena* arena, Cube* cube) {
    ArenaReset(arena);
    TurnType* items = ArenaPushArray(arena, MOVE_STACK_LEN, TurnType);

    MoveStack* moves = ArenaPushStruct(arena, MoveStack);
    MoveStack_init(moves, items, MOVE_STACK_LEN);

    printf("----- SOLVE -----\n");

    SolveStep(SolveCross, arena, moves, cube);
    SolveStep(SolveF2L, arena, moves, cube);
    SolveStep(SolveOLL, arena, moves, cube);
    SolveStep(SolvePLL, arena, moves, cube);

    return moves;
}

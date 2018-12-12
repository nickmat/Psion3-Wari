/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Project: WA  WARI GAME FOR PSION 3a       *  Written by: Nick Matthews  *
 *  Module: CALCULATE WARI'S NEXT MOVE       *  Date Started: 12 Jul 1996  *
 *    File: WARIMOVE.H       Type: C HEADER  *  Date Revised: 12 Jul 1996  *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

*/

#ifndef WARIMOVE_H
#define WARIMOVE_H

/*
**  Position  Pointer to current board
**  Player    0=1st Player, 1=2nd Player
**  Level     Number of levels to try (Max == 10?)
*/
extern int GetWariMove( int* Position, int Player, int Level );

/* Player definitions */
#define PLAYER1  0
#define PLAYER2  1

/* PSION specific stuff */

#ifdef IN_WARIMOVE_C

#include <p_std.h>
#include <hwimman.g>
#include <wari.g>

/* memcpy is the only standard library call used */
#define memcpy(T,S,C)   p_bcpy((T),(S),(C))

GLREF_D PR_HWIMMAN* w_am;

static VOID* aidle;

#define BEGYIELD   aidle = f_newsend( CAT_WARI_OLIB, C_AIDLE, O_AO_INIT );

#define ENDYIELD   p_send2( aidle, O_DESTROY );

#define YIELD  {\
                   static int i;\
                   while( i++ % 100 == 0 )\
                   {\
                       p_send2( aidle, O_AO_QUEUE );\
                       p_send2( w_am, O_AM_START );\
                   }\
               }

#endif /* IN_WARIMOVE_C */

#endif /* WARIMOVE_H */

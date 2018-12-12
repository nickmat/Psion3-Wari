/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Project: WA  WARI GAME SHARED SOURCE FILE *  Written by: See Copyright  *
 *  Module: CALCULATE WARI'S NEXT MOVE       *  Date Started: 25 Jun 1991  *
 *    File: WARIMOVE.C     Type: C SOURCE    *  Date Revised: 12 Jul 1996  *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

 Notes
 These functions have been taken from the Zortech Games library, with
 only slight modification.

 Copyright (c) OXFORD MOBIUS, 1987
 Copyright (c) Nick Matthews, 1991, 1996

 This contains the main search and evaluate functions to determine the
 machine's move. For a fuller explanation see the section on the WARI
 program in the Zortech Games manual.

 12jul96  GetWariMove: Renamed (was MachineMove) and added parameters
          Position and Level instead of using global variables. Header
          file WARIMOVE.H added.
*/

#define IN_WARIMOVE_C
#include "warimove.h"

static int lookm( int ply, int alpha, int beta, int* which );
static int lookh( int ply, int alpha, int beta );
static int evaluate( void );
static void update( int n );

static char acBuffer[ 500 ]; /* look ahead board space ,essentially stack */
                             /* OK for at least level 10 */

static char* board;          /* current board, pointer within acBuffer */

#define BOARDMAX 14          /* size of a board in bytes */

/*#*************************************************************************
 **  GetWariMove  Calculate the best move from Position, for the Player
 **  ~~~~~~~~~~~  (PLAYER1 or PLAYER2) using the given Level.
 **  Position index 0 - 5 are the PLAYER2 pits, 6 - 11 are PLAYER1,
 **  12 is PLAYER2 captures, and 13 is PLAYER1 captures.
 */

int GetWariMove( int* Position, int Player, int Level )
{
    int i, reply;

    BEGYIELD
    board = acBuffer;
    /* The system is based on finding the move for PLAYER2 */
    if( Player == PLAYER2 )
    {
        for( i = 0 ; i < BOARDMAX ; i++ )
        {
            board[i] = Position[i];
        }
        lookm( Level, -32767, 32767, &reply );
    }
    else
    {
        for( i = 0 ; i < 6 ; i++ )
        {
            board[i] = Position[i+6];
            board[i+6] = Position[i];
        }
        board[12] = Position[13];
        board[13] = Position[12];
        lookm( Level, -32767, 32767, &reply );
        if( reply != -1 ) reply += 6;
    }
    ENDYIELD
    return reply;
}

/***************************************************************************
**  downdate  Defined as a macro, it simply re-sets the current board
**  ~~~~~~~~  to the previous one by reducing board by one stack position
*/

#define downdate() board -= BOARDMAX

/*#*************************************************************************
 ** lookm  This looks for the best move to play when it is the machine's
 ** ~~~~~  turn to move. If the last level of search is reached calls
 ** evaluate() to provide a static value of the board. Otherwise it is
 ** mutually recursive with lookh(). Calls lookh() for every possible
 ** move and normally returns the highest value returned from this.
 ** alpha represents the best advantage so far for machine, and beta
 ** likewise for player. Uses alpha-beta cutoff to abandon search if
 ** appropriate. For more details see manual.
 */

static int lookm( int ply, int alpha, int beta, int* which )
{
    int i, val, maxval ;

    if( ply <= 0 ) return evaluate() ;   /* if last level search call */
                                         /* evaluate() for static val */
    maxval = -32767 ;              /* set to low val to ensure change */
    *which = -1 ;               /* init this to indicate no possible  */
                                /* move if not changed (only relevant */
                                /* to call from main)                 */
    --ply ;                       /* indicate a lower level of search */
    for( i=0; i<6; ++i ) {          /* try each poss move for machine */
        if( board[i] ) {             /* dont try if move not possible */
            update( i ) ;           /* make a new board with proposed */
                                    /* move made */
            val = lookh( ply, beta, alpha ) ;   /* get player's value */
                                                /* of this board      */
            downdate() ;          /* restore board as to before update*/
            if( val > maxval ) {  /* is this best of those tried yet? */
                maxval = val ;                  /* yes! re-set maxval */
                *which = i ;   /* and parameter to indicate best move */
                           /* if this is better than best in previous */
                                            /* searches, re-set alpha */
                if( maxval > alpha ) alpha = maxval ;
                         /* alpha-beta cutoff, break if this position */
                            /* not reached if player plays best moves */
                if( maxval >= beta ) break;
            }
        }
    }
    if( maxval < -32760 ) {   /* if end of game return score based on */
                                     /* difference in captured pieces */
        return ( ( board[12] << 1 ) - 48 ) << 7 ;
    }
    return maxval ;      /* otherwise return value of best move found */
}

/*#*************************************************************************
 **  lookh  This is very similar to lookm() above except the player is
 **  ~~~~~  trying to achieve a lower value as this is a better position
 ** to player. Considers all possible player's moves and returns value
 ** of best.
 */

static int lookh( int ply, int alpha, int beta )
{
    int i, j, val, maxval;

    if( ply <= 0 ) {                    /* static value if last level */
        return evaluate() ;
    }
    maxval = 32767 ;                 /* init to high as will go lower */
    --ply ;                                    /* indicate next level */

    YIELD
    for( i = 6 ; i < 12 ; ++i ) {     /* Try all legal player's moves */
        if( board[i] ) {
            update( i ) ;                     /* generate new position*/
            val = lookm( ply, beta, alpha, &j ) ; /* look at position */
                                               /* from machine's view */
            downdate() ;                 /* restore original position */

            if( val < maxval ) {     /* is this move better(ie less!) */
                                                     /* than previous */
                maxval = val ;                   /* yes! reset maxval */
                if( maxval < alpha ) {
                    alpha = maxval;     /* reset players best if necc */
                }
                if( maxval <= beta ) {
                    break ;    /* cutoff if machine will not get here */
                }
            }
        }
    }
                                      /* if no moves return end-score */
    if( maxval > 32760 ) {
        return ( 48 - ( board[13] << 1 ) ) << 7 ;
    }
                                         /* otherwise best move found */
    return maxval ;
}

/*#*************************************************************************
 **  evaluate  This returns a value of a position based purely upon the
 **  ~~~~~~~~  current board without making any further searches. Note
 ** that this value is always based on the machine's viewpoint, hence a
 ** more positive value is a better position for the machine and a more
 ** negative value better for the player. The value is based upon the
 ** difference in captured pieces and to a lesser extent upon the number
 ** of opponents pits being attacked for each player.
 */

static int evaluate( void )
{
    int n, i, score, test[12];

    for( i = 0 ; i < 12 ; test[i++] = 0 ) ;   /* Initialise our array */
                              /* indicating 'hits' found for each pit */
    score = ( board[12] - board[13]) << 4 ;    /* score difference in */
                          /* captured stones multiplied by 16 as this */
                                  /* is more important than following */

    for( i = 0 ; i < 6 ; ++i ) {  /* Go through each machine's pit to */
                                 /* find any opponent's pits attacked */
        n = ( i + board[i] ) % 12 ;    /* Find pit at which this move */
                                                        /* would stop */
        if( n > 5 ) {                   /* Only look at player's pits */
            score += test[n] ? 1 : 2 ;    /* Increase score by two if */
               /* player's pit has not yet been hit, otherwise by one */
            ++test[n] ;                 /* indicate this pit is 'hit' */
        }
    }
    for( i = 6 ; i < 12 ; ++i ) {    /* Go through each player's pits */
        n = ( i + board[i] ) % 12 ;         /* pit which would be hit */
        if( n < 6 ) {                          /* only machine's pits */
            score -= test[n] ? 1 : 2 ;       /* score as above except */
                    /* subtract as this is an advantage to the player */
            ++test[n];
        }
    }

    return score ;
}

/*#*************************************************************************
 **  update  This will produce a board corresponding to the position
 **  ~~~~~~  reached if the given move is made from the current board,
 ** and update the current board to contain this new board. In effect a
 ** new board is generated from a copy of the current board and
 ** 'stacked' so that the space can be later reclaimed.
 */

static void update( int n )
{
    int c, side, pit ;

    if( n < 6 ) {                         /* Is this a machine move ? */
        side = 6 ;           /* yes! set side and pit to reflect this */
        pit = 12 ;
    } else {                            /* no! again set side and pit */
        side = 0 ;
        pit = 13 ;
    }          /* NOTE the side whose move it is is referred as mside */

    memcpy( board + BOARDMAX, board, BOARDMAX ) ;     /* copy current */
                                     /* board to new 'stack' position */
    board += BOARDMAX ;   /* and re-set current board to reflect this */
    c = board[ n ] ;         /* obtain number of stones on 'move' pit */
    board[ n ] = 0 ;                    /* and zero this pit position */

    while( c ) {              /* loop around board placing one of the */
                                      /* available stones on each pit */
        n += 1 ;                                          /* next pit */
        if( n > 11 ) n = 0 ;        /* end of board, go back to start */
        ++board[n] ;                                 /* deposit stone */
        --c ;                              /* reduce stones available */
    }                           /* n is now at the final or 'hit' pit */
    while(                /* while the pit is still on mside's set of */
        n >= side        /* pits AND there were previously one or two */
        && n < side + 6     /* stones in a pit then capture pieces in */
        && board[n] > 1      /* these pits, going back along the pits */
        && board[ n ] < 4 ) {   /* until no more captures can be made */

        board[pit] += board[n] ;   /* increase mside's captured score */
        board[n] = 0 ;                               /* zero this pit */
        --n ;                                        /* next pit down */
    }
}

/* End of WARIMOVE.C file */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Project: WA  WARI GAME FOR PSION 3a       *  Written by: Nick Matthews  *
 *  Module: WARI GLOBALS AND PROTOTYPES      *  Date Started: 13 Jul 1996  *
 *    File: WARI.H          Type: C HEADER   *  Date Revised: 13 Jul 1996  *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/

#ifndef WARI_H
#define WARI_H

extern void DrawBoard( void );
extern void DrawStone( int cup, int stone, int mode );
extern void DrawHiLight( int cup, int mode );

/* defines foe DrawHiLight mode */
#define DRAW_BLACK   0
#define DRAW_BGROUND 1

#endif /* WARI_H */

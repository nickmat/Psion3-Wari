/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Project: WA  WARI GAME FOR PSION 3a       *  Written by: Nick Matthews  *
 *  Module: WARI PAINT AND UPDATE SCREEN     *  Date Started: 27 Jun 1991  *
 *    File: waridraw.c       Type: C SOURCE  *  Date Revised: 22 May 1996  *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

*/

#include <wlib.h>
#include "wari.h"

#define BLACK_CIRCLE    0
#define GREYFILL_CIRCLE 1

#define CUP_RADIUS 25
#define CAP_RADIUS 36
#define HIL_RADIUS 27

#define WBY_TOP   35
#define WBY_CEN   70
#define WBY_BOT  105
#define WBX_LEF   43
#define WBX_CP1  104
#define WBX_CP2  158
#define WBX_CP3  212
#define WBX_CP4  266
#define WBX_CP5  320
#define WBX_CP6  374
#define WBX_RIT  435

/* center coordinate of each cups */
static int axCoord[14] =
{
    WBX_CP6, WBX_CP5, WBX_CP4, WBX_CP3, WBX_CP2, WBX_CP1,
    WBX_CP1, WBX_CP2, WBX_CP3, WBX_CP4, WBX_CP5, WBX_CP6,
    WBX_LEF, WBX_RIT
};
static int ayCoord[14] =
{
    WBY_TOP, WBY_TOP, WBY_TOP, WBY_TOP, WBY_TOP, WBY_TOP,
    WBY_BOT, WBY_BOT, WBY_BOT, WBY_BOT, WBY_BOT, WBY_BOT,
    WBY_CEN, WBY_CEN
};

/* offset of centre of stones in each cup for the nth stone, */
static int axOffset[48] = {
     -3,  3,  -3,  3,  -9,  9,  -9,  9,  -3,  3,  -3,  3,
    -15, 15, -15, 15,  -9,  9,  -9,  9,  -3,  3,  -3,  3,
    -21, 21, -21, 21, -15, 15, -15, 15,  -9,  9,  -9,  9,
     -3,  3,  -3,  3, -21, 21, -21, 21, -15, 15, -15, 15  };
static int ayOffset[48] = {
      3,  3,  -3,  -3,  3,  3, -3, -3,  9,  9,  -9,  -9,
      3,  3,  -3,  -3,  9,  9, -9, -9, 15, 15, -15, -15,
      3,  3,  -3,  -3,  9,  9, -9, -9, 15, 15, -15, -15,
     21, 21, -21, -21,  9,  9, -9, -9, 15, 15, -15, -15  };

static void DrawCircle( int xc, int yc, int rad, UINT mode );

/***************************************************************************
**  DrawCircle  Draws a circle at (xc, yc) with radius r
**  ~~~~~~~~~~
** note: the scaling factor of (SCREEN_WIDTH / SCREEN_HEIGTH) is used when
** updating d.  This makes round circles.  If you want ellipses, you can
** modify that ratio.
** Function taken from Snippets BRESNHAM.C file
*/

static void DrawCircle( int xc, int yc, int radius, UINT mode )
{
    int x = 0;
    int y = radius;
    int yp = radius;
    int d = 2 * ( 1 - radius );
    int xl, xr, yt, yb;
    G_GC gc;

    while( y >= 0 )
    {
        xl = xc - x;
        xr = xc + x;
        yt = yc - y;
        yb = yc + y;
        gDrawLine( xl, yt, xl, yt );
        gDrawLine( xr, yt, xr, yt );
        gDrawLine( xl, yb, xl, yb );
        gDrawLine( xr, yb, xr, yb );

        if( y != yp && mode == GREYFILL_CIRCLE )
        {
            gc.flags = G_GC_FLAG_GREY_PLANE;
            gSetGC( WS_TEMPORARY_GC, G_GC_MASK_GREY, &gc );

            gDrawLine( xl+1, yt, xr, yt );
            gDrawLine( xl+1, yb, xr, yb );

            gc.flags = 0;  /* BLACK Pane */
            gSetGC( WS_TEMPORARY_GC, G_GC_MASK_GREY, &gc );
            yp = y;
        }

        if( d + y > 0 )
        {
            y -= 1;
            d -= ( 2 * y /* * SCREEN_WIDTH / SCREEN_HEIGTH */ ) - 1;
        }
        if( x > d )
        {
            x += 1;
            d += ( 2 * x ) + 1;
        }
    }
}

void DrawStone( int cup, int stone, int mode )
{
    int x = axCoord[cup] + axOffset[stone];
    int y = ayCoord[cup] + ayOffset[stone];
    G_GC gc;

    if( mode == DRAW_BGROUND )
    {
        gc.gmode = G_TRMODE_CLR;
    }
    else
    {
        gc.gmode = G_TRMODE_SET;
    }
    gSetGC( WS_TEMPORARY_GC, G_GC_MASK_GMODE, &gc );

    gDrawLine( x - 1, y - 2, x + 2, y - 2 );
    gDrawLine( x - 2, y - 1, x + 3, y - 1 );
    gDrawLine( x - 2, y,     x + 3, y     );
    gDrawLine( x - 2, y + 1, x + 3, y + 1 );
    gDrawLine( x - 1, y + 2, x + 2, y + 2 );
}

/*#*********************************************************************
 **  PaintBoard  Draw the board without any stones or highlight.
 **  ~~~~~~~~~~
 */

void DrawBoard( void )
{
    int i/*, j*/;

    for( i = 0 ; i < 6 ; i++ )
    {
        DrawCircle( axCoord[i], ayCoord[i], CUP_RADIUS, BLACK_CIRCLE );
    }
    for( ; i < 12 ; i++ )
    {
        DrawCircle( axCoord[i], ayCoord[i], CUP_RADIUS, GREYFILL_CIRCLE );
    }
    DrawCircle( axCoord[12], ayCoord[12], CAP_RADIUS, BLACK_CIRCLE );
    DrawCircle( axCoord[13], ayCoord[13], CAP_RADIUS, GREYFILL_CIRCLE );
}

/*#*********************************************************************
 **  DrawHiLight  Draw or remove HighLight
 **  ~~~~~~~~~~~
 */

extern void DrawHiLight( int cup, int mode )
{
    G_GC gc;

    if( mode == DRAW_BGROUND )
    {
        gc.gmode = G_TRMODE_CLR;
    }
    else
    {
        gc.gmode = G_TRMODE_SET;
    }
    gSetGC( WS_TEMPORARY_GC, G_GC_MASK_GMODE, &gc );
    DrawCircle( axCoord[cup], ayCoord[cup], HIL_RADIUS, BLACK_CIRCLE );
    DrawCircle( axCoord[cup], ayCoord[cup], HIL_RADIUS-1, BLACK_CIRCLE );
}

/* End of "waridraw.c" source */

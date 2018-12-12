/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Project: WA  WARI GAME FOR PSION 3a       *  Written by: Nick Matthews  *
 *  Module: MAIN CLASS MEMBER FUNCTIONS      *  Date Started: 16 Jun 1996  *
 *    File: WARIMAIN.C      Type: C SOURCE   *  Date Revised: 11 Sep 1996  *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/

#include <epoc.h>
#include <hwimman.g>
#include <wari.g>
#include <wari.rsg>
#include <hwim.h>
#include "wari.h"
#include "warimove.h"

GLREF_D WSERV_SPEC* wserv_channel;
GLREF_D PR_HWIMMAN* w_am;
GLREF_D PR_WSERV*   w_ws;
PR_GAME*    Game;

#define NEWGAME   0
#define SETLEVEL  1
#define MESSAGE   3
#define SETSOUND  4

#define CHK_IN_PROGRESS   0x01
#define CHK_END_GAME      0x02
#define CHK_NO_UNDO       0x04

/***************************************************************************
 **  main  See SDK Vol 3 "Object Oriented Programming Guide" V2.10 Page 1-15
 **  ~~~~
 */

GLDEF_C VOID main( VOID )
{
    IN_HWIMMAN app;
    IN_WSERV ws;
    VOID* handle;

    p_linklib( 0 );

    app.flags = FLG_APPMAN_FULLSCREEN | FLG_APPMAN_RSCFILE |
        FLG_APPMAN_CLEAN | FLG_APPMAN_SRSCFILE;
    app.wserv_cat = p_getlibh( CAT_WARI_WARI );
    app.wserv_class = C_WARIWS;

    ws.com_cat = p_getlibh( CAT_WARI_WARI );
    ws.com_class = C_WARICM;

    handle = p_new( CAT_WARI_HWIM, C_HWIMMAN );
    p_send4( handle, O_AM_INIT, &app, &ws );
}

/***************************************************************************
 **  IsEnd  Return TRUE if side (0 or 6) cannot move, ie has no stones left.
 **  ~~~~~
 */

int IsEnd( int side )
{
    int i;

    for( i = side ; i < side + 6 ; i++ )
    {
        if( Game->game.Stones[i] )
        {
            return FALSE;
        }
    }
    return TRUE;
}

/***************************************************************************
 **  GetCupIndex  Return an index into Stones[] from the cup number (0-5).
 **  ~~~~~~~~~~~
 */

int GetCupIndex( int number )
{
    if( Game->game.TurnToPlay == GREY_SIDE )
    {
        return number + 6;
    }
    else
    {
        return 5 - number;
    }
}

/***************************************************************************
 **  GetCupNumber  Return the cup number (0-5) from the Stones[] index
 **  ~~~~~~~~~~~~  (0-11).Returns 6 if index = -1.
 */

int GetCupNumber( int index )
{
    if( index >= 6 )
    {
        return index - 6;
    }
    else
    {
        return 5 - index;
    }
}

/***************************************************************************
 **  MakeClick  Make a click sound on placing stone.
 **  ~~~~~~~~~
 */

void MakeClick( int count )
{
    if( Game->game.SoundOn )
    {
        if( count > 1 )
        {
            while( count-- )
            {
                p_sound( 1, 960 );
            }
        }
        else
        {
            p_sound( 2, 960 );
        }
    }
}

int CheckState( int flags )
{
    if( flags & CHK_IN_PROGRESS && Game->game.InProgress )
    {
        hBeep();
        hInfoPrint( STR_CHK_IN_PROGRESS );
        return TRUE;
    }
    if( flags & CHK_END_GAME &&
        Game->game.Stones[12] + Game->game.Stones[13] == 48 )
    {
        hBeep();
        hInfoPrint( STR_CHK_END_GAME );
        return TRUE;
    }
    if( flags & CHK_NO_UNDO && ! Game->game.UndoOk )
    {
        hBeep();
        hInfoPrint( STR_CHK_NO_UNDO );
        return TRUE;
    }
    return FALSE;
}

/*##########################[ Method Functions ]##########################*/

#pragma METHOD_CALL

/*-----------------------[ wariws - Window Server ]-----------------------*/

METHOD VOID wariws_ws_dyn_init( PR_WARIWS* self )
{
    self->wserv.cli = f_new( CAT_WARI_WARI, C_WARIBW );
    p_send2( self->wserv.cli, O_WN_INIT );
    self->wserv.help_index_id = WARI_HELP;
    Game = f_newsend( CAT_WARI_WARI, C_GAME, O_WG_INIT );
    p_send3( Game, O_WG_RESET, O_COM_GREY );
}

/*----------------------[ waricm - Command Manager ]----------------------*/

/***************************************************************************
 **  com_new  method
 **  ~~~~~~~
 */

METHOD VOID waricm_com_new( PR_WARICM* self )
{
    p_send3( Game, O_WG_RESET, 0 );
}

/***************************************************************************
 **  com_undo method
 **  ~~~~~~~~
 */

METHOD VOID waricm_com_undo( PR_WARICM* self )
{
    int i;

    if( CheckState( CHK_IN_PROGRESS | CHK_NO_UNDO ) ) return;
    for( i = 0 ; i < 14 ; i++ )
    {
        p_send4( w_ws->wserv.cli, O_WN_UPDATE_CUP, i,
            Game->game.LastMove[i] - Game->game.Stones[i] );
    }
    p_send2( w_ws->wserv.cli, O_WN_SET_SCORE );
    Game->game.TurnToPlay = Game->game.LastTurn;
    Game->game.Moves = Game->game.LastMoves;
    Game->game.UndoOk = FALSE;
    i = Game->game.TurnToPlay == GREY_SIDE ?
        STAT_GREY_TO_PLAY : STAT_WHITE_TO_PLAY;
    p_send3( Game->game.swin, O_WN_SET, i );
}

/***************************************************************************
 **  com_hint method
 **  ~~~~~~~~
 */

METHOD VOID waricm_com_hint( PR_WARICM* self )
{
    int cup, rid;

    if( CheckState( CHK_IN_PROGRESS | CHK_END_GAME ) ) return;
    p_send3( Game->game.swin, O_WN_SET, STAT_THINKING_HINT );
    Game->game.InProgress = TRUE;
    cup = GetWariMove( Game->game.Stones, Game->game.TurnToPlay,
        Game->game.Level );
    Game->game.InProgress = FALSE;
    p_send3( w_ws->wserv.cli, O_WN_SET_HILIGHT, cup );
    rid = Game->game.TurnToPlay == GREY_SIDE ?
        STAT_GREY_TO_PLAY : STAT_WHITE_TO_PLAY;
    p_send3( Game->game.swin, O_WN_SET, rid );
}

/***************************************************************************
 **  com_grey method
 **  ~~~~~~~~~
 */

METHOD VOID waricm_com_grey( PR_WARICM* self, UINT com )
{
    p_send3( Game, O_WG_RESET, com );
}

/***************************************************************************
 **  com_beginner method
 **  ~~~~~~~~~~~~
 */

METHOD VOID waricm_com_beginner( PR_WARICM* self )
{
    p_send4( Game, O_WG_SET_LEVEL, LEVEL_BEGINNER, BEGINNER_STR );
}

/***************************************************************************
 **  com_novice method
 **  ~~~~~~~~~~
 */

METHOD VOID waricm_com_novice( PR_WARICM* self )
{
    p_send4( Game, O_WG_SET_LEVEL, LEVEL_NOVICE, NOVICE_STR );
}

/***************************************************************************
 **  com_expert method
 **  ~~~~~~~~~~
 */

METHOD VOID waricm_com_expert( PR_WARICM* self )
{
    p_send4( Game, O_WG_SET_LEVEL, LEVEL_EXPERT, EXPERT_STR );
}

/***************************************************************************
 **  com_master method
 **  ~~~~~~~~~~
 */

METHOD VOID waricm_com_master( PR_WARICM* self )
{
    p_send4( Game, O_WG_SET_LEVEL, LEVEL_MASTER, MASTER_STR );
}

/***************************************************************************
 **  com_sound method
 **  ~~~~~~~~~
 */

METHOD VOID waricm_com_sound( PR_WARICM* self )
{
    DL_DATA data;

    data.id = SOUND_DIALOG;
    data.rbuf = NULL;
    data.pdlg = NULL;
    hLaunchDial( CAT_WARI_WARI, C_SOUNDDLG, &data );
}

/***************************************************************************
 **  com_speed method
 **  ~~~~~~~~~
 */

METHOD VOID waricm_com_speed( PR_WARICM* self )
{
    DL_DATA data;

    data.id = SPEED_DIALOG;
    data.rbuf = NULL;
    data.pdlg = NULL;
    hLaunchDial( CAT_WARI_WARI, C_SPEEDDLG, &data );
}

/***************************************************************************
 **  com_about method
 **  ~~~~~~~~~
 */

METHOD VOID waricm_com_about( PR_WARICM* self )
{
    DL_DATA data;

    data.id = ABOUT_DIALOG;
    data.rbuf = NULL;
    data.pdlg = NULL;
    hLaunchDial( CAT_WARI_HWIM, C_DLGBOX, &data );
}

/*------------------------[ waribw - Wari Window ]------------------------*/

/***************************************************************************
 **  Screen constants
 **  ~~~~~~~~~~~~~~~~
 **
 **    |----------------------------------| WW_TOP     A
 **    |                                  |            |
 **    |                                  |            | WW_HEIGHT
 **    |                                  |            |
 **    |----------------------------------| WW_STAT    X
 **    |                                  |            | WW_STAT_HT
 **    |----------------------------------| WW_BOT     v
 ** WW_LEFT                           WW_RIGHT
 **    <--------------WW_WIDTH------------>
 */

#define WW_TOP      0
#define WW_BOT      159
#define WW_LEFT     0
#define WW_RIGHT    479
#define WW_STAT_HT  20

#define WW_STAT     ( WW_BOT - WW_STAT_HT )
#define WW_HEIGHT     (WW_STAT+1)
#define WW_WIDTH      WW_RIGHT

METHOD VOID waribw_wn_init( PR_WARIBW* self )
{
    W_WINDATA wd;

    wd.extent.tl.x = WW_LEFT;
    wd.extent.tl.y = WW_TOP;
    wd.extent.width = WW_WIDTH;
    wd.extent.height = WW_HEIGHT;
    wd.background = W_WIN_BACK_BITMAP | W_WIN_BACK_GREY_BITMAP;
    p_send5( self, O_WN_CONNECT, NULL,
        W_WIN_EXTENT | W_WIN_BACKGROUND, &wd );
    p_send3( self, O_WN_VISIBLE, WV_INITVIS );
    p_send3( self, O_WN_EMPHASISE, TRUE );
    p_send2( self, O_WN_DODRAW );
    self->waribw.HiLighted = -1;
}

METHOD VOID waribw_wn_draw( PR_WARIBW* self )
{
    G_GC gc;
    G_SHADOW shadow;
    TEXT buf[2];
    int i;

    gc.style = G_STY_BOLD;
    gc.font = WS_FONT_BASE + 7;
    gSetGC( 0, G_GC_MASK_STYLE | G_GC_MASK_FONT, &gc );
    shadow.BodyColour = G_COLOUR_BLACK;
    shadow.ShadowColour = G_COLOUR_GREY;
    shadow.LightColour = G_COLOUR_NONE;
    shadow.filler = 0;
    shadow.Flags = 0;
    shadow.ShadowSizeX = 3;
    shadow.ShadowSizeY = 2;
    shadow.LightSizeX = 0;
    shadow.LightSizeY = 0;
    shadow.Spacing = 2;
    gShadowText( 18, 24, &shadow, "Wari", 4 );
    gc.style = G_STY_ITALIC;
    gc.font = WS_FONT_BASE + 4;
    gSetGC( 0, G_GC_MASK_STYLE | G_GC_MASK_FONT, &gc );
    gPrintText( 430, 12, "From", 4 );
    gc.font = WS_FONT_BASE + 6;
    gSetGC( 0, G_GC_MASK_STYLE | G_GC_MASK_FONT, &gc );
    gPrintText( 409, 27, "KizzyCo", 7 );
    DrawBoard();
    gc.style = G_STY_NORMAL;
    gc.font = WS_FONT_BASE + 8;
    gSetGC( 0, G_GC_MASK_STYLE | G_GC_MASK_FONT, &gc );
    for( i = 0, buf[1] = '/0' ; i < 6 ; i++ )
    {
        buf[0] = '1' + i;
        gPrintText( 102+54*i, 74, buf, 1 );
    }
}

METHOD INT waribw_wn_key( PR_WARIBW* self, INT keycode, INT modifiers )
{
    int cup, num, i;

    if( p_isalpha( keycode ) )
    {
        switch( p_tolower( keycode ) )
        {
        case 'n':
            p_send2( w_ws->wserv.com, O_COM_NEW );
            break;
        case 'u':
            p_send2( w_ws->wserv.com, O_COM_UNDO );
            break;
        case 'h':
            p_send2( w_ws->wserv.com, O_COM_HINT );
            break;
        case 'g':
            p_send3( w_ws->wserv.com, O_COM_GREY, O_COM_GREY );
            break;
        case 'w':
            p_send3( w_ws->wserv.com, O_COM_WHITE, O_COM_WHITE );
            break;
        case 'p':
            p_send3( w_ws->wserv.com, O_COM_2PLAYER, O_COM_2PLAYER );
            break;
        case 'b':
            p_send2( w_ws->wserv.com, O_COM_BEGINNER );
            break;
        case 'v':
            p_send2( w_ws->wserv.com, O_COM_NOVICE );
            break;
        case 'e':
            p_send2( w_ws->wserv.com, O_COM_EXPERT );
            break;
        case 'm':
            p_send2( w_ws->wserv.com, O_COM_MASTER );
            break;
        case 'c':
            p_send2( w_ws->wserv.com, O_COM_SOUND );
            break;
        case 's':
            p_send2( w_ws->wserv.com, O_COM_SPEED );
            break;
        case 'a':
            p_send2( w_ws->wserv.com, O_COM_ABOUT );
            break;
        }
        return WN_KEY_NO_CHANGE;
    }
    num = GetCupNumber( self->waribw.HiLighted );
    switch( keycode )
    {
    case '1': case '2': case '3': case '4': case '5': case '6':
        if( CheckState( CHK_IN_PROGRESS | CHK_END_GAME ) ) break;
        cup = GetCupIndex( keycode - '1' );
        p_send3( Game, O_WG_PLAY_CUP, cup );
        break;
    case W_KEY_LEFT:
        if( CheckState( CHK_IN_PROGRESS | CHK_END_GAME ) ) break;
        for( i = 0 ; i < 6 ; i++ )
        {
            --num;
            if( num < 0 ) num = 5;
            if( Game->game.Stones[ GetCupIndex( num ) ] )
            {
                break;
            }
        }
        if( i == 6 ) break;       /* This should be impossible */
        p_send3( self, O_WN_SET_HILIGHT, GetCupIndex( num ) );
        break;
    case W_KEY_RIGHT:
        if( CheckState( CHK_IN_PROGRESS | CHK_END_GAME ) ) break;
        for( i = 0 ; i < 6 ; i++ )
        {
            num++;
            if( num > 5 ) num = 0;
            if( Game->game.Stones[ GetCupIndex( num ) ] )
            {
                break;
            }
        }
        if( i == 6 ) break;       /* This should be impossible */
        p_send3( self, O_WN_SET_HILIGHT, GetCupIndex( num ) );
        break;
    case W_KEY_RETURN:
        if( CheckState( CHK_IN_PROGRESS | CHK_END_GAME ) ) break;
        if( self->waribw.HiLighted != -1 )
        {
            p_send3( Game, O_WG_PLAY_CUP, self->waribw.HiLighted );
        }
        break;
    case W_KEY_ESCAPE:
        /* remove hilight */
        p_send3( self, O_WN_SET_HILIGHT, -1 );
    }
    return WN_KEY_NO_CHANGE;
}

METHOD VOID waribw_wn_set_hilight( PR_WARIBW* self, INT cup )
{
    if( cup == self->waribw.HiLighted )
    {
        return;
    }
    wValidateWin( self->win.id );
    gCreateTempGC0( self->win.id );
    if( self->waribw.HiLighted != -1 )
    {
        DrawHiLight( self->waribw.HiLighted, DRAW_BGROUND );
    }
    if( cup != -1 )
    {
        DrawHiLight( cup, DRAW_BLACK );
    }
    gFreeTempGC();
    wFlush();
    self->waribw.HiLighted = cup;
}

METHOD VOID waribw_wn_update_cup( PR_WARIBW* self, INT cup, INT number )
{
    int beg, end, mode;

    wValidateWin( self->win.id );
    gCreateTempGC0( self->win.id );

    if( number > 0 )
    {
        beg = Game->game.Stones[cup];
        end = beg + number;
        mode = DRAW_BLACK;
    }
    else
    {
        end = Game->game.Stones[cup];
        beg = end + number;
        mode = DRAW_BGROUND;
    }
    while( beg != end )
    {
        DrawStone( cup, beg, mode );
        beg++;
    }

    gFreeTempGC();
    wFlush();
    Game->game.Stones[cup] += number;
}

METHOD VOID waribw_wn_set_score( PR_WARIBW* self )
{
    G_GC gc;
    static P_RECT rect12 = { { 23, 108 }, { 63, 130 } };
    static P_RECT rect13 = { { 415, 108 }, { 455, 130 } };
    TEXT buf[8];

    wValidateWin( self->win.id );
    gc.style = G_STY_BOLD;
    gc.font = WS_FONT_BASE + 7;
    gCreateTempGC( self->win.id, G_GC_MASK_STYLE | G_GC_MASK_FONT, &gc );

    p_atos( buf, "%d", Game->game.Stones[12] );
    gPrintBoxText( &rect12, 20, G_TEXT_ALIGN_CENTRE, 0, buf, p_slen(buf) );
    p_atos( buf, "%d", Game->game.Stones[13] );
    gPrintBoxText( &rect13, 20, G_TEXT_ALIGN_CENTRE, 0, buf, p_slen(buf) );

    gFreeTempGC();
    wFlush();
}

METHOD VOID statbw_wn_init( PR_STATBW* self )
{
    W_WINDATA wd;

    wd.extent.tl.x = WW_LEFT;
    wd.extent.tl.y = WW_STAT;
    wd.extent.width = WW_WIDTH;
    wd.extent.height = WW_STAT_HT;
    wd.background = W_WIN_BACK_CLR | W_WIN_BACK_GREY_CLR;
    p_send5( self, O_WN_CONNECT, NULL, W_WIN_EXTENT | W_WIN_BACKGROUND, &wd );
    self->statbw.idstate = STAT_GREY_TO_PLAY;
    self->statbw.idlevel = NOVICE_STR;
    p_send2( self, O_WN_DODRAW );
}

METHOD VOID statbw_wn_draw( PR_STATBW* self )
{
    TEXT buf[40];
    P_RECT rect[3] = {{{9,1},{159,19}},{{160,1},{319,19}},{{320,1},{469,19}}};

    p_supersend2( self, O_WN_DRAW );

    p_send4( w_am, O_AM_LOAD_RES_BUF, self->statbw.idstate, buf );
    gPrintBoxText( &rect[0], 14, G_TEXT_ALIGN_LEFT, 0, buf, p_slen(buf) );

    p_send4( w_am, O_AM_LOAD_RES_BUF, self->statbw.idlevel, buf );
    gPrintBoxText( &rect[1], 14, G_TEXT_ALIGN_CENTRE, 0, buf, p_slen(buf) );

    hAtos( buf, MOVE_STR, (Game->game.Moves+2)/2 );
    gPrintBoxText( &rect[2], 14, G_TEXT_ALIGN_RIGHT, 0, buf, p_slen(buf) );
}

METHOD VOID statbw_wn_set( PR_STATBW* self, INT strid )
{
    self->statbw.idstate = strid;
    p_send2( self, O_WN_DODRAW );
}

METHOD VOID statbw_sw_set_level( PR_STATBW* self, INT strid
)
{
    self->statbw.idlevel = strid;
    p_send2( self, O_WN_DODRAW );
}

/*-----------------------[ game - Wari Game Class ]-----------------------*/

METHOD VOID game_wg_init( PR_GAME* self )
{
    self->game.move = f_newsend( CAT_WARI_WARI, C_MOVE, O_AO_INIT );
    self->game.Level = LEVEL_NOVICE;
    self->game.SoundOn = TRUE;
    self->game.Delay = 4;
    self->game.InProgress = FALSE;
    self->game.swin = f_new( CAT_WARI_WARI, C_STATBW );
    p_send2( self->game.swin, O_WN_INIT );
    hInitVis( self->game.swin );
}

METHOD VOID game_wg_reset( PR_GAME* self, UINT play )
{
    int i;
    ULONG seed;

    if( self->game.Moves && self->game.Stones[12] + self->game.Stones[13] != 48 )
    {
        if( ! hConfirm( CONFIRM_QUIT_GAME ) ) return;
    }
    if( play )
    {
        self->game.PlayMode = play;
    }
    p_send4( w_ws->wserv.cli, O_WN_UPDATE_CUP, 12, -self->game.Stones[12] );
    p_send4( w_ws->wserv.cli, O_WN_UPDATE_CUP, 13, -self->game.Stones[13] );
    p_send2( w_ws->wserv.cli, O_WN_SET_SCORE );
    for( i = 0 ; i < 12 ; i++ )
    {
        p_send4( w_ws->wserv.cli, O_WN_UPDATE_CUP, i, 4-self->game.Stones[i] );
    }
    self->game.Moves = 0;
    switch( self->game.PlayMode )
    {
    case O_COM_GREY:
        self->game.GreyIs = HUMAN_PLAYER;
        self->game.WhiteIs = COMPUTER_PLAYER;
        break;
    case O_COM_WHITE:
        self->game.GreyIs = COMPUTER_PLAYER;
        self->game.WhiteIs = HUMAN_PLAYER;
        break;
    case O_COM_2PLAYER:
        self->game.GreyIs = HUMAN_PLAYER;
        self->game.WhiteIs = HUMAN_PLAYER;
        break;
    }
    self->game.TurnToPlay = GREY_SIDE;
    self->game.UndoOk = FALSE;
    p_send3( self->game.swin, O_WN_SET, STAT_GREY_TO_PLAY );
    if( self->game.GreyIs == COMPUTER_PLAYER )
    {
        /* make random first move */
        seed = p_date();
        i = (int) ( p_randl( &seed ) % 6 ) + 6;
        p_send3( self->game.move, O_MV_START, i );
    }
}

METHOD VOID game_wg_set_level( PR_GAME* self, UINT level, UINT id )
{
    self->game.Level = level;
    p_send3( self->game.swin, O_SW_SET_LEVEL, id );
}

METHOD VOID game_wg_play_cup( PR_GAME* self, INT cup )
{
    if( self->game.Stones[cup] > 0 )
    {
        p_bcpy( self->game.LastMove, self->game.Stones, sizeof(int)*14 );
        self->game.LastTurn = self->game.TurnToPlay;
        self->game.LastMoves = self->game.Moves;
        p_send3( self->game.move, O_MV_START, cup );
        self->game.UndoOk = TRUE;
    }
    else
    {
        hBeep();
        hInfoPrint( NO_STONES_TO_PLAY );
    }
}

METHOD VOID game_wg_play_next( PR_GAME* self )
{
    int cup;

    if( self->game.TurnToPlay == GREY_SIDE )
    {
        self->game.TurnToPlay = WHITE_SIDE;
        if( self->game.WhiteIs == COMPUTER_PLAYER )
        {
            p_send3( self->game.swin, O_WN_SET, STAT_THINKING );
            cup = GetWariMove( self->game.Stones, self->game.TurnToPlay,
                self->game.Level );
            p_send3( self->game.move, O_MV_START, cup );
            return;
        }
        p_send3( self->game.swin, O_WN_SET, STAT_WHITE_TO_PLAY );
    }
    else
    {
        self->game.TurnToPlay = GREY_SIDE;
        if( self->game.GreyIs == COMPUTER_PLAYER )
        {
            p_send3( self->game.swin, O_WN_SET, STAT_THINKING );
            cup = GetWariMove( self->game.Stones, self->game.TurnToPlay,
                self->game.Level );
            p_send3( self->game.move, O_MV_START, cup );
            return;
        }
        p_send3( self->game.swin, O_WN_SET, STAT_GREY_TO_PLAY );
    }
    self->game.InProgress = FALSE;
}

METHOD VOID game_wg_end_game( PR_GAME* self )
{
    int white, grey;
    INT winner;
    TEXT winbuf[40];
    END_RESULT result;
    DL_DATA data;

    p_send3( self->game.swin, O_WN_SET, STAT_GAME_OVER );
    white = self->game.Stones[12];
    grey = self->game.Stones[13];
    if( white == grey )
    {
        /* A draw */
        winner = MESS_A_DRAW;
    }
    else if( self->game.GreyIs + self->game.WhiteIs == 1 )
    {
        /* Computer is playing */
        if( ( self->game.GreyIs == COMPUTER_PLAYER && grey > 24 ) ||
            ( self->game.WhiteIs == COMPUTER_PLAYER && white > 24 ) )
        {
            winner = MESS_I_WIN;
        }
        else
        {
            winner = MESS_YOU_WIN;
        }
    }
    else
    {
        /* 2 Player mode */
        if( grey > 24 )
        {
            winner = MESS_GREY_WIN;
        }
        else
        {
            winner = MESS_WHITE_WIN;
        }
    }
    p_send( w_am, O_AM_LOAD_RES_BUF, winner, winbuf );
    result.winner = winbuf;
    data.id = ENDGAME_DIALOG;
    data.rbuf = &result;
    data.pdlg = NULL;
    hLaunchDial( CAT_WARI_WARI, C_ENDDLG, &data );
    self->game.InProgress = FALSE;
    if( *(WORD*) data.rbuf == W_KEY_RETURN )
    {
        p_send3( self, O_WG_RESET, 0 );
    }
}

/*----------------------------------------------------------------*/

METHOD VOID move_ao_init( PR_MOVE* self )
{
    p_send3( w_am, O_AM_ADD_TASK, self );
    p_supersend2( self, O_AO_INIT );
}

METHOD VOID move_mv_start( PR_MOVE* self, UINT cup )
{
    Game->game.InProgress = TRUE;
    p_send3( w_ws->wserv.cli, O_WN_SET_HILIGHT, cup );
    if( cup < 6 )
    {
        p_send3( Game->game.swin, O_WN_SET, STAT_WHITES_MOVE );
        self->move.side = 6;
        self->move.capture = 12;
    }
    else
    {
        p_send3( Game->game.swin, O_WN_SET, STAT_GREYS_MOVE );
        self->move.side = 0;
        self->move.capture = 13;
    }
    self->move.cup = cup;
    self->move.count = Game->game.Stones[cup];
    Game->game.Moves++;
    p_send4( w_ws->wserv.cli, O_WN_UPDATE_CUP, cup, -self->move.count );
    self->move.mode = DISTRIBUTE;
    p_send4( self, O_AO_QUEUE, Game->game.Delay, 0 );
}

METHOD INT move_ao_run( PR_MOVE* self )
{
    switch( self->move.mode )
    {
    case DISTRIBUTE:
        self->move.cup++;
        if( self->move.cup > 11 ) self->move.cup = 0;
        MakeClick( 1 );
        p_send4( w_ws->wserv.cli, O_WN_UPDATE_CUP, self->move.cup, 1 );
        --self->move.count;
        if( self->move.count == 0 )
        {
            self->move.mode = CAPTURE;
        }
        p_send4( self, O_AO_QUEUE, Game->game.Delay, 0 );
        break;
    case CAPTURE:
        if( self->move.cup >= self->move.side &&
            self->move.cup < self->move.side + 6 &&
            Game->game.Stones[self->move.cup] > 1 &&
            Game->game.Stones[self->move.cup] < 4 )
        {
            self->move.count = Game->game.Stones[self->move.cup];
            p_send4( w_ws->wserv.cli, O_WN_UPDATE_CUP, self->move.cup,
                -self->move.count );
            MakeClick( self->move.count );
            p_send4( w_ws->wserv.cli, O_WN_UPDATE_CUP, self->move.capture,
                self->move.count );
            p_send2( w_ws->wserv.cli, O_WN_SET_SCORE );
            --self->move.cup;
            p_send4( self, O_AO_QUEUE, Game->game.Delay, 0 );
            break;
        }
        if( ! IsEnd( self->move.side ) )
        {
            /* End of move */
            self->move.mode = MOVE_DONE;
            p_send3( w_ws->wserv.cli, O_WN_SET_HILIGHT, -1 );
            p_send2( Game, O_WG_PLAY_NEXT );
            break;
        }
        self->move.mode = ENDGAME;
        /* We don't need to worry about where the stones come from */
        self->move.cup = 0;
        /* fall through */
    case ENDGAME:
        while( self->move.cup < 12 &&
            Game->game.Stones[self->move.cup] == 0 )
        {
            self->move.cup++;
        }
        if( self->move.cup == 12 )
        {
            /* End of game */
            self->move.mode = MOVE_DONE;
            p_send3( w_ws->wserv.cli, O_WN_SET_HILIGHT, -1 );
            p_send2( Game, O_WG_END_GAME );
            break;
        }
        self->move.count = Game->game.Stones[self->move.cup];
        p_send4( w_ws->wserv.cli, O_WN_UPDATE_CUP, self->move.cup,
            -self->move.count );
        MakeClick( self->move.count );
        p_send4( w_ws->wserv.cli, O_WN_UPDATE_CUP, self->move.capture,
            self->move.count );
        p_send2( w_ws->wserv.cli, O_WN_SET_SCORE );
        p_send4( self, O_AO_QUEUE, Game->game.Delay, 0 );
        break;
    }
    return RUN_ACTIVE_USED;
}

/*-----------------------[ enddlg - Winner Dialog ]-----------------------*/

METHOD VOID enddlg_dl_dyn_init( PR_ENDDLG* self )
{
    END_RESULT* result;

    result = self->dlgbox.rbuf;
    hDlgSetText( 1, result->winner );
}


METHOD VOID sounddlg_dl_dyn_init( PR_SOUNDDLG* self )
{
    hDlgSetChlist( 1, Game->game.SoundOn );
}

METHOD INT sounddlg_dl_key( PR_SOUNDDLG* self, INT index, INT keycode,
    INT actbut )
{
    /* We only get here if Enter is pressed */
    Game->game.SoundOn = hDlgSenseChlist( 1 );
    return WN_KEY_CHANGED;
}

METHOD VOID speeddlg_dl_dyn_init( PR_SPEEDDLG* self )
{
    hDlgSetNcedit( 1, Game->game.Delay );
}

METHOD INT speeddlg_dl_key( PR_SPEEDDLG* self, INT index, INT keycode,
    INT actbut )
{
    /* We only get here if Enter is pressed */
    Game->game.Delay = hDlgSenseNcedit( 1 );
    return WN_KEY_CHANGED;
}

/* End of WARIMAIN.C file */

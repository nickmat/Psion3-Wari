#* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
#* Project: WA  WARI GAME FOR PSION 3a       *  Written by: Nick Matthews  *
#*  Module: WARI FOR PSION SIBOSDK/TOPSPEED  *  Date Started:  9 Jul 1996  *
#*    File: WARI.MAK        Type: MAKEFILE   *  Date Revised:  9 Jul 1996  *
#* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *


INC=e:\sibosdk\include
PRJ=wari

.c.obj:
  tsc $*.c /fp$(PRJ).pr

.cat.g:
  ctran $*.cat -e$(INC)\ -g -s -v
  copy $*.g $(INC)

.rss.rsc:
  rcomp $*.rss -o$*.rsc -i$(INC)\ -g$*.rsg -v

.rss.rsg:
  rcomp $*.rss -o$*.rsc -i$(INC)\ -g$*.rsg -v

.re.rg:
  rgprep $*.re $*.rex
  lprep -i$(INC)\ -o$*.i $*.rex
  del $*.rex
  rgcomp $*.i $*.rg
  del $*.i
  copy $*.rg $(INC)

.rsc.rzc:
  rchuf hwim -v -i$*.rsc -o$*.rzc

.rht.rhi:
  phcomp $*.rht -v -l

.ms.shd:
  makeshd $*.ms

.pcx.pic:
  wspcx -p $*.pcx

#######################################################################

wari.app : wari.img
  copy wari.img wari.app

wari.img : wari.obj warimain.obj waridraw.obj warimove.obj \
           wari.afl wari.pic wari.rzc wari.shd \
           wari.pr
  tsc /L wari.pr
  del wari.exe

wari.afl : wari.mak
  echo wari.pic>  wari.afl
  echo wari.rzc>> wari.afl
  echo wari.shd>> wari.afl

wari.ms : wari.mak
  echo Wari>  wari.ms
  echo.>>     wari.ms
  echo 1000>>    wari.ms

wari.plk : wari.mak
  echo wari24.pic>  wari.plk
  echo wari48.pic>> wari.plk

wari.pic : wari.plk wari24.pic wari48.pic
  wspcx -L wari.plk

wari24.pic : wari24.pcx
wari48.pic : wari48.pcx

wari.obj : wari.cat wari.g
  ctran $*.cat -e$(INC)\ -c -s -v
  tsc $*.c /fp$(PRJ).pr
  ecobj $*.obj

wari.g : wari.cat

warihelp.rhi : warihelp.rht

wari.rsg : wari.rss wari.rg warihelp.rhi

wari.rsc : wari.rss wari.rg warihelp.rhi

wari.rg : wari.re wari.g

wari.rzc : wari.rsc

wari.shd : wari.ms

warimain.obj : warimain.c wari.g wari.rsg wari.h warimove.h
waridraw.obj : waridraw.c wari.h
warimove.obj : warimove.c warimove.h

#-------------------------------[ Final zip ]------------------------------#

makezip : war_prel.zip

war_prel.zip : wari.app readme.txt
  if exist war_prel.zip del war_prel.zip
  pkzip war_prel.zip wari.app readme.txt

#------------------------------[ Backup zip ]------------------------------#


backup : waribu.rsp
  pkzip j:\backup\war_pbu.zip -u @waribu.rsp

waribu.rsp : wari.mak
  echo files.txt    >  waribu.rsp
  echo wari.mak     >> waribu.rsp
  echo wari.pr      >> waribu.rsp
  echo wari.cat     >> waribu.rsp
  echo wari.h       >> waribu.rsp
  echo warimain.c   >> waribu.rsp
  echo waridraw.c   >> waribu.rsp
  echo warimove.h   >> waribu.rsp
  echo warimove.c   >> waribu.rsp
  echo wari.re      >> waribu.rsp
  echo wari.rss     >> waribu.rsp
  echo warihelp.rht >> waribu.rsp
  echo wari24.pcx   >> waribu.rsp
  echo wari48.pcx   >> waribu.rsp
  echo readme.txt   >> waribu.rsp

# End of WARI.MAK file

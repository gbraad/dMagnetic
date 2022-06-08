#!/bin/sh

cat Pawn.rec | ./magtest.app /home/games/magneticscrolls/pawn.mag /home/games/magneticscrolls/pawn.gfx | tee runlog.log | egrep "(INST|MEMWRITE|RAND)"  >fullpawn.txt

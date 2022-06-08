#!/bin/sh

#(echo "say hello to kronos" ; echo "quit" ; echo "q" ; echo "y") | ./magtest.app /home/games/magneticscrolls/pawn.mag /home/games/magneticscrolls/pawn.gfx | tee runlog.log | egrep -a "(INST|MEMWRITE)" >mylog.txt
(echo "look at wristband" ; echo "quit" ; echo "q" ; echo "y") | ./magtest.app /home/games/magneticscrolls/pawn.mag /home/games/magneticscrolls/pawn.gfx | tee runlog.log | egrep -a "(INST|MEMWRITE)" >mylog.txt


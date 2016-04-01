Daniel Farley, dfarley@ucsc.edu

Heap size used: 2048 bytes

Summary:
    This lab brought together numerous aspects of previous labs as well as a new one: accessing 
    files.  These concepts were combined them in order to create a very basic text-based RPG game.
    With much less "guided" documentation in the form of flow charts or specific implantation 
    directions, this lab was very much about using our knowledge of the subject in order to 
    build the project ourselves.

Approach:
    After reading the manual and watching the video lecture on the file format, I started by 
    implementing Player.c and Game.c.  The former was misleadingly simple as there's no 
    inventory "management" to be done.  Game.c was a little more complex and ended up taking a 
    second iteration and multiple internal functions to work well.  The file format and 
    versioning seemed daunting to work with at first, but it didn't end up being very 
    complicated.  Breaking up the file access into multiple functions helped a lot, it made it 
    simpler to understand and eliminated the need to parse the entire file multiple times when 
    grabbing all the info.  The main() loop took the most time to implement, mostly because of 
    the string manipulation needed.  The state machine itself is reasonably simple, most of the 
    time spent on this lab was parsing the long Title+Description string into pieces that will 
    fit on the OLED.  I ended up taking the single long string and placing newline characters to 
    create "lines" which were less than 21 characters but also didn't split words.  Based on 
    which part of the string should be displayed the correct number of "lines" is then copied 
    into another string to be printed.  This ended up taking a lot of time due to difficulties 
    in debugging strings and correcting the various calculations.

Result:
    The project ended up working pretty well in the end and only took about 12 hours total.  
    The only part of the lab I disliked was the string manipulation, that can be tedious and 
    annoying.  The rest was both interesting and fun to implement; even the file access, which I 
    thought would be very confusing, turned out to be fairly simple to work with once you 
    understand it.  If there were a better alternative I think it would be nice to not deal with 
    the strings as much; perhaps just display things through the terminal.  Considering it's 
    supposed to be a final project the manual provided a decent amount of information, although 
    a more explicit description of the file layout (such as the one used in the lecture) would 
    have been appreciated.

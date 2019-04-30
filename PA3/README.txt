Makefile already runs the command line argument. Just type 
reset; make run 
to clear terminal, and run the line run.
The python file for some odd reason didn't find a certain python library
so I had to do a workaround and change the script a bit. Also the reading
from inputs was all messed up so the file reader names were changed.
Add valgrind to check for memory leakage in the makefile if necessary
Serviced gives the thread ID and the amount of times it serviced a file.
Results.txt gives the output of DNS lookup and files.
Based on what I have tested, and if my math is right, I have implemented mts correctly.

The general flow of the program is as follows:

initialize all the conditions and mutexs that were declared globally, but need to be set still. 

Open the output file to prepare for output

Create the four station Queues

take input from the provided input file, taken from the cmd line. 

close input file and start the clock

begin creating first the dispatcher thread, then the four station threads

The station threads manage the loading of trains, creating a thread for each, while the dispatcher waits untile the readyQueue has an item in it to begin sending trains across. 

The dispatcher thread uses a few different helper functions to support the ordering, and I tentatively think that they work. Allegedly. 

after this, we return to main and clean up any outstanding threads and or mutexes. we also close the output file. job done. 
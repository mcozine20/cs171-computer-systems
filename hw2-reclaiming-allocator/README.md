# HW2 Reclaiming Allocator
This is an allocator that reclaims free blocks of memory. It is an update on a simple pointer-bumping allocator and falls back to pointer-bumping when no reclaimed blocks are large enough to satisfy an allocation request. 
## What I was given
I was given a pointer-bumping allocator that allocated in a first-fit, address-ordered manner with no reclamation of free space. It had basic malloc, calloc, and realloc methods and an empty free method. It also had main and init methods, globals, constants, and macros.
## What I had to do
"Begin with the pointer-bumping allocator of Project 0. Here, allocation was performed in a first-fit, address-ordered manner with no reclamation of free space. It was simple, but it worked.
For this assignment, we want to alter that allocator. First, it must reclaim free space. Second, it must allocate from the collection of reclaimed blocks preferentially. That is, if the allocator can fulfill a request by using a block that was previously made free, then it must do so. Pointer bumping allocation should only be employed when no reclaimed blocks can satisfy the request." - Professor Scott F Kaplan
## How I did it
* Created a link struct to keep track of freed blocks of memory (lines 41-53)
* Put a pointer to the link that stores the first freed block in the Globals (line 70)
* Made malloc check the list of freed blocks to see if there is a block that will fulfill the request. If there is, use that block and remove it from the list (lines 121-165)
* Added content to free so that it is no longer an empty method. Specifically, it takes a pointer to a block, deallocates it, and adds it to the front of the list of freed blocks (lines 199-217)
* Rewrote main to allocate and free a number of different blocks in order to test the code (lines 307-324)
## Built With
* C
## Contributers
* Professor Scott F Kaplan
* McLean Cozine

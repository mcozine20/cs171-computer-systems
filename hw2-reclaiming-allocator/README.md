# HW2 Reclaiming Allocator
This is an allocator that reclaims free blocks of memory. It is an update on a simple pointer-bumping allocator and falls back to pointer-bumping when no reclaimed blocks are large enough to satisfy an allocation request. 
## What I was given

## What I had to do

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

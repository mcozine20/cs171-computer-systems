/**
 * pb-malloc.c
 *
 * A pointer-bumping, reclaiming memory allocator.
 **/
// =============================================================================



// =============================================================================
// INCLUDES

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <sys/mman.h>
// =============================================================================



// =============================================================================
// CONSTANTS AND MACRO FUNCTIONS

/* The system's page size. */
#define PAGE_SIZE sysconf(_SC_PAGESIZE)

/* Macros to easily calculate the number of bytes for larger scales. */
#define KB(size)  ((size_t)size * 1024)
#define MB(size)  (KB(size) * 1024)
#define GB(size)  (MB(size) * 1024)

/* The virtual address space reserved for the heap. */
#define HEAP_SIZE GB(2)
// =============================================================================



// =============================================================================
/**
 * Links are used to keep track of freed blocks. Each link struct keeps track of
 * the size of one block and a pointer to the link that corresponds to the next 
 * free block
 **/
typedef struct link {

  size_t size;
  struct link* next;

} link_s;
// =============================================================================



// =============================================================================
// GLOBALS

/* The current beginning of free heap space. */
static void*    free_ptr = NULL;

/* The beginning of the heap. */
static intptr_t start_ptr;

/* The end of the heap. */
static intptr_t end_ptr;

/* A pointer to the first block in the list of freed blocks. */
static link_s*  head = NULL;
// =============================================================================



// =============================================================================
/**
 * The initialization method. This runs if it is the first use of the heap.
 **/
void init () {

  /* Only do anything if the heap has not yet been used. */
  if (free_ptr == NULL) {

    /* Allocate the virtual address space for the heap. Kill the program if
       this mapping fails. */
    free_ptr = mmap(NULL, HEAP_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (free_ptr == MAP_FAILED) {
      write(STDOUT_FILENO, "mmap failed!\n", 13);
      exit(1);
    }

    /* The boundaries of the heap as a whole. */
    start_ptr = (intptr_t)free_ptr;
    end_ptr   = start_ptr + HEAP_SIZE;

    /* Put out a message to indicate that this allocator is being used. */
    write(STDOUT_FILENO, "pb!\n", 4);
    fsync(STDOUT_FILENO);

  }

  /* A debugging statement:
     printf("free pointer is:  %x\n", (intptr_t)free_ptr); */

} // init()
// =============================================================================



// =============================================================================
/**
 * Allocate and return 'size' bytes of heap space.
 *
 * \param size The number of bytes to allocate.
 * \return A pointer to the allocated block, if successful; otherwise 'NULL'
 **/
void* malloc (size_t size) {

  init();

  /* Only attempt to traverse the list of freed blocks if there are any
     freed blocks in the first place. */
  if (head != NULL) {

    link_s* previous_link = NULL;
    link_s* current_link = head;

    /* If the first freed block is large enough, reuse it. */
    if (size <= current_link -> size){

      /* Remove the block that is being reused from the list of freed blocks. */
      head = current_link -> next;
      
      /* A debugging statement:
	 printf("block starts: %x\n", (intptr_t)current_link + sizeof(size_t)); */ 
      
      /* Add the sizeof(size_t) so that the returned pointer starts after
	 the header */
      return (void*)((intptr_t)current_link + sizeof(size_t));
    }
    else {

      /* Traverse the list of freed blocks. */
      while (current_link -> next != NULL) {

	     previous_link = current_link;
	     current_link = current_link -> next;

	     /* When a block that is large enough is found, reuse it. */
	     if (size <= current_link -> size){
        
	      /* Remove the block that is being reused from the list of freed
	         blocks. */
	      previous_link -> next = current_link -> next;

	      /* A debugging statement:
	         printf("block starts: %x\n", (intptr_t)current_link + sizeof(size_t)); */

	       /* Add the sizeof(size_t) so that the returned pointer starts after
	          the header */
	       return (void*)((intptr_t)current_link + sizeof(size_t));
      	}
      }
    }
  }
  
  /* If there are no freed blocks, or none of them are big enough, allocate from
     the beginning of the free space. */

  /* Starting at the beginning of free space, mark where the header and the
     beginning of the allocated block will be. */
  size_t* header_ptr    = (size_t*)free_ptr;
  void*   new_block_ptr = (void*)((intptr_t)free_ptr + sizeof(size_t));

  /* Move the free pointer forward to the end of the newly allocated block. */
  free_ptr = (void*)((intptr_t)new_block_ptr + size);

  /* Set the header to store the block size (not including the header). */
  *header_ptr = size;

  /* A debugging statement:
     printf("block starts: %x\n", (intptr_t)new_block_ptr); */

  return new_block_ptr;
  
} // malloc()
// =============================================================================


// =============================================================================
/**
 * Deallocate a given block on the heap. Add the block to the front of the 
 * linked list of freed blocks.
 *
 * \param ptr A pointer to the block to be deallocated.
 **/
void free (void* ptr) {

  /* If the pointer is null, don't add it to the list of free blocks. */
  if (ptr == NULL)
    return;

  /* Go to the start of the header for the given block. */
  intptr_t header_addr = (intptr_t)ptr - sizeof(size_t);
 
 /* Make a link pointer for the block you are freeing, starting at the same
    place that the header starts at. Doing this allows the link's size field to
    be filled automatically because it is located at the same place in memory 
    that the header stored the size. */
  link_s* new_link_ptr = (link_s*)header_addr;

  /* Set the new link's next field to point to the link that the head pointer 
     currently points to. */
  new_link_ptr -> next = head;

  /* Set the head pointer to point to the new link. */
  head = new_link_ptr;

} // free()
// =============================================================================



// =============================================================================
/**
 * Allocate a block of 'nmemb * size' bytes, zeroing its contents.
 *
 * \param nmemb The number of elements in the new block.
 * \param size The size, in bytes, of each of the 'nmemb' elements.
 * \return A pointer to the newly allocated a zeroed block, or 'NULL' if
 *         unsuccessful.
 **/
void* calloc (size_t nmemb, size_t size) {

  /* Allocate a block of the requested size. */
  size_t block_size    = nmemb * size;
  void*  new_block_ptr = malloc(block_size);
  assert(new_block_ptr != NULL);

  /* Clear the entire block. */
  bzero(new_block_ptr, block_size);

  return new_block_ptr;
  
} // calloc()
// =============================================================================



// =============================================================================
/**
 * Update the block at 'ptr' to become the given 'size'. If size makes the block
 * smaller, then the block is returned unchanged. If size makes the block
 * larger, then a new and larger block is allocated, the data from the old 
 * block is copied, the old block is freed, and the new block is returned.
 *
 * \param ptr The block to be assigned a new size.
 * \param size The new size that the block should assume.
 * \return A pointer to the resultant block, which may be 'ptr' or may be a
 *         newly allocated block.
 **/
void* realloc (void* ptr, size_t size) {

  /* If there is no original block, just allocate a new one of the given size. */
  if (ptr == NULL) {
    return malloc(size);
  }

  /* If the new size is zero, it is the same as freeing the block. */
  if (size == 0) {
    free(ptr);
    return NULL;
  }

  /* Get the current block size. */
  size_t* header_ptr = (size_t*)((intptr_t)ptr - sizeof(size_t));
  size_t  block_size = *header_ptr;

  /* If the new size isn't bigger than the old, return the original block. */
  if (size <= block_size) {
    return ptr;
  }

  /* If the new size is bigger than the old, allocate a new and larger block,
     copy the old block into it, and free the old block. */
  void* new_block_ptr = malloc(size);
  assert(new_block_ptr != NULL);
  memcpy(new_block_ptr, ptr, block_size);
  free(ptr);
    
  return new_block_ptr;
  
} // realloc()
// =============================================================================



// =============================================================================
/**
 * The entry point if this program is compiled as a standalone program. 
 * Allocates and frees a series of blocks to test the code and assist in 
 * debugging.
 **/
#if !defined (PB_NO_MAIN)
void main () {

  void* first_block = malloc(45);
  free(first_block);
  void* new_first_block = malloc(30);
  void* second_block = malloc(10);
  void* third_block = malloc(20);
  void* fourth_block = malloc(75);
  free(third_block);
  void* new_block = malloc(12);
  void* newnew_block = malloc(5);
  free(new_first_block);
  free(second_block);
  free(fourth_block);
  free(new_block);
  free(newnew_block);
  void* extra_new_block = malloc(23);
  void* really_new_block = malloc(4);
  void* big_new_block = malloc(120);
  void* the_newest_block = malloc(2);
 
  
} // main()
// =============================================================================
#endif

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#include "bitmap.h"
#include "block_store.h"
// include more if you need


// You might find this handy. I put it around unused parameters, but you should
// remove it before you submit. Just allows things to compile initially.
#define UNUSED(x) (void)(x)

struct block_store
{
	bitmap_t *map;
	uint8_t data[BLOCK_STORE_NUM_BYTES];
};


block_store_t *block_store_create()
{
	block_store_t* bs = calloc(1, sizeof(block_store_t));
	if(bs == NULL)
		return NULL;
	/*bs->map = bitmap_create(BLOCK_STORE_NUM_BLOCKS);
	if(bs->map == NULL)
		return NULL;
	return bs;*/

	//found the rest of this function on github
	bs->map =bitmap_overlay(BITMAP_SIZE_BITS,bs->data+( BITMAP_START_BLOCK*BLOCK_SIZE_BYTES)); //creates
	if(bs->map == NULL)
		return NULL;
	for(size_t i = BITMAP_START_BLOCK; i < BITMAP_START_BLOCK +BITMAP_NUM_BLOCKS; i++)
	{
		block_store_request(bs, i);
	}
	return bs;
}

void block_store_destroy(block_store_t *const bs)
{
	if(bs == NULL)
		return;
	if (bs->map != NULL)
		bitmap_destroy(bs->map);

	free(bs);
}

//checkpoint 2
size_t block_store_allocate(block_store_t *const bs)
{
	//checks for valid parameter
	if(bs == NULL)
		return SIZE_MAX;
	if(bs->map == NULL)
		return SIZE_MAX;
	//finds the first free block
	size_t firstFreeBlock = bitmap_ffz(bs->map);
	if(firstFreeBlock == SIZE_MAX)
		return SIZE_MAX;
	//sets and tests that is was set correctly
	bitmap_set(bs->map, firstFreeBlock);
	if(bitmap_test(bs->map, firstFreeBlock) == 0)
		return SIZE_MAX;
	return firstFreeBlock;
}

// checkpoint 2
bool block_store_request(block_store_t *const bs, const size_t block_id)
{
	//test valid parameters
	if(bs == NULL || block_id >= BLOCK_STORE_NUM_BLOCKS)
		return false;
	if(bs->map == NULL)
		return false;
	//checks if the id is already allocated
	if(bitmap_test(bs->map, block_id))
		return false;
	//allocates the ID and checks it was done correctly
	bitmap_set(bs->map, block_id);
	if(bitmap_test(bs->map, block_id))
		return true;
	return false;
}

//checkpoint 2
void block_store_release(block_store_t *const bs, const size_t block_id)
{
	if(bs == NULL || block_id >= BLOCK_STORE_NUM_BLOCKS)
		return;
	if(bs->map == NULL)
		return;
	bitmap_reset(bs->map, block_id);
}

//checkpoint 3
size_t block_store_get_used_blocks(const block_store_t *const bs)
{
	if (bs == NULL || bs->map == NULL)
		return SIZE_MAX;

	size_t used_count = 0; 

	for (size_t i = 0; i < BLOCK_STORE_NUM_BLOCKS; ++i) 
	{
		if (bitmap_test(bs->map, i)) 
			used_count++;
	}

	return used_count;
}

//checkpoint 3
size_t block_store_get_free_blocks(const block_store_t *const bs)
{
	if (bs == NULL || bs->map == NULL)
		return SIZE_MAX;

	size_t free_count = 0; 

	for (size_t i = 0; i < BLOCK_STORE_NUM_BLOCKS; ++i) 
	{
		if (!bitmap_test(bs->map, i)) 
			free_count++;
	}

	return free_count;
}

//checkpoint 2
size_t block_store_get_total_blocks()
{
	return BITMAP_SIZE_BITS;
}

//checkpoint 3
size_t block_store_read(const block_store_t *const bs, const size_t block_id, void *buffer)
{
	if(bs == NULL || buffer == NULL)
		return 0;

	if (block_id >= BLOCK_STORE_NUM_BLOCKS)
		return 0;

	if(bs->map == NULL)
		return 0;

	size_t block_offset = block_id * BLOCK_SIZE_BYTES;

	memcpy(buffer, bs->data + block_offset, BLOCK_SIZE_BYTES);

	return BLOCK_SIZE_BYTES;
}


//writes data from buffer into a block
size_t block_store_write(block_store_t *const bs, const size_t block_id, const void *buffer)
{
		// Validate inputs
	if(bs == NULL || buffer == NULL)
		return 0;

	if(block_id >= BLOCK_STORE_NUM_BLOCKS)
		return 0;

	if(bs->map == NULL)
		return 0;

	// Calculate the starting position of the block
	size_t block_offset = block_id * BLOCK_SIZE_BYTES;

	// Copy data from buffer to block store
	memcpy(bs->data + block_offset, buffer, BLOCK_SIZE_BYTES);

	// Return number of bytes written
	return BLOCK_SIZE_BYTES;

}

block_store_t *block_store_deserialize(const char *const filename)
{
	if(filename == NULL)
		return NULL;
	int fd = open(filename, O_RDONLY);
	if(fd == -1)
		return NULL;
	block_store_t *bs = block_store_create();
	if(bs == NULL)
	{
		close(fd);
		return NULL;
	}
	ssize_t bytes_read = read(fd, bs->data, BLOCK_STORE_NUM_BYTES);
	close(fd);

	if(bytes_read != BLOCK_STORE_NUM_BYTES)
	{
		block_store_destroy(bs);
		return NULL;
	}

	bitmap_destroy(bs->map);
	bs->map = bitmap_overlay(BITMAP_SIZE_BITS, bs->data + (BITMAP_START_BLOCK * BLOCK_SIZE_BYTES));
	if(bs->map == NULL)
	{
		block_store_destroy(bs);
		return NULL;
	}

	return bs;

}

size_t block_store_serialize(const block_store_t *const bs, const char *const filename)
{
	// Validate inputs
	if(bs == NULL || filename == NULL)
		return 0;
	// Open file for writing (create if doesn't exist, truncate if exists)
	int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if(fd == -1)
		return 0;

	// Write the entire data array to file
	ssize_t bytes_written = write(fd, bs->data, BLOCK_STORE_NUM_BYTES);

	// Close the file
	close(fd);

	// Return number of bytes written
	if(bytes_written < 0)
		return 0;

	return (size_t)bytes_written;
}

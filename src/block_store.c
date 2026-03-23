#include <stdio.h>
#include <stdint.h>

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

size_t block_store_get_used_blocks(const block_store_t *const bs)
{
	UNUSED(bs);
	return 0;
}

size_t block_store_get_free_blocks(const block_store_t *const bs)
{
	UNUSED(bs);
	return 0;
}

//checkpoint 2
size_t block_store_get_total_blocks()
{
	return BITMAP_SIZE_BITS;
}

size_t block_store_read(const block_store_t *const bs, const size_t block_id, void *buffer)
{
	UNUSED(bs);
	UNUSED(block_id);
	UNUSED(buffer);
	return 0;
}

size_t block_store_write(block_store_t *const bs, const size_t block_id, const void *buffer)
{
	UNUSED(bs);
	UNUSED(block_id);
	UNUSED(buffer);
	return 0;
}

block_store_t *block_store_deserialize(const char *const filename)
{
	UNUSED(filename);
	return NULL;
}

size_t block_store_serialize(const block_store_t *const bs, const char *const filename)
{
	UNUSED(bs);
	UNUSED(filename);
	return 0;
}

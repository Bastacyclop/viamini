#include "bit_set.h"

typedef uint32_t Block;

#define BITS_PER_BLOCK (sizeof(Block)*8)
static const size_t bits_per_block = BITS_PER_BLOCK;
static const size_t max_block_index = BITS_PER_BLOCK - 1;
#undef BITS_PER_BLOCK

size_t blocks_for_bits(size_t bits);
size_t block_of(size_t value);
size_t block_bit_of(size_t value);
bool block_get(Block b, size_t i);
void block_set(Block* b, size_t i);
void block_clear(Block* b, size_t i);
void BitSet_grow_to(BitSet* set, size_t len);

size_t blocks_for_bits(size_t bits) {
    size_t blocks = bits / bits_per_block;
    if (bits % bits_per_block) {
        blocks++;
    }
    return blocks;
}

size_t block_of(size_t value) {
    return value / bits_per_block;
}

size_t block_bit_of(size_t value) {
    return value % bits_per_block;
}

bool block_get(Block b, size_t i) {
    return b & (1 << i);
}

void block_set(Block* b, size_t i) {
    *b |= (1 << i);
}

void block_clear(Block* b, size_t i) {
    *b &= ~(1 << i);
}

BitSet BitSet_new() {
    return (BitSet) {
        .storage = Vec_new(sizeof(Block)),
        .nbits = 0
    };
}

BitSet BitSet_with_capacity(size_t nbits) {
    return (BitSet) {
        .storage = Vec_with_capacity(blocks_for_bits(nbits), sizeof(Block)),
        .nbits = 0
    };
}

void BitSet_drop(BitSet* set) {
    Vec_plain_drop(&set->storage);
}

size_t BitSet_len(const BitSet* set) {
    size_t len = 0;

    size_t max_b = blocks_for_bits(set->nbits);
    if (max_b > 0) {
        size_t tail_b = max_b - 1;
        for (size_t b = 0; b < tail_b; b++) {
            const Block* block = Vec_unsafe_get(&set->storage, b);

            for (size_t i = 0; i < max_block_index; i++) {
                if (block_get(*block, i)) {
                    len++;
                }
            }
        }
        {
            const Block* tail = Vec_unsafe_get(&set->storage, tail_b);
            size_t tail_bits = set->nbits % bits_per_block;

            for (size_t i = 0; i < tail_bits; i++) {
                if (block_get(*tail, i)) {
                    len++;
                }
            }
        }
    }

    return len;
}

bool BitSet_is_empty(const BitSet* set) {
    return BitSet_len(set) == 0;
}

size_t BitSet_capacity(const BitSet* set) {
    return Vec_capacity(&set->storage)*bits_per_block;
}

void BitSet_grow_to(BitSet* set, size_t nbits) {
    size_t old_blocks = Vec_len(&set->storage);
    size_t blocks = blocks_for_bits(nbits);
    Vec_reserve_len(&set->storage, blocks);

    if (blocks > old_blocks) {
        Block* new_begin = Vec_unsafe_get_mut(&set->storage, old_blocks);
        memset(new_begin, 0, sizeof(Block)*(blocks - old_blocks));
    }

    set->storage.len = blocks;
    set->nbits = nbits;
}

bool BitSet_insert(BitSet* set, size_t value) {
    if (value >= set->nbits) {
        BitSet_grow_to(set, value + 1);
    }

    size_t b = block_of(value);
    size_t i = block_bit_of(value);
    Block* block = Vec_unsafe_get_mut(&set->storage, b);

    bool insert = !block_get(*block, i);
    block_set(block, i); // if (insert) ?
    return insert;
}

bool BitSet_contains(const BitSet* set, size_t value) {
    if (value >= set->nbits) return false;

    size_t b = block_of(value);
    size_t i = block_bit_of(value);
    Block block = *(const Block*)Vec_unsafe_get(&set->storage, b);

    return block_get(block, i);
}

bool BitSet_remove(BitSet* set, size_t value) {
    if (value >= set->nbits) return false;

    size_t b = block_of(value);
    size_t i = block_bit_of(value);
    Block* block = Vec_unsafe_get_mut(&set->storage, b);

    bool remove = block_get(*block, i);
    block_clear(block, i); // if (remove) ?
    return remove;
}

void BitSet_clear(BitSet* set) {
    Vec_plain_clear(&set->storage);
    set->nbits = 0;
}

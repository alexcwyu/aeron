#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stdlib.h>
#include "collections/aeron_int64_to_ptr_hash_map.h"
int test_assembler_map_init(aeron_int64_to_ptr_hash_map_t *, size_t, float);
int test_assembler_map_put(aeron_int64_to_ptr_hash_map_t *, int64_t, void *);
int test_assembler_alloc(void **, size_t);
void test_assembler_free(void *);
int test_assembler_reallocf(void **, size_t);
#define aeron_alloc test_assembler_alloc
#define aeron_free test_assembler_free
#define aeron_reallocf test_assembler_reallocf
#define aeron_int64_to_ptr_hash_map_init test_assembler_map_init
#define aeron_int64_to_ptr_hash_map_put test_assembler_map_put
#include "aeron_fragment_assembler.c"
#undef aeron_alloc
#undef aeron_free
#undef aeron_reallocf
#undef aeron_int64_to_ptr_hash_map_init
#undef aeron_int64_to_ptr_hash_map_put

static int allocations;
static int calls;
static int fail_at;
static bool real_map_init;
static bool fail_map_put;
int test_assembler_alloc(void **ptr, size_t size)
{
    if (++calls == fail_at)
    {
        AERON_SET_ERR(ENOMEM, "%s", "injected assembler allocation failure");
        return -1;
    }
    *ptr = calloc(1, size);
    assert(NULL != *ptr);
    allocations++;
    return 0;
}
void test_assembler_free(void *ptr)
{
    if (NULL != ptr)
    {
        allocations--;
        free(ptr);
    }
}
int test_assembler_map_init(aeron_int64_to_ptr_hash_map_t *map, size_t capacity, float load_factor)
{
    if (real_map_init)
    {
        return aeron_int64_to_ptr_hash_map_init(map, capacity, load_factor);
    }
    AERON_SET_ERR(ENOMEM, "%s", "injected assembler map failure");
    return -1;
}
int test_assembler_map_put(aeron_int64_to_ptr_hash_map_t *map, int64_t key, void *value)
{
    int result = aeron_int64_to_ptr_hash_map_put(map, key, value);
    if (fail_map_put)
    {
        map->resize_threshold = (size_t)((float)(map->capacity << 1) * map->load_factor);
        AERON_SET_ERR(ENOMEM, "%s", "injected post-insertion rehash failure");
        return -1;
    }
    return result;
}
int test_assembler_reallocf(void **ptr, size_t size)
{
    test_assembler_free(*ptr);
    *ptr = NULL;
    AERON_SET_ERR(ENOMEM, "%s", "injected growth failure");
    return -1;
}
static int deliveries;
static void verify_message(void *clientd, const uint8_t *buffer, size_t length, aeron_header_t *header)
{
    assert(4104 == length);
    for (size_t i = 0; i < 4096; i++)
    {
        assert(0x17 == buffer[i]);
    }
    for (size_t i = 4096; i < length; i++)
    {
        assert(0x29 == buffer[i]);
    }
    deliveries++;
}
static aeron_controlled_fragment_handler_action_t verify_controlled_message(
    void *clientd, const uint8_t *buffer, size_t length, aeron_header_t *header)
{
    verify_message(clientd, buffer, length, header);
    return AERON_ACTION_CONTINUE;
}
static aeron_controlled_fragment_handler_action_t dispatch(
    int kind, void *assembler, const uint8_t *buffer, size_t length, aeron_header_t *header)
{
    switch (kind)
    {
        case 0: aeron_image_fragment_assembler_handler(assembler, buffer, length, header); break;
        case 1: return aeron_image_controlled_fragment_assembler_handler(assembler, buffer, length, header);
        case 2: aeron_fragment_assembler_handler(assembler, buffer, length, header); break;
        default: return aeron_controlled_fragment_assembler_handler(assembler, buffer, length, header);
    }
    return AERON_ACTION_CONTINUE;
}
static aeron_buffer_builder_t *builder_for(int kind, void *assembler)
{
    switch (kind)
    {
        case 0: return ((aeron_image_fragment_assembler_t *)assembler)->buffer_builder;
        case 1: return ((aeron_image_controlled_fragment_assembler_t *)assembler)->buffer_builder;
        case 2: return aeron_int64_to_ptr_hash_map_get(
            &((aeron_fragment_assembler_t *)assembler)->builder_by_session_id_map, 7);
        default: return aeron_int64_to_ptr_hash_map_get(
            &((aeron_controlled_fragment_assembler_t *)assembler)->builder_by_session_id_map, 7);
    }
}
static void handler_failure(int kind, bool fail_begin, bool map_failure, bool fail_middle)
{
    void *assembler = NULL;
    real_map_init = true;
    fail_at = 0;
    deliveries = 0;
    switch (kind)
    {
        case 0: assert(0 == aeron_image_fragment_assembler_create(
            (aeron_image_fragment_assembler_t **)&assembler, verify_message, NULL)); break;
        case 1: assert(0 == aeron_image_controlled_fragment_assembler_create(
            (aeron_image_controlled_fragment_assembler_t **)&assembler, verify_controlled_message, NULL)); break;
        case 2: assert(0 == aeron_fragment_assembler_create(
            (aeron_fragment_assembler_t **)&assembler, verify_message, NULL)); break;
        default: assert(0 == aeron_controlled_fragment_assembler_create(
            (aeron_controlled_fragment_assembler_t **)&assembler, verify_controlled_message, NULL)); break;
    }
    uint8_t begin[4096], end[8];
    memset(begin, 0x17, sizeof(begin));
    memset(end, 0x29, sizeof(end));
    aeron_data_header_t frame = { 0 };
    frame.frame_header.flags = AERON_DATA_HEADER_BEGIN_FLAG;
    frame.frame_header.frame_length = AERON_DATA_HEADER_LENGTH + sizeof(begin);
    frame.session_id = 7;
    aeron_header_t header = { 0 };
    header.frame = &frame;
    header.position_bits_to_shift = 16;
    int before = allocations;
    if (map_failure)
    {
        fail_map_put = true;
    }
    else if (fail_begin)
    {
        fail_at = calls + (kind >= 2 ? 3 : 1);
    }
    aeron_controlled_fragment_handler_action_t action = dispatch(kind, assembler, begin, sizeof(begin), &header);
    if (fail_begin || map_failure)
    {
        assert((kind % 2 ? AERON_ACTION_ABORT : AERON_ACTION_CONTINUE) == action);
        assert(0 == deliveries);
        if (map_failure)
        {
            aeron_int64_to_ptr_hash_map_t *map = kind == 2 ?
                &((aeron_fragment_assembler_t *)assembler)->builder_by_session_id_map :
                &((aeron_controlled_fragment_assembler_t *)assembler)->builder_by_session_id_map;
            assert(NULL == builder_for(kind, assembler));
            assert((size_t)((float)map->capacity * map->load_factor) == map->resize_threshold);
            assert(before == allocations);
            fail_map_put = false;
        }
        else
        {
            assert(0 == builder_for(kind, assembler)->limit);
            assert(-1 == builder_for(kind, assembler)->next_term_offset);
        }
        fail_at = 0;
        assert(AERON_ACTION_CONTINUE == dispatch(kind, assembler, begin, sizeof(begin), &header));
    }
    frame.term_offset = (int32_t)(AERON_DATA_HEADER_LENGTH + sizeof(begin));
    frame.frame_header.flags = fail_middle ? 0 : AERON_DATA_HEADER_END_FLAG;
    frame.frame_header.frame_length = AERON_DATA_HEADER_LENGTH + sizeof(end);
    if (!fail_begin && !map_failure)
    {
        fail_at = calls + 1;
        action = dispatch(kind, assembler, end, sizeof(end), &header);
        assert((kind % 2 ? AERON_ACTION_ABORT : AERON_ACTION_CONTINUE) == action);
        assert(0 == deliveries);
        aeron_buffer_builder_t *builder = builder_for(kind, assembler);
        assert((kind % 2 ? sizeof(begin) : 0) == builder->limit);
        assert((kind % 2 ? frame.term_offset : -1) == builder->next_term_offset);
        fail_at = 0;
        if (!(kind % 2))
        {
            dispatch(kind, assembler, end, sizeof(end), &header);
            assert(0 == deliveries);
            frame.term_offset = 0;
            frame.frame_header.flags = AERON_DATA_HEADER_BEGIN_FLAG;
            frame.frame_header.frame_length = AERON_DATA_HEADER_LENGTH + sizeof(begin);
            dispatch(kind, assembler, begin, sizeof(begin), &header);
            frame.term_offset = (int32_t)(AERON_DATA_HEADER_LENGTH + sizeof(begin));
            frame.frame_header.flags = fail_middle ? 0 : AERON_DATA_HEADER_END_FLAG;
            frame.frame_header.frame_length = AERON_DATA_HEADER_LENGTH + sizeof(end);
        }
    }
    assert(AERON_ACTION_CONTINUE == dispatch(kind, assembler, end, sizeof(end), &header));
    if (fail_middle)
    {
        assert(0 == deliveries);
        frame.term_offset = aeron_header_next_term_offset(&header);
        frame.frame_header.flags = AERON_DATA_HEADER_END_FLAG;
        frame.frame_header.frame_length = AERON_DATA_HEADER_LENGTH;
        assert(AERON_ACTION_CONTINUE == dispatch(kind, assembler, end, 0, &header));
    }
    assert(1 == deliveries);
    dispatch(kind, assembler, end, sizeof(end), &header);
    assert(1 == deliveries);
    switch (kind)
    {
        case 0: aeron_image_fragment_assembler_delete(assembler); break;
        case 1: aeron_image_controlled_fragment_assembler_delete(assembler); break;
        case 2: aeron_fragment_assembler_delete(assembler); break;
        default: aeron_controlled_fragment_assembler_delete(assembler); break;
    }
    assert(0 == allocations);
}
int main(int argc, char **argv)
{
    if (argc > 2 && 0 == strncmp(argv[2], "handler-", 8))
    {
        handler_failure(atoi(argv[1]), 0 == strcmp(argv[2], "handler-begin"),
            0 == strcmp(argv[2], "handler-map"), 0 == strcmp(argv[2], "handler-middle"));
        return 0;
    }
    fail_at = argc > 1 ? atoi(argv[1]) : 3;
    if (argc > 2 && strcmp(argv[2], "growth") == 0)
    {
        aeron_buffer_builder_t *builder = NULL;
        assert(0 == aeron_buffer_builder_create(&builder));
        assert(0 == test_assembler_alloc((void **)&builder->buffer, 4096));
        builder->buffer_length = 4096;
        builder->limit = 8;
        memset(builder->buffer, 7, builder->limit);
        uint8_t *original = builder->buffer;
        fail_at = calls + 1;
        assert(-1 == aeron_buffer_builder_ensure_capacity(builder, 8192));
        assert(original == builder->buffer);
        assert(4096 == builder->buffer_length && 8 == builder->limit && 7 == builder->buffer[0]);
        aeron_buffer_builder_delete(builder);
    }
    else if (argc > 2 && strcmp(argv[2], "map") == 0)
    {
        aeron_fragment_assembler_t *assembler = NULL;
        assert(-1 == aeron_fragment_assembler_create(&assembler, NULL, NULL));
        assert(NULL == assembler);
    }
    else if (argc > 2 && strcmp(argv[2], "controlled-map") == 0)
    {
        aeron_controlled_fragment_assembler_t *assembler = NULL;
        assert(-1 == aeron_controlled_fragment_assembler_create(&assembler, NULL, NULL));
        assert(NULL == assembler);
    }
    else if (argc > 2)
    {
        aeron_image_controlled_fragment_assembler_t *assembler = NULL;
        assert(-1 == aeron_image_controlled_fragment_assembler_create(&assembler, NULL, NULL));
        assert(NULL == assembler);
    }
    else
    {
        aeron_image_fragment_assembler_t *assembler = NULL;
        assert(-1 == aeron_image_fragment_assembler_create(&assembler, NULL, NULL));
        assert(NULL == assembler);
    }
    assert(ENOMEM == aeron_errcode());
    assert(0 == allocations);
    return 0;
}

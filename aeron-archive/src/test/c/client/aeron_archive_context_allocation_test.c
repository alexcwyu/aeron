#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#define aeron_alloc test_archive_alloc
#define aeron_reallocf test_archive_reallocf
#include "client/aeron_archive_context.c"
#undef aeron_alloc
#undef aeron_reallocf

int aeron_alloc(void **ptr, size_t size);
int aeron_reallocf(void **ptr, size_t size);
static bool fail_allocation;

int test_archive_alloc(void **ptr, size_t size)
{
    if (fail_allocation)
    {
        AERON_SET_ERR(ENOMEM, "%s", "injected archive allocation failure");
        return -1;
    }
    return aeron_alloc(ptr, size);
}

int test_archive_reallocf(void **ptr, size_t size)
{
    if (fail_allocation)
    {
        aeron_free(*ptr);
        *ptr = NULL;
        AERON_SET_ERR(ENOMEM, "%s", "injected archive allocation failure");
        return -1;
    }
    return aeron_reallocf(ptr, size);
}

int main(int argc, char **argv)
{
    int (*setters[])(aeron_archive_context_t *, const char *) = {
        aeron_archive_context_set_control_request_channel,
        aeron_archive_context_set_control_response_channel,
        aeron_archive_context_set_recording_events_channel
    };
    const char *(*getters[])(aeron_archive_context_t *) = {
        aeron_archive_context_get_control_request_channel,
        aeron_archive_context_get_control_response_channel,
        aeron_archive_context_get_recording_events_channel
    };
    int first = argc > 1 ? atoi(argv[1]) : 0;
    int limit = argc > 1 ? first + 1 : 3;
    assert(first >= 0 && limit <= 3);
    for (int i = first; i < limit; i++)
    {
        aeron_archive_context_t *ctx = NULL;
        assert(0 == aeron_archive_context_init(&ctx));
        assert(0 == setters[i](ctx, "aeron:ipc"));
        fail_allocation = true;
        assert(-1 == setters[i](ctx, "aeron:udp?endpoint=localhost:8090"));
        assert(ENOMEM == aeron_errcode());
        fail_allocation = false;
        assert(0 == strcmp("aeron:ipc", getters[i](ctx)));
        assert(0 == aeron_archive_context_close(ctx));
    }
    return EXIT_SUCCESS;
}

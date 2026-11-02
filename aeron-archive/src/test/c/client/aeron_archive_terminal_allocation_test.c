#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stdlib.h>

#define aeron_alloc test_terminal_alloc
#define aeron_free test_terminal_free
#include "aeron_fragment_assembler.c"
#include "client/aeron_archive_recording_descriptor_poller.c"
#include "client/aeron_archive_recording_subscription_descriptor_poller.c"
#include "client/aeron_archive_async_connect.c"
#include "client/aeron_archive_client.c"
#undef aeron_alloc
#undef aeron_free

static void *allocations[16];
static int allocation_count;
static int call_count;
static int fail_at;

int test_terminal_alloc(void **ptr, size_t size)
{
    if (++call_count == fail_at)
    {
        AERON_SET_ERR(ENOMEM, "%s", "injected terminal allocation failure");
        return -1;
    }
    *ptr = calloc(1, size);
    assert(NULL != *ptr && allocation_count < 16);
    allocations[allocation_count++] = *ptr;
    return 0;
}

void test_terminal_free(void *ptr)
{
    if (NULL == ptr)
    {
        return;
    }
    for (int i = 0; i < allocation_count; i++)
    {
        if (allocations[i] == ptr)
        {
            allocations[i] = allocations[--allocation_count];
            free(ptr);
            return;
        }
    }
    assert(false && "unknown or already freed allocation");
}

int main(int argc, char **argv)
{
    int first = argc > 1 ? atoi(argv[1]) : 1;
    int limit = argc > 1 ? first + 1 : 6;
    assert(first >= 1 && limit <= 6);
    for (int failure = first; failure < limit; failure++)
    {
        fail_at = 0;
        aeron_archive_async_connect_t *async = NULL;
        assert(0 == test_terminal_alloc((void **)&async, sizeof(*async)));
        call_count = 0;
        fail_at = failure;
        aeron_archive_t *archive = NULL;
        assert(-1 == aeron_archive_async_connect_transition_to_done(&archive, async, 7));
        assert(NULL == archive && ENOMEM == aeron_errcode());
        assert(0 == allocation_count);
    }
    return EXIT_SUCCESS;
}

#include <stdlib.h>
#include <stdio.h>

/* Inject failures only into this translation unit, leaving the production allocator unchanged. */
#define aeron_alloc test_alloc
#define aeron_free test_free
#include "aeron_congestion_control.c"
#undef aeron_alloc
#undef aeron_free

static int allocation_count;
static int fail_at;
static int free_count;
static void *allocated;

int test_alloc(void **ptr, size_t size)
{
    if (++allocation_count == fail_at)
    {
        AERON_SET_ERR(ENOMEM, "%s", "injected allocation failure");
        return -1;
    }

    allocated = calloc(1, size);
    *ptr = allocated;
    return NULL == allocated ? -1 : 0;
}

void test_free(void *ptr)
{
    if (ptr != allocated)
    {
        fprintf(stderr, "supplier freed caller output slot instead of its allocation\n");
        abort();
    }
    free_count++;
    free(ptr);
    allocated = NULL;
}

int main(void)
{
    for (fail_at = 1; fail_at <= 2; fail_at++)
    {
        aeron_congestion_control_strategy_t *strategy = NULL;
        allocation_count = 0;
        free_count = 0;
        if (-1 != aeron_cubic_congestion_control_strategy_supplier(
            &strategy, NULL, 1, 2, 3, 65536, 1408, NULL, NULL, NULL, NULL) ||
            NULL != strategy || NULL != allocated || free_count != fail_at - 1 || ENOMEM != aeron_errcode())
        {
            fprintf(stderr, "allocation failure did not preserve output, cleanup, and error contract\n");
            return EXIT_FAILURE;
        }
    }
    return EXIT_SUCCESS;
}

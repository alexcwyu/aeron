#define aeron_alloc test_conductor_alloc
#define aeron_reallocf test_conductor_reallocf
#include "aeron_client_conductor.c"
extern int aeron_array_ensure_capacity(
    uint8_t **array, size_t element_size, size_t old_capacity, size_t new_capacity);
#undef aeron_alloc
#undef aeron_reallocf

int aeron_alloc(void **ptr, size_t size);
int aeron_reallocf(void **ptr, size_t size);
bool test_conductor_fail_allocations = false;
int test_conductor_allocation_failures = 0;

int test_conductor_alloc(void **ptr, size_t size)
{
    if (test_conductor_fail_allocations)
    {
        test_conductor_allocation_failures++;
        AERON_SET_ERR(ENOMEM, "%s", "injected conductor allocation failure");
        return -1;
    }
    return aeron_alloc(ptr, size);
}

int test_conductor_reallocf(void **ptr, size_t size)
{
    if (test_conductor_fail_allocations)
    {
        test_conductor_allocation_failures++;
        aeron_free(*ptr);
        *ptr = NULL;
        AERON_SET_ERR(ENOMEM, "%s", "injected conductor allocation failure");
        return -1;
    }
    return aeron_reallocf(ptr, size);
}

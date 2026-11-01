#include <array>
#include <cstring>
#include <gtest/gtest.h>

extern "C"
{
#include "aeron_alloc.h"
#include "aeron_driver_sender.h"
#include "aeron_driver_sender_proxy.h"
#include "concurrent/aeron_spsc_rb.h"
#include "uri/aeron_uri.h"
}

namespace
{
bool command_executed = false;
void ordinary_command(void *, void *) { command_executed = true; }
void discard(int32_t, const void *, size_t, void *) {}
}

TEST(SenderShutdownTest, shouldDiscardOwningDestinationCommandsAcrossWrapPaddingWithoutExecutingWork)
{
    alignas(64) std::array<uint8_t, 2048 + AERON_RB_TRAILER_LENGTH> storage = {};
    aeron_spsc_rb_t queue = {};
    ASSERT_EQ(0, aeron_spsc_rb_init(&queue, storage.data(), storage.size()));
    aeron_driver_sender_t sender = {};
    sender.sender_proxy.command_queue = &queue;
    aeron_command_base_t ordinary = {};
    ordinary.func = ordinary_command;
    command_executed = false;

    // Leave less than a destination record at the end, then consume the prefix. The next write wraps.
    const size_t record_length = AERON_ALIGN(sizeof(ordinary) + AERON_RB_RECORD_HEADER_LENGTH, AERON_RB_ALIGNMENT);
    const size_t record_count = (2048 - 56) / record_length;
    for (size_t i = 0; i < record_count; i++)
    {
        ASSERT_EQ(AERON_RB_SUCCESS, aeron_spsc_rb_write(&queue, 1, &ordinary, sizeof(ordinary)));
    }
    ASSERT_EQ(record_count, aeron_spsc_rb_read(&queue, discard, nullptr, SIZE_MAX));
    ASSERT_LT(2048 - aeron_spsc_rb_consumer_position(&queue), sizeof(aeron_command_destination_t));

    aeron_uri_t *uri = nullptr;
    ASSERT_EQ(0, aeron_alloc(reinterpret_cast<void **>(&uri), sizeof(*uri)));
    const char *channel = "aeron:udp?endpoint=localhost:40123|alias=shutdown-test";
    ASSERT_EQ(0, aeron_uri_parse(std::strlen(channel), channel, uri));
    aeron_command_destination_t destination = {};
    destination.base.func = aeron_driver_sender_on_add_destination;
    destination.uri = uri;
    ASSERT_EQ(AERON_RB_SUCCESS, aeron_spsc_rb_write(&queue, 1, &destination, sizeof(destination)));
    ASSERT_EQ(AERON_RB_SUCCESS, aeron_spsc_rb_write(&queue, 1, &ordinary, sizeof(ordinary)));
    const int64_t producer_position = aeron_spsc_rb_producer_position(&queue);
    ASSERT_GT(producer_position, 2048);

    aeron_driver_sender_drain_commands_for_shutdown(&sender);
    EXPECT_EQ(producer_position, aeron_spsc_rb_consumer_position(&queue));
    EXPECT_FALSE(command_executed);
    // A second drain must not revisit the URI or execute callbacks (ASAN catches duplicate release).
    aeron_driver_sender_drain_commands_for_shutdown(&sender);
    EXPECT_EQ(producer_position, aeron_spsc_rb_consumer_position(&queue));
    EXPECT_FALSE(command_executed);
}

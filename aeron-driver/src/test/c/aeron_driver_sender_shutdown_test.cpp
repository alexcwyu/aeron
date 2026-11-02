#include <array>
#include <cstring>
#include <thread>
#include <chrono>
#include <gtest/gtest.h>

extern "C"
{
#include "aeron_alloc.h"
#include "aeron_driver.h"
#include "aeron_driver_sender.h"
#include "aeron_driver_sender_proxy.h"
#include "concurrent/aeron_spsc_rb.h"
#include "uri/aeron_uri.h"
}

namespace
{
struct StartupProbe
{
    aeron_driver_t *driver;
    int starts = 0;
    int work = 0;
    bool in_start = false;
    bool ordered = true;
    std::thread::id thread;
};

void on_sender_start(void *state, const char *)
{
    auto *probe = static_cast<StartupProbe *>(state);
    probe->starts++;
    probe->thread = std::this_thread::get_id();
    probe->in_start = true;
    EXPECT_EQ(0, aeron_driver_sender_invoke(probe->driver));
    probe->in_start = false;
}

int startup_work(void *state)
{
    auto *probe = static_cast<StartupProbe *>(state);
    probe->ordered &= 1 == probe->starts && !probe->in_start;
    probe->work++;
    return 0;
}

int saturate_sender(void *state)
{
    auto *proxy = static_cast<aeron_driver_sender_proxy_t *>(state);
    aeron_command_base_t command = {};
    while (AERON_RB_SUCCESS == aeron_spsc_rb_write(proxy->command_queue, 1, &command, sizeof(command)))
    {
    }
    aeron_uri_t *uri = nullptr;
    if (aeron_alloc(reinterpret_cast<void **>(&uri), sizeof(*uri)) < 0)
    {
        return -1;
    }
    const char *channel = "aeron:udp?endpoint=localhost:40123|alias=cancelled";
    EXPECT_EQ(0, aeron_uri_parse(std::strlen(channel), channel, uri));
    sockaddr_storage address = {};
    aeron_driver_sender_proxy_on_add_destination(proxy, nullptr, uri, &address, 17);
    return 0;
}
}

TEST(SenderShutdownTest, shouldStartInvokedSenderOnceOnHostThreadBeforeWork)
{
    aeron_driver_t driver = {};
    aeron_driver_context_t context = {};
    context.threading_mode = AERON_THREADING_MODE_DEDICATED_SENDER_INVOKED;
    driver.context = &context;
    StartupProbe probe = {};
    probe.driver = &driver;
    auto *runner = &driver.runners[AERON_AGENT_RUNNER_SENDER];
    ASSERT_EQ(0, aeron_agent_init(runner, "sender", &probe, on_sender_start, &probe,
        startup_work, nullptr, aeron_idle_strategy_noop_idle, nullptr));
    ASSERT_EQ(0, aeron_driver_start(&driver, true));
    EXPECT_EQ(0, probe.starts);
    std::thread host([&]()
    {
        EXPECT_EQ(0, aeron_driver_sender_invoke(&driver));
        EXPECT_EQ(0, aeron_driver_sender_invoke(&driver));
        EXPECT_EQ(std::this_thread::get_id(), probe.thread);
    });
    host.join();
    EXPECT_EQ(1, probe.starts);
    EXPECT_EQ(2, probe.work);
    EXPECT_TRUE(probe.ordered);
    EXPECT_EQ(0, aeron_agent_stop(runner));
    EXPECT_EQ(0, aeron_driver_sender_invoke(&driver));
    EXPECT_EQ(2, probe.work);
    EXPECT_EQ(0, aeron_agent_close(runner));
}

TEST(SenderShutdownTest, shouldJoinNativeConductorBlockedByUninvokedSender)
{
    alignas(64) std::array<uint8_t, 1024 + AERON_RB_TRAILER_LENGTH> storage = {};
    aeron_spsc_rb_t queue = {};
    ASSERT_EQ(0, aeron_spsc_rb_init(&queue, storage.data(), storage.size()));
    aeron_driver_context_t context = {};
    context.threading_mode = AERON_THREADING_MODE_DEDICATED_SENDER_INVOKED;
    aeron_driver_t *driver = nullptr;
    ASSERT_EQ(0, aeron_alloc(reinterpret_cast<void **>(&driver), sizeof(*driver)));
    driver->context = &context;
    int64_t failures = 0;
    driver->sender.sender_proxy.command_queue = &queue;
    driver->sender.sender_proxy.fail_counter = &failures;
    ASSERT_EQ(0, aeron_agent_init(&driver->runners[AERON_AGENT_RUNNER_CONDUCTOR], "conductor",
        &driver->sender.sender_proxy, nullptr, nullptr, saturate_sender, nullptr,
        aeron_idle_strategy_noop_idle, nullptr));
    ASSERT_EQ(0, aeron_driver_start(driver, false));
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(2);
    while (0 == aeron_counter_get_acquire(&failures) && std::chrono::steady_clock::now() < deadline)
    {
        std::this_thread::yield();
    }
    EXPECT_GT(aeron_counter_get_acquire(&failures), 0);
    EXPECT_EQ(0, aeron_driver_close(driver));
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

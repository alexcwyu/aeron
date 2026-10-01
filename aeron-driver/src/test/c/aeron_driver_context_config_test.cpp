/*
 * Copyright 2014-2025 Real Logic Limited.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <functional>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "aeron_test_base.h"
#include "aeronmd.h"

extern "C"
{
#include "aeron_driver_context.h"
#include "media/aeron_debug_channel_endpoint_configuration.h"
}

using namespace aeron;

static const uint32_t DEFAULT_VALUE = UINT32_C(4);
static const uint32_t MIN_VALUE = UINT32_C(1);
static const uint32_t MAX_VALUE = UINT32_C(16);

static void test_send_channel_loss_supplier(void *clientd, struct aeron_send_channel_endpoint_stct *endpoint)
{
}

static void test_receive_channel_loss_supplier(void *clientd, struct aeron_receive_channel_endpoint_stct *endpoint)
{
}

class DriverContextConfigTest : public testing::Test
{
protected:
    void TearDown() override
    {
        aeron_env_unset(AERON_RECEIVER_IO_VECTOR_CAPACITY_ENV_VAR);
        aeron_env_unset(AERON_SENDER_IO_VECTOR_CAPACITY_ENV_VAR);
        aeron_env_unset(AERON_LOW_FILE_STORE_WARNING_THRESHOLD_ENV_VAR);
        aeron_env_unset(AERON_NAK_UNICAST_DELAY_ENV_VAR);
        aeron_env_unset(AERON_NAK_UNICAST_RETRY_DELAY_RATIO_ENV_VAR);
        aeron_env_unset(AERON_CUBICCONGESTIONCONTROL_INITIALRTT_ENV_VAR);
        aeron_env_unset(AERON_CUBICCONGESTIONCONTROL_MEASURERTT_ENV_VAR);
        aeron_env_unset(AERON_CUBICCONGESTIONCONTROL_TCPMODE_ENV_VAR);
    }
};

TEST_F(DriverContextConfigTest, shouldValidateReceiverIoVectorCapacity)
{
    aeron_driver_context_t *context;

    EXPECT_EQ(0, aeron_driver_context_init(&context));
    EXPECT_EQ(DEFAULT_VALUE, aeron_driver_context_get_receiver_io_vector_capacity(context));
    aeron_driver_context_set_receiver_io_vector_capacity(context, 0);
    EXPECT_EQ(MIN_VALUE, aeron_driver_context_get_receiver_io_vector_capacity(context));
    aeron_driver_context_set_receiver_io_vector_capacity(context, 2);
    EXPECT_EQ(2, aeron_driver_context_get_receiver_io_vector_capacity(context));
    aeron_driver_context_set_receiver_io_vector_capacity(context, 16);
    EXPECT_EQ(MAX_VALUE, aeron_driver_context_get_receiver_io_vector_capacity(context));
    aeron_driver_context_set_receiver_io_vector_capacity(context, 17);
    EXPECT_EQ(MAX_VALUE, aeron_driver_context_get_receiver_io_vector_capacity(context));

    aeron_driver_context_close(context);

    aeron_env_set(AERON_RECEIVER_IO_VECTOR_CAPACITY_ENV_VAR, "-1");
    EXPECT_EQ(0, aeron_driver_context_init(&context));
    EXPECT_EQ(MIN_VALUE, aeron_driver_context_get_receiver_io_vector_capacity(context));
    aeron_driver_context_close(context);

    aeron_env_set(AERON_RECEIVER_IO_VECTOR_CAPACITY_ENV_VAR, "0");
    EXPECT_EQ(0, aeron_driver_context_init(&context));
    EXPECT_EQ(MIN_VALUE, aeron_driver_context_get_receiver_io_vector_capacity(context));
    aeron_driver_context_close(context);

    aeron_env_set(AERON_RECEIVER_IO_VECTOR_CAPACITY_ENV_VAR, "17");
    EXPECT_EQ(0, aeron_driver_context_init(&context));
    EXPECT_EQ(MAX_VALUE, aeron_driver_context_get_receiver_io_vector_capacity(context));
    aeron_driver_context_close(context);

    aeron_env_set(AERON_RECEIVER_IO_VECTOR_CAPACITY_ENV_VAR, "1");
    ASSERT_EQ(0, aeron_driver_context_init(&context));
    EXPECT_EQ(MIN_VALUE, aeron_driver_context_get_receiver_io_vector_capacity(context));
    aeron_driver_context_close(context);

    aeron_env_set(AERON_RECEIVER_IO_VECTOR_CAPACITY_ENV_VAR, "16");
    ASSERT_EQ(0, aeron_driver_context_init(&context));
    EXPECT_EQ(MAX_VALUE, aeron_driver_context_get_receiver_io_vector_capacity(context));
    aeron_driver_context_close(context);
}

TEST_F(DriverContextConfigTest, shouldValidateSenderIoVectorCapacity)
{
    aeron_driver_context_t *context;

    EXPECT_EQ(0, aeron_driver_context_init(&context));
    EXPECT_EQ(DEFAULT_VALUE, aeron_driver_context_get_sender_io_vector_capacity(context));
    EXPECT_EQ(DEFAULT_VALUE, aeron_driver_context_get_sender_io_vector_capacity(context));
    aeron_driver_context_set_sender_io_vector_capacity(context, 0);
    EXPECT_EQ(MIN_VALUE, aeron_driver_context_get_sender_io_vector_capacity(context));
    aeron_driver_context_set_sender_io_vector_capacity(context, 2);
    EXPECT_EQ(2, aeron_driver_context_get_sender_io_vector_capacity(context));
    aeron_driver_context_set_sender_io_vector_capacity(context, 16);
    EXPECT_EQ(MAX_VALUE, aeron_driver_context_get_sender_io_vector_capacity(context));
    aeron_driver_context_set_sender_io_vector_capacity(context, 17);
    EXPECT_EQ(MAX_VALUE, aeron_driver_context_get_sender_io_vector_capacity(context));
    aeron_driver_context_close(context);

    aeron_env_set(AERON_SENDER_IO_VECTOR_CAPACITY_ENV_VAR, "-1");
    EXPECT_EQ(0, aeron_driver_context_init(&context));
    EXPECT_EQ(MIN_VALUE, aeron_driver_context_get_sender_io_vector_capacity(context));
    aeron_driver_context_close(context);

    aeron_env_set(AERON_SENDER_IO_VECTOR_CAPACITY_ENV_VAR, "0");
    EXPECT_EQ(0, aeron_driver_context_init(&context));
    EXPECT_EQ(MIN_VALUE, aeron_driver_context_get_sender_io_vector_capacity(context));
    aeron_driver_context_close(context);

    aeron_env_set(AERON_SENDER_IO_VECTOR_CAPACITY_ENV_VAR, "17");
    EXPECT_EQ(0, aeron_driver_context_init(&context));
    EXPECT_EQ(MAX_VALUE, aeron_driver_context_get_sender_io_vector_capacity(context));
    aeron_driver_context_close(context);

    aeron_env_set(AERON_SENDER_IO_VECTOR_CAPACITY_ENV_VAR, "1");
    ASSERT_EQ(0, aeron_driver_context_init(&context));
    EXPECT_EQ(MIN_VALUE, aeron_driver_context_get_sender_io_vector_capacity(context));
    aeron_driver_context_close(context);

    aeron_env_set(AERON_SENDER_IO_VECTOR_CAPACITY_ENV_VAR, "16");
    ASSERT_EQ(0, aeron_driver_context_init(&context));
    EXPECT_EQ(MAX_VALUE, aeron_driver_context_get_sender_io_vector_capacity(context));
    aeron_driver_context_close(context);
}

TEST_F(DriverContextConfigTest, shouldValidateMaxMessagesPerSendBuffers)
{
    aeron_driver_context_t *context;

    EXPECT_EQ(0, aeron_driver_context_init(&context));
    EXPECT_EQ(DEFAULT_VALUE, aeron_driver_context_get_network_publication_max_messages_per_send(context));
    aeron_driver_context_set_network_publication_max_messages_per_send(context, 0);
    EXPECT_EQ(MIN_VALUE, aeron_driver_context_get_network_publication_max_messages_per_send(context));
    aeron_driver_context_set_network_publication_max_messages_per_send(context, 2);
    EXPECT_EQ(2, aeron_driver_context_get_network_publication_max_messages_per_send(context));
    aeron_driver_context_set_network_publication_max_messages_per_send(context, 16);
    EXPECT_EQ(MAX_VALUE, aeron_driver_context_get_network_publication_max_messages_per_send(context));
    aeron_driver_context_set_network_publication_max_messages_per_send(context, 17);
    EXPECT_EQ(MAX_VALUE, aeron_driver_context_get_network_publication_max_messages_per_send(context));
    aeron_driver_context_close(context);

    aeron_env_set(AERON_NETWORK_PUBLICATION_MAX_MESSAGES_PER_SEND_ENV_VAR, "-1");
    EXPECT_EQ(0, aeron_driver_context_init(&context));
    EXPECT_EQ(MIN_VALUE, aeron_driver_context_get_network_publication_max_messages_per_send(context));
    aeron_driver_context_close(context);

    aeron_env_set(AERON_NETWORK_PUBLICATION_MAX_MESSAGES_PER_SEND_ENV_VAR, "0");
    EXPECT_EQ(0, aeron_driver_context_init(&context));
    EXPECT_EQ(MIN_VALUE, aeron_driver_context_get_network_publication_max_messages_per_send(context));
    aeron_driver_context_close(context);

    aeron_env_set(AERON_NETWORK_PUBLICATION_MAX_MESSAGES_PER_SEND_ENV_VAR, "17");
    EXPECT_EQ(0, aeron_driver_context_init(&context));
    EXPECT_EQ(MAX_VALUE, aeron_driver_context_get_network_publication_max_messages_per_send(context));
    aeron_driver_context_close(context);

    aeron_env_set(AERON_NETWORK_PUBLICATION_MAX_MESSAGES_PER_SEND_ENV_VAR, "1");
    ASSERT_EQ(0, aeron_driver_context_init(&context));
    EXPECT_EQ(MIN_VALUE, aeron_driver_context_get_network_publication_max_messages_per_send(context));
    aeron_driver_context_close(context);

    aeron_env_set(AERON_NETWORK_PUBLICATION_MAX_MESSAGES_PER_SEND_ENV_VAR, "16");
    ASSERT_EQ(0, aeron_driver_context_init(&context));
    EXPECT_EQ(MAX_VALUE, aeron_driver_context_get_network_publication_max_messages_per_send(context));
    aeron_driver_context_close(context);
}

TEST_F(DriverContextConfigTest, shouldHandleValuesOutsideOfUint32Range)
{
    aeron_driver_context_t *context;

    const char *uint32_max_plus_one = "4294967296";
    aeron_env_set(AERON_DRIVER_RESOURCE_FREE_LIMIT_ENV_VAR, uint32_max_plus_one);
    EXPECT_EQ(0, aeron_driver_context_init(&context));
    EXPECT_EQ(INT32_MAX, aeron_driver_context_get_resource_free_limit(context));
    aeron_driver_context_close(context);

    aeron_env_set(AERON_DRIVER_RESOURCE_FREE_LIMIT_ENV_VAR, "-1");
    EXPECT_EQ(0, aeron_driver_context_init(&context));
    EXPECT_EQ(1, aeron_driver_context_get_resource_free_limit(context));
    aeron_driver_context_close(context);
}

TEST_F(DriverContextConfigTest, shouldReturnDefaultLowFileStoreWarningThresholdIfNoneProvided)
{
    const uint64_t default_low_storage_warning_threshold = 160 * 1024 * 1024;

    aeron_driver_context_t *context = nullptr;
    EXPECT_EQ(default_low_storage_warning_threshold, aeron_driver_context_get_low_file_store_warning_threshold(context));

    ASSERT_EQ(0, aeron_driver_context_init(&context));
    EXPECT_EQ(default_low_storage_warning_threshold, aeron_driver_context_get_low_file_store_warning_threshold(context));
    aeron_driver_context_close(context);
}

TEST_F(DriverContextConfigTest, shouldAssignLowStoreWarningThreshold)
{
    aeron_driver_context_t *context = nullptr;
    EXPECT_EQ(-1, aeron_driver_context_set_low_file_store_warning_threshold(context, 42));

    const uint64_t threshold = 1024 * 1024;
    ASSERT_EQ(0, aeron_driver_context_init(&context));
    EXPECT_EQ(0, aeron_driver_context_set_low_file_store_warning_threshold(context, threshold));
    EXPECT_EQ(threshold, aeron_driver_context_get_low_file_store_warning_threshold(context));
    aeron_driver_context_close(context);
}

TEST_F(DriverContextConfigTest, shouldReadLowFileStoreWarningThresholdFromAnEnvironmentVariable)
{
    aeron_driver_context_t *context = nullptr;
    aeron_env_set(AERON_LOW_FILE_STORE_WARNING_THRESHOLD_ENV_VAR, "2m");
    ASSERT_EQ(0, aeron_driver_context_init(&context));
    EXPECT_EQ(2 * 1024 * 1024, aeron_driver_context_get_low_file_store_warning_threshold(context));
    aeron_driver_context_close(context);

    const uint64_t default_low_storage_warning_threshold = 160 * 1024 * 1024;
    aeron_env_set(AERON_LOW_FILE_STORE_WARNING_THRESHOLD_ENV_VAR, "garbage");
    ASSERT_EQ(0, aeron_driver_context_init(&context));
    EXPECT_EQ(default_low_storage_warning_threshold, aeron_driver_context_get_low_file_store_warning_threshold(context));
    aeron_driver_context_close(context);
}


TEST_F(DriverContextConfigTest, shouldHonorUntetheredLingerFromAnEnvironmentVariable)
{
    aeron_driver_context_t *context = nullptr;

    ASSERT_EQ(0, aeron_driver_context_init(&context));
    EXPECT_EQ(AERON_NULL_VALUE, aeron_driver_context_get_untethered_linger_timeout_ns(context));
    aeron_driver_context_close(context);

    aeron_env_set(AERON_UNTETHERED_LINGER_TIMEOUT_ENV_VAR, "5000000");
    ASSERT_EQ(0, aeron_driver_context_init(&context));
    EXPECT_EQ(5000000, aeron_driver_context_get_untethered_linger_timeout_ns(context));
    aeron_driver_context_close(context);

    ASSERT_EQ(0, aeron_driver_context_init(&context));
    aeron_driver_context_set_untethered_linger_timeout_ns(context, 3000000);
    EXPECT_EQ(3000000, aeron_driver_context_get_untethered_linger_timeout_ns(context));
    aeron_driver_context_close(context);
}

TEST_F(DriverContextConfigTest, shouldInitializeNakUnicastDelayToDefaultValue)
{
    aeron_driver_context_t *context;
    EXPECT_EQ(0, aeron_driver_context_init(&context));

    EXPECT_EQ(1000, aeron_driver_context_get_nak_unicast_delay_ns(context));
    EXPECT_EQ(100, aeron_driver_context_get_nak_unicast_retry_delay_ratio(context));

    aeron_driver_context_close(context);
}

TEST_F(DriverContextConfigTest, shouldInitializeNakUnicastDelayFromEnvironment)
{
    aeron_driver_context_t *context;
    aeron_env_set(AERON_NAK_UNICAST_DELAY_ENV_VAR, "3s");
    aeron_env_set(AERON_NAK_UNICAST_RETRY_DELAY_RATIO_ENV_VAR, "171");
    EXPECT_EQ(0, aeron_driver_context_init(&context));

    EXPECT_EQ(3ul * 1000 * 1000 * 1000, aeron_driver_context_get_nak_unicast_delay_ns(context));
    EXPECT_EQ(171, aeron_driver_context_get_nak_unicast_retry_delay_ratio(context));

    aeron_driver_context_close(context);
}

TEST_F(DriverContextConfigTest, shouldUseMinimumValuesIsNakDelayIsOutOfBounds)
{
    aeron_driver_context_t *context;
    aeron_env_set(AERON_NAK_UNICAST_DELAY_ENV_VAR, "876");
    aeron_env_set(AERON_NAK_UNICAST_RETRY_DELAY_RATIO_ENV_VAR, "0");
    EXPECT_EQ(0, aeron_driver_context_init(&context));

    EXPECT_EQ(1000, aeron_driver_context_get_nak_unicast_delay_ns(context));
    EXPECT_EQ(1, aeron_driver_context_get_nak_unicast_retry_delay_ratio(context));

    aeron_driver_context_close(context);
}

TEST_F(DriverContextConfigTest, shouldSetNakDelayExplicitly)
{
    aeron_driver_context_t *context;
    EXPECT_EQ(0, aeron_driver_context_init(&context));

    const uint64_t nakDelayNs = 15ul * 1000 * 1245;
    EXPECT_EQ(0, aeron_driver_context_set_nak_unicast_delay_ns(context, nakDelayNs));
    EXPECT_EQ(nakDelayNs, aeron_driver_context_get_nak_unicast_delay_ns(context));

    const uint64_t nakDelayRatio = INT64_MAX;
    EXPECT_EQ(0, aeron_driver_context_set_nak_unicast_retry_delay_ratio(context, nakDelayRatio));
    EXPECT_EQ(nakDelayRatio, aeron_driver_context_get_nak_unicast_retry_delay_ratio(context));

    aeron_driver_context_close(context);
}

TEST_F(DriverContextConfigTest, shouldFailInitIfNakDelayComboIsOutOfBounds)
{
    aeron_driver_context_t *context;
    aeron_env_set(AERON_NAK_UNICAST_DELAY_ENV_VAR, "1000000s");
    aeron_env_set(AERON_NAK_UNICAST_RETRY_DELAY_RATIO_ENV_VAR, "567890473482340000");
    EXPECT_EQ(-1, aeron_driver_context_init(&context));
    EXPECT_NE(
        std::string::npos,
        std::string(aeron_errmsg()).find("nak_unicast_delay_ns (1000000000000000) * nak_unicast_retry_delay_ratio (567890473482340000) exceeds 9223372036854775807"));
}

TEST_F(DriverContextConfigTest, shouldApplyCpusetAffinity)
{
    aeron_driver_context_t *context;
    aeron_env_set(AERON_CONDUCTOR_CPU_AFFINITY_ENV_VAR, "1");
    aeron_env_set(AERON_SENDER_CPU_AFFINITY_ENV_VAR, "2");
    aeron_env_set(AERON_RECEIVER_CPU_AFFINITY_ENV_VAR, "3");
    aeron_env_set(AERON_DRIVER_NATIVE_RESOURCE_AGENT_CPU_AFFINITY_ENV_VAR, "4");

    EXPECT_EQ(0, aeron_driver_context_init(&context)) << aeron_errmsg();
    EXPECT_EQ(1, aeron_driver_context_get_conductor_cpu_affinity(context));
    EXPECT_EQ(2, aeron_driver_context_get_sender_cpu_affinity(context));
    EXPECT_EQ(3, aeron_driver_context_get_receiver_cpu_affinity(context));
    EXPECT_EQ(4, aeron_driver_context_get_native_resource_agent_cpu_affinity(context));

    int cpus[5] = { 9, 11, 13, 17, 19 };
    EXPECT_EQ(0, aeron_driver_context_apply_cpuset_affinity(context, cpus, 5)) << aeron_errmsg();

    EXPECT_EQ(11, aeron_driver_context_get_conductor_cpu_affinity(context));
    EXPECT_EQ(13, aeron_driver_context_get_sender_cpu_affinity(context));
    EXPECT_EQ(17, aeron_driver_context_get_receiver_cpu_affinity(context));
    EXPECT_EQ(19, aeron_driver_context_get_native_resource_agent_cpu_affinity(context));

    aeron_driver_context_close(context);
}

TEST_F(DriverContextConfigTest, shouldErrorWithInvalidCpusetAffinity)
{
    aeron_driver_context_t *context;
    int cpus[4] = { 9, 11, 13, 17 };

    EXPECT_EQ(0, aeron_driver_context_init(&context)) << aeron_errmsg();

    aeron_driver_context_set_conductor_cpu_affinity(context, 5);
    aeron_driver_context_set_sender_cpu_affinity(context, 2);
    aeron_driver_context_set_receiver_cpu_affinity(context, 3);
    aeron_driver_context_set_native_resource_agent_cpu_affinity(context, 1);

    EXPECT_EQ(-1, aeron_driver_context_apply_cpuset_affinity(context, cpus, 4)) << aeron_errmsg();

    aeron_driver_context_set_conductor_cpu_affinity(context, 1);
    aeron_driver_context_set_sender_cpu_affinity(context, 5);
    aeron_driver_context_set_receiver_cpu_affinity(context, 3);
    aeron_driver_context_set_native_resource_agent_cpu_affinity(context, 2);

    EXPECT_EQ(-1, aeron_driver_context_apply_cpuset_affinity(context, cpus, 4)) << aeron_errmsg();

    aeron_driver_context_set_conductor_cpu_affinity(context, 1);
    aeron_driver_context_set_sender_cpu_affinity(context, 2);
    aeron_driver_context_set_receiver_cpu_affinity(context, 5);
    aeron_driver_context_set_native_resource_agent_cpu_affinity(context, 3);

    EXPECT_EQ(-1, aeron_driver_context_apply_cpuset_affinity(context, cpus, 4)) << aeron_errmsg();

    aeron_driver_context_set_conductor_cpu_affinity(context, 1);
    aeron_driver_context_set_sender_cpu_affinity(context, 2);
    aeron_driver_context_set_receiver_cpu_affinity(context, 3);
    aeron_driver_context_set_native_resource_agent_cpu_affinity(context, 5);

    EXPECT_EQ(-1, aeron_driver_context_apply_cpuset_affinity(context, cpus, 4)) << aeron_errmsg();

    aeron_driver_context_close(context);
}

TEST_F(DriverContextConfigTest, shouldNotChangeAffinityWhenUnset)
{
    aeron_driver_context_t *context;
    int cpus[4] = { 9, 11, 13, 17 };

    EXPECT_EQ(0, aeron_driver_context_init(&context)) << aeron_errmsg();

    aeron_driver_context_set_conductor_cpu_affinity(context, -1);
    aeron_driver_context_set_sender_cpu_affinity(context, -1);
    aeron_driver_context_set_receiver_cpu_affinity(context, -1);
    aeron_driver_context_set_native_resource_agent_cpu_affinity(context, -1);
    EXPECT_EQ(-1, aeron_driver_context_get_conductor_cpu_affinity(context));
    EXPECT_EQ(-1, aeron_driver_context_get_sender_cpu_affinity(context));
    EXPECT_EQ(-1, aeron_driver_context_get_receiver_cpu_affinity(context));
    EXPECT_EQ(-1, aeron_driver_context_get_native_resource_agent_cpu_affinity(context));

    EXPECT_EQ(0, aeron_driver_context_apply_cpuset_affinity(context, cpus, 4)) << aeron_errmsg();

    EXPECT_EQ(-1, aeron_driver_context_get_conductor_cpu_affinity(context));
    EXPECT_EQ(-1, aeron_driver_context_get_sender_cpu_affinity(context));
    EXPECT_EQ(-1, aeron_driver_context_get_receiver_cpu_affinity(context));
    EXPECT_EQ(-1, aeron_driver_context_get_native_resource_agent_cpu_affinity(context));

    aeron_driver_context_close(context);
}

TEST_F(DriverContextConfigTest, shouldReadCubicInitialRttLazilyAndLetTheSetterWin)
{
    aeron_driver_context_t *context;

    aeron_env_unset(AERON_CUBICCONGESTIONCONTROL_INITIALRTT_ENV_VAR);
    ASSERT_EQ(0, aeron_driver_context_init(&context)) << aeron_errmsg();
    EXPECT_EQ(
        (uint64_t)AERON_CUBICCONGESTIONCONTROL_INITIALRTT_DEFAULT,
        aeron_driver_context_get_cubic_congestion_control_initial_rtt_ns(context));

    aeron_env_set(AERON_CUBICCONGESTIONCONTROL_INITIALRTT_ENV_VAR, "250us");
    EXPECT_EQ(UINT64_C(250000), aeron_driver_context_get_cubic_congestion_control_initial_rtt_ns(context));

    EXPECT_EQ(0, aeron_driver_context_set_cubic_congestion_control_initial_rtt_ns(context, UINT64_C(500000)));
    EXPECT_EQ(UINT64_C(500000), aeron_driver_context_get_cubic_congestion_control_initial_rtt_ns(context));

    EXPECT_EQ(-1, aeron_driver_context_set_cubic_congestion_control_initial_rtt_ns(context, 0));
    EXPECT_EQ(EINVAL, aeron_errcode());
    EXPECT_EQ(
        -1,
        aeron_driver_context_set_cubic_congestion_control_initial_rtt_ns(context, (uint64_t)(INT64_MAX / 4) + 1));
    EXPECT_EQ(EINVAL, aeron_errcode());
    EXPECT_EQ(UINT64_C(500000), aeron_driver_context_get_cubic_congestion_control_initial_rtt_ns(context));

    aeron_env_unset(AERON_CUBICCONGESTIONCONTROL_INITIALRTT_ENV_VAR);
    aeron_driver_context_close(context);
}

TEST_F(DriverContextConfigTest, shouldReadCubicMeasureRttAndTcpModeLazilyAndLetTheSetterWin)
{
    aeron_driver_context_t *context;

    ASSERT_EQ(0, aeron_driver_context_init(&context)) << aeron_errmsg();
    EXPECT_FALSE(aeron_driver_context_get_cubic_congestion_control_measure_rtt(context));
    EXPECT_FALSE(aeron_driver_context_get_cubic_congestion_control_tcp_mode(context));

    aeron_env_set(AERON_CUBICCONGESTIONCONTROL_MEASURERTT_ENV_VAR, "true");
    aeron_env_set(AERON_CUBICCONGESTIONCONTROL_TCPMODE_ENV_VAR, "true");
    EXPECT_TRUE(aeron_driver_context_get_cubic_congestion_control_measure_rtt(context));
    EXPECT_TRUE(aeron_driver_context_get_cubic_congestion_control_tcp_mode(context));

    EXPECT_EQ(0, aeron_driver_context_set_cubic_congestion_control_measure_rtt(context, false));
    EXPECT_EQ(0, aeron_driver_context_set_cubic_congestion_control_tcp_mode(context, false));
    EXPECT_FALSE(aeron_driver_context_get_cubic_congestion_control_measure_rtt(context));
    EXPECT_FALSE(aeron_driver_context_get_cubic_congestion_control_tcp_mode(context));

    aeron_driver_context_close(context);
}

TEST_F(DriverContextConfigTest, shouldSetConductorUdpChannelTransportBindingsSeparately)
{
    aeron_driver_context_t *context;

    ASSERT_EQ(0, aeron_driver_context_init(&context)) << aeron_errmsg();
    aeron_udp_channel_transport_bindings_t *default_bindings = aeron_udp_channel_transport_bindings_load_media("default");
    ASSERT_NE(nullptr, default_bindings);
    EXPECT_EQ(default_bindings, aeron_driver_context_get_conductor_udp_channel_transport_bindings(context));

    aeron_udp_channel_transport_bindings_t conductor_bindings = *default_bindings;
    conductor_bindings.meta_info.name = "conductor-only";
    EXPECT_EQ(0, aeron_driver_context_set_conductor_udp_channel_transport_bindings(context, &conductor_bindings));
    EXPECT_EQ(&conductor_bindings, aeron_driver_context_get_conductor_udp_channel_transport_bindings(context));
    EXPECT_EQ(default_bindings, aeron_driver_context_get_udp_channel_transport_bindings(context));

    EXPECT_EQ(-1, aeron_driver_context_set_conductor_udp_channel_transport_bindings(nullptr, &conductor_bindings));
    EXPECT_EQ(-1, aeron_driver_context_set_conductor_udp_channel_transport_bindings(context, nullptr));
    EXPECT_EQ(EINVAL, aeron_errcode());
    EXPECT_EQ(&conductor_bindings, aeron_driver_context_get_conductor_udp_channel_transport_bindings(context));

    aeron_driver_context_close(context);
}

TEST_F(DriverContextConfigTest, shouldSetChannelLossSuppliers)
{
    aeron_driver_context_t *context;
    int send_clientd = 0;
    int receive_clientd = 0;

    ASSERT_EQ(0, aeron_driver_context_init(&context)) << aeron_errmsg();
    EXPECT_EQ(nullptr, aeron_driver_context_get_send_channel_loss_supplier(context));
    EXPECT_EQ(nullptr, aeron_driver_context_get_send_channel_loss_supplier_clientd(context));
    EXPECT_EQ(nullptr, aeron_driver_context_get_receive_channel_loss_supplier(context));
    EXPECT_EQ(nullptr, aeron_driver_context_get_receive_channel_loss_supplier_clientd(context));

    EXPECT_EQ(0, aeron_driver_context_set_send_channel_loss_supplier(
        context, test_send_channel_loss_supplier, &send_clientd));
    EXPECT_EQ(0, aeron_driver_context_set_receive_channel_loss_supplier(
        context, test_receive_channel_loss_supplier, &receive_clientd));
    EXPECT_EQ(test_send_channel_loss_supplier, aeron_driver_context_get_send_channel_loss_supplier(context));
    EXPECT_EQ(&send_clientd, aeron_driver_context_get_send_channel_loss_supplier_clientd(context));
    EXPECT_EQ(test_receive_channel_loss_supplier, aeron_driver_context_get_receive_channel_loss_supplier(context));
    EXPECT_EQ(&receive_clientd, aeron_driver_context_get_receive_channel_loss_supplier_clientd(context));

    EXPECT_EQ(0, aeron_driver_context_set_send_channel_loss_supplier(context, nullptr, nullptr));
    EXPECT_EQ(nullptr, aeron_driver_context_get_send_channel_loss_supplier(context));

    aeron_driver_context_close(context);
}

TEST_F(DriverContextConfigTest, shouldReleaseDebugInstalledStateWhenReplacingChannelLossSuppliers)
{
    aeron_driver_context_t *context;
    int send_clientd = 0;

    ASSERT_EQ(0, aeron_driver_context_init(&context)) << aeron_errmsg();

    // the debug install allocates a clientd the context owns; a replacement through the setter must release it
    ASSERT_EQ(0, aeron_debug_channel_endpoint_configuration_install(context));
    ASSERT_NE(nullptr, aeron_driver_context_get_send_channel_loss_supplier(context));
    ASSERT_NE(nullptr, aeron_driver_context_get_send_channel_loss_supplier_clientd(context));
    ASSERT_NE(nullptr, aeron_driver_context_get_receive_channel_loss_supplier(context));
    ASSERT_NE(nullptr, aeron_driver_context_get_receive_channel_loss_supplier_clientd(context));

    EXPECT_EQ(0, aeron_driver_context_set_send_channel_loss_supplier(
        context, test_send_channel_loss_supplier, &send_clientd));
    EXPECT_EQ(&send_clientd, aeron_driver_context_get_send_channel_loss_supplier_clientd(context));

    EXPECT_EQ(0, aeron_driver_context_set_receive_channel_loss_supplier(
        context, test_receive_channel_loss_supplier, nullptr));
    EXPECT_EQ(nullptr, aeron_driver_context_get_receive_channel_loss_supplier_clientd(context));

    // a second replacement must not free the caller-owned clientd (only the debug-installed one)
    EXPECT_EQ(0, aeron_driver_context_set_send_channel_loss_supplier(
        context, test_send_channel_loss_supplier, &send_clientd));
    EXPECT_EQ(&send_clientd, aeron_driver_context_get_send_channel_loss_supplier_clientd(context));

    aeron_driver_context_close(context);
}

TEST_F(DriverContextConfigTest, shouldSetFlowControlRetransmitReceiverWindowMultiples)
{
    aeron_driver_context_t *context;

    ASSERT_EQ(0, aeron_driver_context_init(&context)) << aeron_errmsg();
    EXPECT_EQ(
        (size_t)AERON_UNICAST_FLOW_CONTROL_RETRANSMIT_RECEIVER_WINDOW_MULTIPLE,
        aeron_driver_context_get_unicast_flow_control_rrwm(context));
    EXPECT_EQ(
        (size_t)AERON_MULTICAST_FLOW_CONTROL_RETRANSMIT_RECEIVER_WINDOW_MULTIPLE,
        aeron_driver_context_get_multicast_flow_control_rrwm(context));

    EXPECT_EQ(0, aeron_driver_context_set_unicast_flow_control_rrwm(context, 8));
    EXPECT_EQ(0, aeron_driver_context_set_multicast_flow_control_rrwm(context, 2));
    EXPECT_EQ((size_t)8, aeron_driver_context_get_unicast_flow_control_rrwm(context));
    EXPECT_EQ((size_t)2, aeron_driver_context_get_multicast_flow_control_rrwm(context));

    EXPECT_EQ(-1, aeron_driver_context_set_unicast_flow_control_rrwm(context, 0));
    EXPECT_EQ(EINVAL, aeron_errcode());
    EXPECT_EQ(-1, aeron_driver_context_set_multicast_flow_control_rrwm(context, (size_t)INT32_MAX + 1));
    EXPECT_EQ(EINVAL, aeron_errcode());
    EXPECT_EQ((size_t)8, aeron_driver_context_get_unicast_flow_control_rrwm(context));
    EXPECT_EQ((size_t)2, aeron_driver_context_get_multicast_flow_control_rrwm(context));

    aeron_driver_context_close(context);
}

/*
 * Copyright 2026 Alex Yu.
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

#include <gtest/gtest.h>

extern "C"
{
#include "aeron_agent.h"
#include "aeron_alloc.h"
}

class IdleStrategyTest : public testing::Test
{
};

TEST_F(IdleStrategyTest, shouldRejectInvalidSleepingInitArgsWithoutAllocatingState)
{
    void *state = nullptr;

    EXPECT_EQ(-1, aeron_idle_strategy_sleeping_init_args(&state, nullptr, "not a duration"));
    EXPECT_EQ(nullptr, state) << "failed parse must not leave a leaked allocation behind";
}

TEST_F(IdleStrategyTest, shouldParseSleepingInitArgsIntoAllocatedState)
{
    void *state = nullptr;

    ASSERT_EQ(0, aeron_idle_strategy_sleeping_init_args(&state, nullptr, "250us"));
    ASSERT_NE(nullptr, state);
    EXPECT_EQ(UINT64_C(250000), *reinterpret_cast<uint64_t *>(state));

    aeron_free(state);
}

TEST_F(IdleStrategyTest, shouldDefaultSleepingInitArgsToOneNanosecond)
{
    void *state = nullptr;

    ASSERT_EQ(0, aeron_idle_strategy_sleeping_init_args(&state, nullptr, nullptr));
    ASSERT_NE(nullptr, state);
    EXPECT_EQ(UINT64_C(1), *reinterpret_cast<uint64_t *>(state));

    aeron_free(state);
}

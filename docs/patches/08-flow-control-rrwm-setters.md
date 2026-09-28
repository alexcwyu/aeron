# Public setters for the flow control retransmit receiver window multiples

Part of `1.53.3-patch.2` (2026-09-28), on `patch/1.53.3-consolidated`.

## What

`aeronmd.h` gains two set/get pairs over existing `aeron_driver_context_t`
fields:

| Field | Java property | Setter / getter | Default |
|---|---|---|---|
| `unicast_flow_control_rrwm` | `aeron.unicast.flow.control.rrwm` | `aeron_driver_context_{set,get}_unicast_flow_control_rrwm` (`size_t`) | 16 |
| `multicast_flow_control_rrwm` | `aeron.multicast.flow.control.rrwm` | `aeron_driver_context_{set,get}_multicast_flow_control_rrwm` (`size_t`) | 4 |

A setter rejects a value outside 1 to `INT32_MAX` with `-1` and `EINVAL` and
leaves the field unchanged, the range Java's `MediaDriver.Context.conclude`
enforces. With a `NULL` context the getters return the defaults.

## Why

The fields exist, the flow control strategies read them
(`aeron_flow_control.c`), and Java configures them through the two properties
and `MediaDriver.Context`, but the C driver had neither an environment
variable nor a setter: only the compiled-in defaults applied. The upstream
tests set the fields directly (`FlowControlTest.unicastStrategyShouldUseWindowMultipleFromContext`,
`MinFlowControlTest.minStrategyShouldUseDefaultWindowMultipleFromContext`).

## Behaviour

Defaults are unchanged, and no environment variable is added, so an existing
driver behaves exactly as before. A channel's own `rrwm` flow control option
still overrides the multicast value. No hot-path code changes.

## Test

`DriverContextConfigTest.shouldSetFlowControlRetransmitReceiverWindowMultiples`:
the defaults after init, set values read back, and `0` and `INT32_MAX + 1`
rejected with `EINVAL` without changing the stored values. The existing
`flow_control_test` cases that read the fields into each strategy still pass.

Linux x86-64, GCC, Debug, 2026-09-28: `driver_context_config_test` 21/21,
`flow_control_test` 66/66.

## Reproduce

```sh
cmake -S . -B build-c-tests -DCMAKE_BUILD_TYPE=Debug -DAERON_TESTS=ON \
  -DAERON_SYSTEM_TESTS=OFF -DAERON_BUILD_SAMPLES=OFF -DBUILD_AERON_ARCHIVE_API=OFF
cmake --build build-c-tests --target driver_context_config_test flow_control_test
LD_LIBRARY_PATH=$PWD/build-c-tests/lib ctest --test-dir build-c-tests \
  -R '^(driver_context_config_test|flow_control_test)$' --output-on-failure
```

`LD_LIBRARY_PATH` takes precedence over the binaries' `RUNPATH`; on a host with
an older `libaeron_driver.so` on that path, point it at the build first.

## Upstream candidate

Yes; upstream would likely also want the two environment variables, for
parity with the Java properties. This fork adds only the setters.

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
`flow_control_test` 66/66. Build and run as in
[05](05-cubic-congestion-control-setters.md#reproduce).

## Upstream candidate

Yes; upstream would likely also want the two environment variables, for
parity with the Java properties. This fork adds only the setters.

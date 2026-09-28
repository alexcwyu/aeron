# Public setters for the Cubic congestion control settings

Part of `1.53.3-patch.2` (2026-09-28), on `patch/1.53.3-consolidated` over
`1.53.3-patch.1` (`267dcadf62`).

## What

`aeronmd.h` gains three set/get pairs:

| Environment variable | Java property | Setter / getter | Default |
|---|---|---|---|
| `AERON_CUBICCONGESTIONCONTROL_MEASURERTT` | `aeron.CubicCongestionControl.measureRtt` | `aeron_driver_context_{set,get}_cubic_congestion_control_measure_rtt` (`bool`) | `false` |
| `AERON_CUBICCONGESTIONCONTROL_INITIALRTT` | `aeron.CubicCongestionControl.initialRtt` | `aeron_driver_context_{set,get}_cubic_congestion_control_initial_rtt_ns` (`uint64_t`) | 100 us |
| `AERON_CUBICCONGESTIONCONTROL_TCPMODE` | `aeron.CubicCongestionControl.tcpMode` | `aeron_driver_context_{set,get}_cubic_congestion_control_tcp_mode` (`bool`) | `false` |

`aeron_driver_context_t` gains a value and an `_is_set` flag per setting;
`aeron_driver_context_init` clears the flags and stores the defaults. The
defaults move from `aeron_congestion_control.c` to `aeron_driver_context.h`
(`AERON_CUBICCONGESTIONCONTROL_{MEASURERTT,INITIALRTT,TCPMODE}_DEFAULT`).

## Why

These were the only driver settings that could be configured by environment
variable alone: `aeron_cubic_congestion_control_strategy_supplier` read them
with `getenv` each time it created a strategy. An embedding application (the
Rust wrapper) had no way to configure them per context, as Java's
`CubicCongestionControlConfiguration` system properties allow.

## Behaviour

Parsing stays lazy, so an environment-only caller sees no change:

- While a setting is unset, the strategy reads its variable when it is
  created, exactly as before, including the failure of strategy creation on an
  invalid `AERON_CUBICCONGESTIONCONTROL_INITIALRTT`.
- A set value wins over the variable.
- A getter returns the set value, or else parses the variable at call time;
  an unparsable initial RTT reads as the default there, since a getter cannot
  fail.

No hot-path code changes: the strategy reads the values once, at creation.

## Test

- `DriverContextConfigTest.shouldReadCubicInitialRttLazilyAndLetTheSetterWin`:
  default with the variable unset, `250us` read after init, and a set 500 us
  winning over the still-set variable.
- `DriverContextConfigTest.shouldReadCubicMeasureRttAndTcpModeLazilyAndLetTheSetterWin`:
  the same for the two booleans.
- `CongestionControlTest.cubicCongestionControlStrategyConfigurationFromSetters`:
  `cubicCongestionControlStrategyConfiguration` driven through the setters with
  the variables unset, plus one RTT-timeout check at 22 s that only a 1 s
  initial RTT satisfies.
- `CongestionControlTest.cubicCongestionControlSupplierReturnsNegativeValueIfInitialRttIsInvalid`
  is unchanged and still passes.

Negative control: with `aeron_congestion_control.c` restored to
`1.53.3-patch.1`, the setter test fails (`should_measure_rtt` false at 10 s and
30 s, because the set values never reach the strategy).

Linux x86-64, GCC, Debug, 2026-09-28: `congestion_control_test` 12/12,
`driver_context_config_test` 18/18.

## Reproduce

```sh
cmake -S . -B build-c-tests -DCMAKE_BUILD_TYPE=Debug -DAERON_TESTS=ON \
  -DAERON_SYSTEM_TESTS=OFF -DAERON_BUILD_SAMPLES=OFF -DBUILD_AERON_ARCHIVE_API=OFF
cmake --build build-c-tests --target driver_context_config_test congestion_control_test
LD_LIBRARY_PATH=$PWD/build-c-tests/lib ctest --test-dir build-c-tests \
  -R '^(driver_context_config_test|congestion_control_test)$' --output-on-failure
```

`LD_LIBRARY_PATH` takes precedence over the binaries' `RUNPATH`; on a host with
an older `libaeron_driver.so` on that path, point it at the build first.

## Upstream candidate

Yes. The change is additive and keeps the environment behaviour.

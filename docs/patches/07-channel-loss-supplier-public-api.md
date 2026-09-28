# Public API for the send and receive channel loss suppliers

Part of `1.53.3-patch.2` (2026-09-28), on `patch/1.53.3-consolidated`.

## What

The two loss-supplier callback types and their setters move from the internal
`aeron_driver_context.h` to the public `aeronmd.h`, unchanged, and gain
getters:

- `aeron_send_channel_loss_supplier_func_t`,
  `aeron_driver_context_set_send_channel_loss_supplier(context, func, clientd)`,
  `aeron_driver_context_get_send_channel_loss_supplier(context)` and
  `aeron_driver_context_get_send_channel_loss_supplier_clientd(context)`;
- the same four for the receive side.

The endpoint types stay opaque in the public header (forward-declared
structs).

## Why

`aeron_send_channel_endpoint_create` and
`aeron_receive_channel_endpoint_create` call these suppliers with each new
endpoint, which is how the debug channel endpoints attach loss generators
(`aeron_debug_channel_endpoint_configuration_install`). They are the C
counterpart of Java's `MediaDriver.Context.sendChannelEndpointSupplier` and
`receiveChannelEndpointSupplier`, but could only be set through the internal
header, and had no getters.

## Behaviour

No change: the setters keep their signatures and semantics, and both
suppliers stay `NULL` by default. The context does not own `clientd`;
`aeron_debug_channel_endpoint_configuration_cleanup` still frees only the
state it installed itself.

## Test

`DriverContextConfigTest.shouldSetChannelLossSuppliers`: both suppliers and
their `clientd` are `NULL` after init, the getters return what the setters
stored, and a `NULL` supplier can be set back. The existing
`aeron_test_loss_generators_test` (which installs the debug suppliers through
the same setters) still passes.

Linux x86-64, GCC, Debug, 2026-09-28: `driver_context_config_test` 20/20,
`aeron_test_loss_generators_test` 39/39.

## Reproduce

```sh
cmake -S . -B build-c-tests -DCMAKE_BUILD_TYPE=Debug -DAERON_TESTS=ON \
  -DAERON_SYSTEM_TESTS=OFF -DAERON_BUILD_SAMPLES=OFF -DBUILD_AERON_ARCHIVE_API=OFF
cmake --build build-c-tests --target driver_context_config_test aeron_test_loss_generators_test
LD_LIBRARY_PATH=$PWD/build-c-tests/lib ctest --test-dir build-c-tests \
  -R '^(driver_context_config_test|aeron_test_loss_generators_test)$' --output-on-failure
```

`LD_LIBRARY_PATH` takes precedence over the binaries' `RUNPATH`; on a host with
an older `libaeron_driver.so` on that path, point it at the build first.

## Upstream candidate

Yes, as the public counterpart of the Java endpoint suppliers.

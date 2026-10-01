# Public setter for the conductor UDP channel transport bindings

Part of `1.53.3-patch.2` (2026-09-28), on `patch/1.53.3-consolidated`.

## What

`aeronmd.h` gains
`aeron_driver_context_set_conductor_udp_channel_transport_bindings` and
`aeron_driver_context_get_conductor_udp_channel_transport_bindings`, the same
shape as the existing `udp_channel_transport_bindings` pair. With a `NULL`
context the getter returns the `default` bindings, as its sibling does.

## Why

`aeron_driver_context_init` loads
`conductor_udp_channel_transport_bindings` from
`AERON_CONDUCTOR_UDP_CHANNEL_TRANSPORT_BINDINGS_MEDIA`
(`aeron_driver_context.c`, beside the data-path bindings), and the driver name
resolver uses them (`aeron_driver_name_resolver.c`). The data-path bindings
had a setter; the conductor's did not, so they could only be chosen by
environment variable.

## Behaviour

Init is unchanged: it still loads the bindings named by the variable, and
still fails on an unknown name. A value set afterwards replaces them. The
context never frees transport bindings, so the caller keeps ownership, as with
`aeron_driver_context_set_udp_channel_transport_bindings`.

A `NULL` value is rejected with `-1` and `EINVAL` (added 2026-10-01), leaving
the field unchanged. The driver name resolver dereferences the conductor
bindings when it initialises its data paths
(`aeron_udp_channel_data_paths_init` reads `media_bindings->send_func`), so a
`NULL` stored here would crash driver startup rather than fall back to a
default. The data-path sibling setter keeps accepting `NULL`: it is
upstream's own API, and `aeron_driver_context_init` always leaves it loaded,
whereas the conductor setter this patch adds is new surface with no such
invariant to preserve.

## Test

`DriverContextConfigTest.shouldSetConductorUdpChannelTransportBindingsSeparately`:
the default is the `default` media bindings; a set value is returned by the
conductor getter while the data-path getter still returns the default; a
`NULL` context is rejected with `-1`; a `NULL` value is rejected with `-1` and
`EINVAL` without changing the stored bindings (added 2026-10-01).

Linux x86-64, GCC, Debug, 2026-09-28: `driver_context_config_test` 19/19.
Build and run as in [05](05-cubic-congestion-control-setters.md#reproduce).

## Upstream candidate

Yes. It completes the pair the data-path bindings already have.

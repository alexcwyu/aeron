# Review fixes over patch.5

Part of `1.53.3-patch.6` (2026-11-02), on `patch/1.53.3-consolidated`.

## Defects fixed

| Where | Defect | Fix |
|---|---|---|
| `aeron_archive_client.c` | `aeron_archive_dispatch_recording_signal`, the path every synchronous archive call uses while it waits for its response, never set the `correlation_id` that `1.53.3-patch.5` appended, so the consumer received an uninitialised value. | Copy it from the control response poller, as the descriptor pollers already did. |
| `aeron_driver_name_resolver.c` | Entries were accepted with an empty name or a name longer than Java's `ResolutionEntryFlyweight.MAX_NAME_LENGTH` (512). A cached entry too long for one neighbour frame stops `aeron_driver_name_resolver_send_neighbor_resolutions` from advancing, so the native resource agent spins. | Accept a name of 1 to 512 bytes only, as Java's `isValid` does. |
| `aeron_udp_protocol.c` | `aeron_res_header_entry_length` accepted a negative name length, which the frame dissector then printed with a negative precision, reading past the entry. | Report no entry length for a negative name length. |
| `aeronmd.c` | The standalone driver accepted `DEDICATED_SENDER_INVOKED`, whose sender only runs when a host invokes it, so its network publications never sent. | Refuse the mode at startup. |

Native tests: `NameResolverMalformedEntryTest` (empty name, 513 byte name,
512 byte name) and `NameResolverTest.shouldGiveNoEntryLengthForNegativeNameLength`.
The three rejection cases fail against `1.53.3-patch.5` and pass here
(`name_resolver_test`); `publication_image_test` and `sender_shutdown_test`
still pass. The Rust recording-signal tests exercise constructed events and Rust
dispatch; they do not prove the synchronous native producer's correlation field.
That assignment is source-reviewed here, with a native negative-control test still
outstanding. The name bound also does not prevent a valid name exceeding a small
local resolver MTU; patch.7 addresses that separate send-loop progress defect.

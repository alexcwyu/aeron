# Driver and client defects, the sender mode and the signal correlation id

Part of `1.53.3-patch.5` (2026-11-01), on `patch/1.53.3-consolidated`.

## Defects fixed

| Where | Defect | Fix |
|---|---|---|
| `aeron_publication_image.c` | A DATA or PAD frame shorter than a complete data header passed validation, because only lengths `<= 0` were rejected. | Reject any frame shorter than the header. |
| `aeron_driver_name_resolver.c` | An entry's name length was checked against the whole packet instead of the bytes left in the frame, and a negative length was accepted: a crafted resolution entry read past the buffer. The IPv4 wildcard test also matched IPv6 entries. | Bound each entry by the bytes left, reject a negative length, test each family on its own bytes. |
| `aeron_send_channel_endpoint.c` | The send-timestamp loop stepped the iovec base in `aeron_data_header_t` units instead of bytes, so frames after the first in a multi-frame datagram were stamped at the wrong address. | Step in bytes. |
| `aeron_image.c` | `aeron_image_reject` followed the image's parent subscription, which a concurrent close can free. | Use the image's own conductor, which the retained image pins; fail with `EINVAL` once the image is closed. |

Native tests: `PublicationImageValidationTest` (3 cases, and the existing
fixtures now use valid frame lengths), `NameResolverMalformedEntryTest` (2
cases). Both suites fail against `1.53.3-patch.4` and pass here
(`publication_image_test`, `name_resolver_test`).

## Small API addition

* `aeron_archive_recording_signal_t` gains `correlation_id`, appended last so
  every existing field keeps its offset, filled from the signal event. Java's
  `RecordingSignalConsumer` receives it; the C struct dropped it.

## Not a defect

* `DEDICATED_SENDER_INVOKED`: a driver threading mode that leaves the sender
  duty cycle to the host thread, with an orderly sender shutdown
  (`aeron_driver_sender_drain_commands_for_shutdown`). Test:
  `sender_shutdown_test`.

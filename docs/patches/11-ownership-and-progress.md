# Ownership and progress fixes over patch.6

Patch.7 review batch, 2026-11-02. All production changes are BUG FIXes.

| Path | Failure | Correction |
|---|---|---|
| Client conductor retained-image close | Linger-array growth can fail after image ownership is removed; the destructive realloc can also discard existing linger entries. | Reserve failure-preserving capacity before image admission, account for every owned image until deletion, and make linger insertion allocation-free. Put unavailable images in the linger list before callbacks so an invoker callback can close their parent safely. |
| Debug loss suppliers | A second context copying a debug callback/clientd pair frees state owned by the first; changing only the callback frees and reattaches the same pointer. | Track ownership independently of callback identity. Preserve ownership when clientd is unchanged; other setters borrow clientd. |
| Cubic supplier | Failure of the second allocation frees the caller's output-slot address. | Free the allocated strategy object. This defect was inherited from upstream 1.53.3. |
| Resolver gossip | A valid remote name cannot fit MTU 64; the sender retries the same entry forever. | Skip entries that cannot fit an empty local frame, retain them for local lookup/expiry, and continue with later entries. Do not send empty frames. |
| Host-invoked Sender close | A native Conductor blocked on a full Sender queue cannot be joined after the host stops invoking. | Publish shutdown before joining; failed offers discard their untransferred commands using the same ownership rules as final queue draining. Other threading modes retain their existing behavior. |
| Host-invoked Sender startup | Sender bypasses agent_start, so its configured startup callback never runs. | Invoke it once on the host thread before the first duty cycle; recursive invokes during startup return without work. |

## Regression evidence

Serial Debug CMake build with `SANITISE_BUILD=ON` on macOS arm64:

- `driver_context_config_test`: 25 passed. Both new ownership cases abort with
  AddressSanitizer double-free before the fix and pass after it.
- `congestion_control_allocation_test`: first/second allocation failure paths
  verify the freed pointer, output, ENOMEM and cleanup count. The old operand
  aborts; the correction passes. Existing `congestion_control_test`: 13 passed.
- `name_resolver_test`: 17 passed, one existing disabled IPv6 case. MTU-64
  reproduction times out before the fix and returns after it. This proves
  progress and retained cache contents, not every gossip delivery combination.
- `sender_shutdown_test`: three passed. Startup test fails before the fix;
  native-Conductor/full-queue close times out before it. The producer fixture
  also covers cancellation of an owning add-destination URI. The separate
  existing test covers successful queueing and wrapped queue draining.
- `client_conductor_test` and `client_conductor_allocation_test`: 105 passed
  each, including 24 retained-image close variants. Fault injection makes the
  old close path fail; the corrected close performs no allocation. The test
  target compiles the production subscription and conductor sources so macOS
  two-level dynamic linking cannot bypass the injected allocator.

Independent source review checked ownership, cleanup and progress invariants.
Admission-OOM combinations, full native-driver saturation through client traffic,
Linux/Windows execution, and a fresh complete integration gate remain unproven.
These targeted results are not a claim that the whole fork or Rust workspace passed.

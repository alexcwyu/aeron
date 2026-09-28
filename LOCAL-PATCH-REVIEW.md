# Aeron 1.52.2 native patch review

Status: complete, 2026-09-13. User approved three independent PR commits instead
of one squash. Each commit's sole parent is upstream `1.52.2` (`5b62f21d91`).

1. Retained-image close: verify reference ownership, close state, unavailable
   callbacks and eventual reclamation. Correct the callback regression found in
   patch2; retain the subscription reference until the callback returns.
2. Destination registration: pin the original publication identity in the actual
   outbound add/remove command, with distinct client/original IDs.
3. Spy removal: pin the unavailable-image identity emitted by the real conductor
   when a spy destination is removed.

## Commits and review reports

| Branch | Commit | Independent ASan/UBSan suite | Rationale and evidence |
|---|---|---|---|
| `patch/1.52.2-retained-image-close-reviewed` | `44c81f9c09a059368ca04578439567ccc6300e23` | 102 passed | [Lifetime](docs/patches/1.52.2-retained-image-close.md) |
| `patch/1.52.2-async-registration-id-reviewed` | `c1f91d3289c223ffa00b540b22786cc7a963f52e` | 79 passed | [Registration](docs/patches/1.52.2-destination-original-registration.md) |
| `patch/1.52.2-spy-destination-removal` | `144d26cb8d1dda6dabe5a79cda1fe368bd48e7ed` | 18 passed | [Spy removal](docs/patches/1.52.2-spy-removal-image-identity.md) |

All three native negative controls fail against upstream for their named reason.
The combined source passed 235 native tests across six suites with ASan/UBSan,
and five live Rust-wrapper tests against an embedded native driver. Each PR was
also exported and built independently; every committed production/test blob was
checked against its independently tested export before commit creation.

Independent read-only reviewers traced lifetime ownership, destination identity,
caller paths, Java behavior, and the native tests. The final source rechecks
found no remaining blocker in these three fixes. The separate upstream
remove-by-destination-ID bug remains documented, not repaired by these PRs.

`patch/1.52.2-consolidated` combines all three reviewed branches in a merge
commit, preserving their original commit IDs. The merged production/test bytes
match the previously tested combined source; all six affected native suites
were rebuilt serially and passed again before integration. This index document
belongs to the consolidated branch; each independent PR retains its standalone
report. Existing patch branches remain unchanged.

The user authorized pushing these three reviewed branches and the consolidated
branch to `origin` on 2026-09-13. No force push or PR creation is required. The
foundation repository's submodule pointer is not staged or committed here.

Raw logs and exported test trees: `/tmp/aeron-native-review-20260913/`.
Per-PR Markdown retains summarized evidence and log hashes so raw logs can be
cleaned later. Historical Docker runs are explicitly not claimed as validation
of the new lifetime callback correction.

## Rebase onto Aeron 1.53.3 (2026-09-26)

Base moved from upstream `1.52.2` (`5b62f21d91`) to upstream `1.53.3`
(`377943bcb8`), branch `patch/1.53.3-consolidated`, tag `1.53.3-patch.1`.
`1.52.2` is not an ancestor of `1.53.3` (cut from a 1.52.x release branch;
merge-base `1.52.0`), so the four fixes were cherry-picked in dependency
order onto the `1.53.3` tag rather than rebased in place:

| Order | Commit (new, on `patch/1.53.3-consolidated`) | Cherry-picked from | Fix |
|---|---|---|---|
| 1 | `45d780736a` | `144d26cb8d` | Spy removal |
| 2 | `cb3621c852` | `c1f91d3289` | Destination registration |
| 3 | `52fb57add1` | `44c81f9c09` | Retained-image close |
| 4 | `bfd722e44f` | `c456fa85c7` | Sleeping idle strategy state leak |

Only the destination-registration cherry-pick (`c1f91d3289`) conflicted, and
only in the test file `aeron-client/src/test/c/aeron_client_conductor_test.cpp`:
upstream had independently added `TEST_F(ClientConductorTest,
shouldRemovePublicationDestinationById)` at the same spot where this fix adds
`TEST_F(ClientConductorTest, shouldTargetOriginalPublicationForDestinationCommands)`.
Resolved by keeping both tests, upstream's first, each closing its own brace;
no production code conflicted.

`diff <(git diff 1.53.3 HEAD -- '*.c' '*.h') <(git diff 1.52.2 c456fa85c7 -- '*.c' '*.h')`
filtered to changed (`+`/`-`) lines produced no output: the net C-source delta
introduced onto `1.53.3` is byte-identical in substance to the net delta the
original three-plus-one fixes introduced onto `1.52.2`, modulo context-line
shift.

Redundancy check against upstream `1.53.3` (all four fixes still required):
- `aeron_client_conductor_get_async_registration_id` in
  `aeron-client/src/main/c/aeron_client_conductor.c` still returns
  `registration_id`, not `original_registration_id`, for publications and
  exclusive publications. Upstream's own `aa03d702d0` (present in `1.53.3`)
  fixed a different field, `destination_registration_id` in
  `aeron_client_conductor_on_cmd_destination_by_id`; it does not overlap.
- `aeron-driver/src/main/c/aeron_network_publication.c`'s
  `aeron_network_publication_create` still leaves
  `conductor_fields.subscribable.correlation_id` uninitialized (no assignment
  from `registration_id` in the init block).
- `aeron-client/src/main/c/aeron_agent.c`'s
  `aeron_idle_strategy_sleeping_init_args` still calls `aeron_alloc` for the
  state before `aeron_parse_duration_ns`, so a parse failure still leaks the
  allocation.
- The retained-image double-decrement/skip-`aeron_image_close` bug has no
  upstream commit touching that path in `1.53.3`.

No fix was dropped. `patch/1.52.2-consolidated` (tip `c456fa85c7`) is
unchanged by this rebase.

## Public setters for C-only settings (1.53.3-patch.2, 2026-09-28)

`1.53.3-patch.2` is `1.53.3-patch.1` plus public set/get pairs for the C
settings that had none. An inventory of every `AERON_*` environment variable
the driver, client and archive-client C libraries define, and of the
`aeron_driver_context_t` fields, found them. One commit per setting family,
each with its own `docs/patches/` entry; a setter always wins over the
environment variable, and a variable that is read lazily today is still read
lazily. No benchmark-driven proposal was approved for this tag, so none is
included.

| Order | Family | Public header | Test | Upstream candidate | Detail |
|---|---|---|---|---|---|
| 5 | Cubic congestion control (`measure_rtt`, `initial_rtt_ns`, `tcp_mode`) | `aeronmd.h` | `driver_context_config_test`, `congestion_control_test` | yes | [Cubic](docs/patches/05-cubic-congestion-control-setters.md) |
| 6 | Conductor UDP channel transport bindings | `aeronmd.h` | `driver_context_config_test` | yes | [Conductor bindings](docs/patches/06-conductor-udp-channel-transport-bindings-setter.md) |
| 7 | Send and receive channel loss suppliers (moved from the internal header, plus getters) | `aeronmd.h` | `driver_context_config_test`, `aeron_test_loss_generators_test` | yes | [Loss suppliers](docs/patches/07-channel-loss-supplier-public-api.md) |
| 8 | Unicast and multicast flow control retransmit receiver window multiples (`rrwm`) | `aeronmd.h` | `driver_context_config_test`, `flow_control_test` | yes | [rrwm](docs/patches/08-flow-control-rrwm-setters.md) |

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

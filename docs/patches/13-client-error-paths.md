# Client error paths (local patch.7 follow-up)

These BUG FIX changes extend the local patch.7 tag without changing the wire
format or editing generated bindings/codecs. The fork has not been pushed.

- Blocking archive operations invoke an attached manually driven Aeron client
  while waiting for registrations or responses. A manual-client recorded-publication
  regression times out before the fix and completes after it.
- Recording descriptor pollers ignore matching descriptors when no consumer has
  been armed. Both descriptor kinds previously called a null function pointer;
  generated-codec fixtures abort before the fix and pass after it.
- Background client handler commands retain heap-owned command storage until the
  conductor finishes or discards them. A timed-out pending command is cancelled
  rather than applied later through expired stack storage. The timeout regression
  fails before; all 106 conductor tests pass after. Shutdown drain releases the
  queue's ownership and wakes a still-waiting caller.
- Fragment assembler constructors release partial allocations. Buffer growth
  allocates before replacing the old buffer, preserving the prefix on failure.
  All four handlers check append failures: ordinary polling drops the incomplete
  message; controlled polling aborts the fragment and retains a retryable prefix.
  Failed session-map insertion removes any inserted entry before freeing its
  builder and restores the prior resize threshold.

Parent `target/review/` holds the before/after receipts. Seven allocator-failure
cases and fourteen handler-failure cases fail before their fixes and pass after.
The four regular assembler suites pass 30 tests. The map-threshold regression
also checks the real rehash-failure side effect: the map must retain its previous
resize threshold after rejecting an insertion. Complete release validation is
recorded separately by the parent repository's gate, not implied by these tests.

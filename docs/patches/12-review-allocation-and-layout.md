# Additional allocation and layout fixes (local patch.7 follow-up)

These BUG FIX changes follow the locally tagged patch.7 commit. They extend the local patch.7 tag; neither correction commit has been pushed. No generated bindings or
codecs were edited.

- Archive channel setters allocate a replacement before releasing the previous
  string. Allocation failure preserves the old channel and allows context close.
- Terminal archive connection failure releases both descriptor pollers and the
  async connection. Poller creation releases its allocation if assembler creation
  fails. Successful ownership transfer is unchanged.
- The shared CnC loader validates nonnegative buffer lengths, ring capacities and
  alignment, complete counter records and metadata/value capacity consistency
  before computing buffer pointers. Invalid layouts return EINVAL.
- Recording-position source identity lookup rejects a negative or oversized length
  outside the counter metadata key before copying it; valid identities retain
  destination-length truncation behavior.

Regression evidence is stored in the parent repository target/review directory.
Three channel allocation fault cases failed with AddressSanitizer before and pass
after. Five terminal allocation fault cases failed their allocation accounting
before and pass after. The parent CnC malformed-layout and misalignment tests fail
before and pass after. The recording identity regression fails before; the full
archive library suite passes after (888 passed, one ignored). The complete final
gate and independent verification of every change are not yet complete.

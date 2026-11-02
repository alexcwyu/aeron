# Additional allocation and layout fixes (patch.7 follow-up)

These BUG FIX changes are part of the patch.7 tag. No generated bindings or codecs
were edited.

- Archive channel setters allocate a replacement before releasing the previous
  string. Allocation failure preserves the old channel and allows context close.
- Terminal archive connection failure releases both descriptor pollers and the
  async connection. Poller creation releases its allocation if assembler creation
  fails. Successful ownership transfer is unchanged.
- The shared CnC loader validates nonnegative buffer lengths, ring capacities and
  alignment, and that the metadata buffer covers every complete value record,
  before computing buffer pointers. Invalid layouts return EINVAL. A length that is
  not a whole number of records stays valid, as both drivers accept it (for example
  1,500,000 bytes); the reader ignores the partial trailing record.
- Recording-position source identity lookup reads the length once, then rejects a
  negative or oversized length outside the counter metadata key before copying it;
  valid identities retain destination-length truncation behavior.

Regression evidence is stored in the parent repository target/review directory.
Three channel allocation fault cases failed with AddressSanitizer before and pass
after. Five terminal allocation fault cases failed their allocation accounting
before and pass after. The parent CnC malformed-layout and misalignment tests fail
before and pass after. The recording identity regression fails before; the full
archive library suite passes after (888 passed, one ignored). The independent
recheck added the parent regression `cnc_counter_lengths_need_not_be_whole_records`,
which fails against the earlier record-multiple check and passes after it was removed.

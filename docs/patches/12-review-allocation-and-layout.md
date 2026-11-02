# Additional allocation and layout fixes (patch.7 follow-up)

These BUG FIX changes are part of the patch.7 tag. No generated bindings or codecs
were edited.

- Archive channel setters allocate a replacement before releasing the previous
  string. Allocation failure preserves the old channel and allows context close.
- Terminal archive connection failure releases both descriptor pollers and the
  async connection. Poller creation releases its allocation if assembler creation
  fails. Successful ownership transfer is unchanged.
- The shared CnC loader validates nonnegative buffer lengths, ring capacities and
  alignment, counter buffer lengths that are multiples of 8 (so the values buffer
  and the error log stay aligned for 64-bit atomics), and that the metadata buffer
  covers every complete value record, before computing buffer pointers. Invalid
  layouts return EINVAL. A length that is not a whole number of records stays valid,
  as both drivers accept it (for example 1,500,000 bytes); the reader ignores the
  partial trailing record.
- Counter iteration (`aeron_counters_reader_foreach_counter`,
  `aeron_counters_reader_find_by_type_id_and_registration_id` and
  `aeron_counters_reader_foreach_metadata`) visits only complete metadata records
  and, where it reads values, only ids up to `max_counter_id`. An allocated entry
  past the values buffer was read out of bounds before; Java's bounds-checked
  buffers throw there instead. The id bound guards the value reads. The
  complete-record bound guards `aeron_counters_reader_foreach_metadata`, which has no
  id bound and which the local socket-address lookup runs over the CnC's own metadata:
  metadata 992 / values 32 passes the loader, and an allocated record at 512 was read
  past the metadata buffer before. Valid layouts iterate exactly as before, since a
  driver never allocates past `max_counter_id`.
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
A second recheck added `cnc_counter_buffers_misaligned_for_atomics_are_rejected`
(metadata 513 / values 128, and 1024 / 132, were accepted before) and
`counter_iteration_stops_at_the_last_complete_record` (an allocated entry past the
values buffer was visited, and its value read, before); both fail before and pass
after. The fork test `CountersMetadataTest.shouldVisitCompleteMetadataRecordsOnly`
visits only the whole record of a 992-byte buffer (it visited the partial one too before).

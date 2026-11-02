#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <string.h>
#include "client/aeron_archive_recording_descriptor_poller.h"
#include "client/aeron_archive_recording_subscription_descriptor_poller.h"
#include "c/aeron_archive_client/recordingDescriptor.h"
#include "c/aeron_archive_client/recordingSubscriptionDescriptor.h"

aeron_controlled_fragment_handler_action_t aeron_archive_recording_descriptor_poller_on_fragment(
    void *, const uint8_t *, size_t, aeron_header_t *);
aeron_controlled_fragment_handler_action_t aeron_archive_recording_subscription_descriptor_poller_on_fragment(
    void *, const uint8_t *, size_t, aeron_header_t *);

int main(int argc, char **argv)
{
    char buffer[512] = { 0 };
    struct aeron_archive_client_messageHeader header;
    if (argc == 1 || strcmp(argv[1], "recording") == 0)
    {
        struct aeron_archive_client_recordingDescriptor descriptor;
        aeron_archive_client_recordingDescriptor_wrap_and_apply_header(&descriptor, buffer, 0, sizeof(buffer), &header);
        aeron_archive_client_recordingDescriptor_set_controlSessionId(&descriptor, 7);
        aeron_archive_client_recordingDescriptor_set_correlationId(&descriptor, 0);
        aeron_archive_client_recordingDescriptor_put_strippedChannel(&descriptor, "", 0);
        aeron_archive_client_recordingDescriptor_put_originalChannel(&descriptor, "", 0);
        aeron_archive_client_recordingDescriptor_put_sourceIdentity(&descriptor, "", 0);
        aeron_archive_recording_descriptor_poller_t poller = { 0 };
        poller.control_session_id = 7;
        size_t length = aeron_archive_client_messageHeader_encoded_length() +
            aeron_archive_client_recordingDescriptor_encoded_length(&descriptor);
        assert(AERON_ACTION_CONTINUE == aeron_archive_recording_descriptor_poller_on_fragment(
            &poller, (const uint8_t *)buffer, length, NULL));
        assert(0 == poller.remaining_record_count);
    }
    if (argc == 1 || strcmp(argv[1], "subscription") == 0)
    {
        struct aeron_archive_client_recordingSubscriptionDescriptor descriptor;
        aeron_archive_client_recordingSubscriptionDescriptor_wrap_and_apply_header(
            &descriptor, buffer, 0, sizeof(buffer), &header);
        aeron_archive_client_recordingSubscriptionDescriptor_set_controlSessionId(&descriptor, 7);
        aeron_archive_client_recordingSubscriptionDescriptor_set_correlationId(&descriptor, 0);
        aeron_archive_client_recordingSubscriptionDescriptor_put_strippedChannel(&descriptor, "", 0);
        aeron_archive_recording_subscription_descriptor_poller_t poller = { 0 };
        poller.control_session_id = 7;
        size_t length = aeron_archive_client_messageHeader_encoded_length() +
            aeron_archive_client_recordingSubscriptionDescriptor_encoded_length(&descriptor);
        assert(AERON_ACTION_CONTINUE == aeron_archive_recording_subscription_descriptor_poller_on_fragment(
            &poller, (const uint8_t *)buffer, length, NULL));
        assert(0 == poller.remaining_subscription_count);
    }
    return 0;
}

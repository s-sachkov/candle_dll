#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "candle.h"

#define INDENT_DEV_INFO "  "
#define INDENT_CHANNEL_INFO "   "

static void print_channel_min_max(uint8_t ch, const char *prefix, uint32_t min, uint32_t max)
{
    fprintf(stdout, INDENT_CHANNEL_INFO "[%" PRIu8 "] %s: min %" PRIu32 ", max %" PRIu32 "\n", ch, prefix, min, max);
}

static void print_channel_value(uint8_t ch, const char *prefix, uint32_t value)
{
    fprintf(stdout, INDENT_CHANNEL_INFO "[%" PRIu8 "] %s: %" PRIu32 "\n", ch, prefix, value);
}

static void print_device_error(candle_handle hdev, const char *func)
{
    candle_err_t errnum = candle_dev_last_error(hdev);
    fprintf(stderr, INDENT_DEV_INFO "[%" PRIxPTR "] '%s' error: %d\n", (uintptr_t)hdev, func, errnum);
}

static void print_device_info_value(candle_handle hdev, const char *prefix, uint32_t value)
{
    fprintf(stdout, INDENT_DEV_INFO "[%" PRIxPTR "] %s: %" PRIu32 "\n", (uintptr_t)hdev, prefix, value);
}

static void print_device_info_str(candle_handle hdev, const char *prefix, const char *value)
{
    fprintf(stdout, INDENT_DEV_INFO "[%" PRIxPTR "] %s: %s\n", (uintptr_t)hdev, prefix, value);
}

static void print_lib_error(const char *func)
{
    fprintf(stderr, "Error call '%s'\n", func);
}

static bool print_device_info(candle_handle hdev)
{
    bool res                = false;
    candle_devstate_t state = CANDLE_DEVSTATE_INUSE;
    if (!candle_dev_get_state(hdev, &state)) {
        print_device_error(hdev, "candle_dev_get_state");
        goto end;
    }
    print_device_info_str(hdev, "state", state == CANDLE_DEVSTATE_AVAIL ? "AVAILABLE" : "IN USE");

    wchar_t *path = candle_dev_get_path(hdev);
    if (!path) {
        print_device_error(hdev, "candle_dev_get_path");
        goto end;
    }
    print_device_info_str(hdev, "path", (char *)path);
    if (!candle_dev_open(hdev)) {
        print_device_error(hdev, "candle_dev_open");
        goto end;
    }
    uint32_t timestamp_us = 0;
    if (!candle_dev_get_timestamp_us(hdev, &timestamp_us)) {
        print_device_error(hdev, "candle_dev_get_timestamp_us");
    } else {
        print_device_info_value(hdev, "timestamp_us", timestamp_us);
    }
    uint8_t num_channels = 0;
    if (!candle_channel_count(hdev, &num_channels)) {
        print_device_error(hdev, "candle_channel_count");
        goto close;
    }
    print_device_info_value(hdev, "channels", num_channels);
    for (uint8_t ch = 0; ch < num_channels; ++ch) {
        candle_capability_t cap;
        memset(&cap, 0, sizeof(cap));
        if (!candle_channel_get_capabilities(hdev, ch, &cap)) {
            print_device_error(hdev, "candle_channel_get_capabilities");
            continue;
        }
        print_channel_value(ch, "support 'normal'", cap.feature & CANDLE_FEATURE_NORMAL);
        print_channel_value(ch, "support 'listen_only'", cap.feature & CANDLE_FEATURE_LISTEN_ONLY);
        print_channel_value(ch, "support 'loop_back'", cap.feature & CANDLE_FEATURE_LOOP_BACK);
        print_channel_value(ch, "support 'triple_sample'", cap.feature & CANDLE_FEATURE_TRIPLE_SAMPLE);
        print_channel_value(ch, "support 'one_shot'", cap.feature & CANDLE_FEATURE_ONE_SHOT);
        print_channel_value(ch, "support 'hw_timestamp'", cap.feature & CANDLE_FEATURE_HW_TIMESTAMP);
        print_channel_value(ch, "support 'identify'", cap.feature & CANDLE_FEATURE_IDENTIFY);
        print_channel_value(ch, "support 'user_id'", cap.feature & CANDLE_FEATURE_USER_ID);
        print_channel_value(ch, "support 'pad_pkts_to_max_pkt_size'", cap.feature & CANDLE_FEATURE_PAD_PKTS_TO_MAX_PKT_SIZE);
        print_channel_value(ch, "fclk_can", cap.fclk_can);
        print_channel_min_max(ch, "tseg1", cap.tseg1_min, cap.tseg1_max);
        print_channel_min_max(ch, "tseg2", cap.tseg2_min, cap.tseg2_max);
        print_channel_value(ch, "sjw_max", cap.sjw_max);
        print_channel_min_max(ch, "brp", cap.brp_min, cap.brp_max);
        print_channel_value(ch, "brp_inc", cap.brp_inc);
    }
    res = true;

close:
    if (!candle_dev_close(hdev)) {
        print_device_error(hdev, "candle_dev_close");
        res = false;
    }
end:
    return res;
}

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    int res = EXIT_FAILURE;
    candle_list_handle list;
    uint8_t len;

    if (!candle_list_scan(&list)) {
        print_lib_error("candle_list_scan");
        goto end;
    }

    if (!candle_list_length(list, &len)) {
        print_lib_error("candle_list_length");
        goto free_list;
    }

    fprintf(stdout, "Founded devices: %d\n", len);
    for (uint8_t i = 0; i < len; ++i) {
        candle_handle hdev;
        if (!candle_dev_get(list, i, &hdev)) {
            print_lib_error("candle_list_length");
            continue;
        }
        print_device_info(hdev);
        if (!candle_dev_free(hdev)) {
            print_lib_error("candle_dev_free");
        }
    }
    res = EXIT_SUCCESS;

free_list:
    if (!candle_list_free(list)) {
        print_lib_error("candle_list_free");
        res = EXIT_FAILURE;
    }
end:
    return res;
}
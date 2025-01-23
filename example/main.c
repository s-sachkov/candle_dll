#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <windows.h>

#include "candle.h"

#define INDENT_DEV_INFO     "- "
#define INDENT_CHANNEL_INFO "  - "

static void print_channel_min_max(uint8_t ch, const char *prefix, uint32_t min, uint32_t max)
{
    (void)ch;
    fprintf(stdout, INDENT_CHANNEL_INFO "%s: min %" PRIu32 ", max %" PRIu32 "\n", prefix, min, max);
}

static void print_channel_value(uint8_t ch, const char *prefix, uint32_t value)
{
    (void)ch;
    fprintf(stdout, INDENT_CHANNEL_INFO "%s: %" PRIu32 "\n", prefix, value);
}

static void print_device_error(candle_handle hdev, const char *func)
{
    candle_err_t errnum = candle_dev_last_error(hdev);
    fprintf(stderr, INDENT_DEV_INFO "Error call '%s': %d\n", func, errnum);
}

static void print_device_info_value(candle_handle hdev, const char *prefix, uint32_t value)
{
    (void)hdev;
    fprintf(stdout, INDENT_DEV_INFO "%s: %" PRIu32 "\n", prefix, value);
}

static void print_device_info_str(candle_handle hdev, const char *prefix, const char *value)
{
    (void)hdev;
    fprintf(stdout, INDENT_DEV_INFO "%s: %s\n", prefix, value);
}

static void print_device_info_wstr(candle_handle hdev, const char *prefix, const wchar_t *value)
{
    (void)hdev;
    fprintf(stdout, INDENT_DEV_INFO "%s: %ls\n", prefix, value);
}

static void print_lib_error(const char *func)
{
    fprintf(stderr, "Error call '%s'\n", func);
}

static bool print_device_info(candle_handle hdev)
{
    bool res = false;
    candle_devstate_t state = CANDLE_DEVSTATE_INUSE;
    if (!candle_dev_get_state(hdev, &state)) {
        print_device_error(hdev, "candle_dev_get_state");
        goto end;
    }
    print_device_info_str(hdev, "State", state == CANDLE_DEVSTATE_AVAIL ? "available" : "in use");

    wchar_t *path = candle_dev_get_path(hdev);
    if (!path) {
        print_device_error(hdev, "candle_dev_get_path");
        goto end;
    }
    print_device_info_wstr(hdev, "Path", path);
    if (!candle_dev_open(hdev)) {
        print_device_error(hdev, "candle_dev_open");
        goto end;
    }
    uint32_t timestamp_us = 0;
    if (!candle_dev_get_timestamp_us(hdev, &timestamp_us)) {
        print_device_error(hdev, "candle_dev_get_timestamp_us");
    } else {
        print_device_info_value(hdev, "Timestamp (us)", timestamp_us);
    }
    uint8_t num_channels = 0;
    if (!candle_channel_count(hdev, &num_channels)) {
        print_device_error(hdev, "candle_channel_count");
        goto close;
    }

    for (uint8_t ch = 0; ch < num_channels; ++ch) {
        fprintf(stdout, INDENT_DEV_INFO "Channel %d/%d:\n", ch + 1, num_channels);

        candle_capability_t cap;
        memset(&cap, 0, sizeof(cap));
        if (!candle_channel_get_capabilities(hdev, ch, &cap)) {
            print_device_error(hdev, "candle_channel_get_capabilities");
            continue;
        }
        print_channel_value(ch, "Feature 'listen_only'", (cap.feature & CANDLE_FEATURE_LISTEN_ONLY) != 0);
        print_channel_value(ch, "Feature 'listen_only'", (cap.feature & CANDLE_FEATURE_LISTEN_ONLY) != 0);
        print_channel_value(ch, "Feature 'loop_back'", (cap.feature & CANDLE_FEATURE_LOOP_BACK) != 0);
        print_channel_value(ch, "Feature 'triple_sample'", (cap.feature & CANDLE_FEATURE_TRIPLE_SAMPLE) != 0);
        print_channel_value(ch, "Feature 'one_shot'", (cap.feature & CANDLE_FEATURE_ONE_SHOT) != 0);
        print_channel_value(ch, "Feature 'hw_timestamp'", (cap.feature & CANDLE_FEATURE_HW_TIMESTAMP) != 0);
        print_channel_value(ch, "Feature 'identify'", (cap.feature & CANDLE_FEATURE_IDENTIFY) != 0);
        print_channel_value(ch, "Feature 'user_id'", (cap.feature & CANDLE_FEATURE_USER_ID) != 0);
        print_channel_value(ch, "Feature 'pad_pkts_to_max_pkt_size'", (cap.feature & CANDLE_FEATURE_PAD_PKTS_TO_MAX_PKT_SIZE) != 0);
        print_channel_value(ch, "Fclk_can", cap.fclk_can);
        print_channel_min_max(ch, "Tseg1", cap.tseg1_min, cap.tseg1_max);
        print_channel_min_max(ch, "Tseg2", cap.tseg2_min, cap.tseg2_max);
        print_channel_value(ch, "Sjw_max", cap.sjw_max);
        print_channel_min_max(ch, "Brp", cap.brp_min, cap.brp_max);
        print_channel_value(ch, "Brp_inc", cap.brp_inc);
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
    for (uint8_t i = 0; i < len; ++i) {
        fprintf(stdout, "Device %d/%d:\n", i + 1, len);
        candle_handle hdev;
        if (!candle_dev_get(list, i, &hdev)) {
            print_lib_error("candle_list_length");
            continue;
        }
        print_device_info(hdev);
        if (!candle_dev_open(hdev)) {
            print_device_error(hdev, "candle_dev_open");
            goto end;
        }
        const uint8_t ch = 0;
        if (!candle_channel_set_bitrate(hdev, ch, 500000))
        {
            print_device_error(hdev, "candle_channel_set_bitrate");
        }

        if (!candle_channel_start(hdev, ch, 0))
        {
            print_device_error(hdev, "candle_channel_start");
        }

        for(int i = 0; i < 1000; ++i)
        {
            candle_frame_t frame;
            frame.can_id = i;
            frame.can_dlc = 8;
            frame.data[0] = i;
            frame.data[1] = i;
            frame.data[2] = i;
            frame.data[3] = i;
            frame.data[4] = i;
            frame.data[5] = i;
            frame.data[6] = i;
            frame.data[7] = i;

            if (!candle_frame_send(hdev, ch, &frame))
            {
                print_device_error(hdev, "candle_frame_send");
                goto end;
            }
            Sleep(20);
        }
        if (!candle_channel_stop(hdev, ch))
        {
            print_device_error(hdev, "candle_channel_stop");
        }
        if (!candle_dev_close(hdev)) {
            print_device_error(hdev, "candle_dev_close");
            goto end;
        }
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

#pragma once

#include <gui/view.h>
#include <furi.h>

typedef struct NfcEmulateView NfcEmulateView;

typedef enum {
    NfcEmulateViewEventStop,
} NfcEmulateViewEvent;

typedef void (*NfcEmulateViewCallback)(NfcEmulateViewEvent event, void* context);

NfcEmulateView* nfc_emulate_view_alloc(void);

void nfc_emulate_view_free(NfcEmulateView* instance);

View* nfc_emulate_view_get_view(NfcEmulateView* instance);

void nfc_emulate_view_reset(NfcEmulateView* instance);

void nfc_emulate_view_set_callback(
    NfcEmulateView* instance,
    NfcEmulateViewCallback callback,
    void* context);

void nfc_emulate_view_set_data(
    NfcEmulateView* instance,
    const char* card_name,
    const char* protocol_name,
    const char* uid_str);

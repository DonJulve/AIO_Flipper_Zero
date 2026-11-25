#pragma once

#include <gui/view.h>
#include <furi.h>

typedef struct RfidEmulateView RfidEmulateView;

typedef enum {
    RfidEmulateViewEventStop,
} RfidEmulateViewEvent;

typedef void (*RfidEmulateViewCallback)(RfidEmulateViewEvent event, void* context);

RfidEmulateView* rfid_emulate_view_alloc(void);

void rfid_emulate_view_free(RfidEmulateView* instance);

View* rfid_emulate_view_get_view(RfidEmulateView* instance);

void rfid_emulate_view_reset(RfidEmulateView* instance);

void rfid_emulate_view_set_callback(
    RfidEmulateView* instance,
    RfidEmulateViewCallback callback,
    void* context);

void rfid_emulate_view_set_data(
    RfidEmulateView* instance,
    const char* card_name,
    const char* protocol_name,
    const char* data_str);

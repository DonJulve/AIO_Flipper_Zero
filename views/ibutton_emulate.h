#pragma once

#include <gui/view.h>
#include <furi.h>

typedef struct iButtonEmulateView iButtonEmulateView;

typedef enum {
    iButtonEmulateViewEventStop,
} iButtonEmulateViewEvent;

typedef void (*iButtonEmulateViewCallback)(iButtonEmulateViewEvent event, void* context);

iButtonEmulateView* ibutton_emulate_view_alloc(void);

void ibutton_emulate_view_free(iButtonEmulateView* instance);

View* ibutton_emulate_view_get_view(iButtonEmulateView* instance);

void ibutton_emulate_view_reset(iButtonEmulateView* instance);

void ibutton_emulate_view_set_callback(
    iButtonEmulateView* instance,
    iButtonEmulateViewCallback callback,
    void* context);

void ibutton_emulate_view_set_data(
    iButtonEmulateView* instance,
    const char* key_name,
    const char* protocol_name,
    const char* data_str);

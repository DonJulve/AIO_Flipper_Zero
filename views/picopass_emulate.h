#pragma once

#include <gui/view.h>
#include <furi.h>

typedef struct PicopassEmulateView PicopassEmulateView;

typedef enum {
    PicopassEmulateViewEventStop,
} PicopassEmulateViewEvent;

typedef void (*PicopassEmulateViewCallback)(PicopassEmulateViewEvent event, void* context);

PicopassEmulateView* picopass_emulate_view_alloc(void);

void picopass_emulate_view_free(PicopassEmulateView* instance);

View* picopass_emulate_view_get_view(PicopassEmulateView* instance);

void picopass_emulate_view_reset(PicopassEmulateView* instance);

void picopass_emulate_view_set_callback(
    PicopassEmulateView* instance,
    PicopassEmulateViewCallback callback,
    void* context);

void picopass_emulate_view_set_data(
    PicopassEmulateView* instance,
    const char* card_name,
    const char* protocol_name,
    const char* csn_str);

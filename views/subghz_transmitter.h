#pragma once

#include <gui/view.h>
#include <furi.h>

typedef struct SubGhzTransmitterView SubGhzTransmitterView;

typedef enum {
    SubGhzTransmitterViewEventSendStart,
    SubGhzTransmitterViewEventSendStop,
} SubGhzTransmitterViewEvent;

typedef void (*SubGhzTransmitterViewCallback)(SubGhzTransmitterViewEvent event, void* context);

SubGhzTransmitterView* subghz_transmitter_view_alloc(void);

void subghz_transmitter_view_free(SubGhzTransmitterView* instance);

View* subghz_transmitter_view_get_view(SubGhzTransmitterView* instance);

void subghz_transmitter_view_reset(SubGhzTransmitterView* instance);

void subghz_transmitter_view_set_callback(
    SubGhzTransmitterView* instance,
    SubGhzTransmitterViewCallback callback,
    void* context);

void subghz_transmitter_view_set_data(
    SubGhzTransmitterView* instance,
    const char* key_info,
    const char* frequency,
    const char* modulation,
    bool can_tx);

void subghz_transmitter_view_set_tx(SubGhzTransmitterView* instance, bool is_tx);


#include <furi.h>

#include <gui/view_dispatcher.h>
#include <gui/scene_manager.h>
#include <notification/notification_messages.h>
#include <flipper_format/flipper_format.h>

#include "julve_aio.h"
#include "scenes.h"
#include "scene_subghz_transmitter.h"
#include "../actions/action_i.h"
#include "../actions/helpers/subghz_txrx.h"
#include "../views/subghz_transmitter.h"

#define SUBGHZ_TX_MIN_MS 350

typedef enum {
    SubGhzTxSceneEventSendStart = 100,
    SubGhzTxSceneEventSendStop,
} SubGhzTxSceneEvent;

typedef struct {
    SubGhzNeedSaveContext save_ctx;
    FuriString* file_path;
    uint32_t tx_start_tick;
    bool is_tx;
} SubGhzTransmitterSceneState;

static void scene_subghz_transmitter_view_callback(
    SubGhzTransmitterViewEvent event,
    void* context) {
    App* app = context;
    if(event == SubGhzTransmitterViewEventSendStart) {
        view_dispatcher_send_custom_event(app->view_dispatcher, SubGhzTxSceneEventSendStart);
    } else if(event == SubGhzTransmitterViewEventSendStop) {
        view_dispatcher_send_custom_event(app->view_dispatcher, SubGhzTxSceneEventSendStop);
    }
}

static void scene_subghz_transmitter_update_view(App* app) {
    if(!app->subghz_txrx) return;

    FuriString* key_info = furi_string_alloc();
    FuriString* frequency = furi_string_alloc();
    FuriString* modulation = furi_string_alloc();

    SubGhzProtocolDecoderBase* decoder = subghz_txrx_get_decoder(app->subghz_txrx);
    if(decoder) {
        subghz_protocol_decoder_base_get_string(decoder, key_info);
    }
    subghz_txrx_get_frequency_and_modulation(app->subghz_txrx, frequency, modulation);
    bool can_tx = subghz_txrx_protocol_is_transmittable(app->subghz_txrx, false);

    subghz_transmitter_view_set_data(
        app->subghz_transmitter,
        furi_string_get_cstr(key_info),
        furi_string_get_cstr(frequency),
        furi_string_get_cstr(modulation),
        can_tx);

    furi_string_free(key_info);
    furi_string_free(frequency);
    furi_string_free(modulation);
}

void scene_subghz_transmitter_on_enter(void* context) {
    App* app = context;
    furi_assert(app->subghz_txrx);

    SubGhzTransmitterSceneState* state = malloc(sizeof(SubGhzTransmitterSceneState));
    state->file_path = furi_string_alloc_set(app->temp_str);
    state->save_ctx.app = app;
    state->save_ctx.txrx = app->subghz_txrx;
    state->save_ctx.file_path = state->file_path;
    state->tx_start_tick = 0;
    state->is_tx = false;
    app->subghz_scene_state = state;

    subghz_txrx_set_need_save_callback(
        app->subghz_txrx, action_subghz_need_save_callback, &state->save_ctx);

    subghz_transmitter_view_set_callback(
        app->subghz_transmitter, scene_subghz_transmitter_view_callback, app);
    scene_subghz_transmitter_update_view(app);

    view_dispatcher_switch_to_view(app->view_dispatcher, QView_SubGhzTransmitter);
}

bool scene_subghz_transmitter_on_event(void* context, SceneManagerEvent event) {
    App* app = context;
    SubGhzTransmitterSceneState* state = app->subghz_scene_state;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom && state && app->subghz_txrx) {
        if(event.event == SubGhzTxSceneEventSendStart) {
            consumed = true;
            if(!state->is_tx) {
                notification_message(app->notifications, &sequence_blink_start_blue);
                SubGhzTxRxStartTxState tx_state = subghz_txrx_tx_start(
                    app->subghz_txrx, subghz_txrx_get_fff_data(app->subghz_txrx));
                if(tx_state == SubGhzTxRxStartTxStateOk) {
                    state->is_tx = true;
                    state->tx_start_tick = furi_get_tick();
                    subghz_transmitter_view_set_tx(app->subghz_transmitter, true);
                } else {
                    FURI_LOG_E(TAG, "SUBGHZ: Failed to start TX (%d)", tx_state);
                    notification_message(app->notifications, &sequence_blink_stop);
                    notification_message(app->notifications, &sequence_error);
                    subghz_transmitter_view_set_tx(app->subghz_transmitter, false);
                }
            }
        } else if(event.event == SubGhzTxSceneEventSendStop) {
            consumed = true;
            if(state->is_tx) {
                uint32_t elapsed_ticks = furi_get_tick() - state->tx_start_tick;
                uint32_t min_ticks = furi_ms_to_ticks(SUBGHZ_TX_MIN_MS);
                if(elapsed_ticks < min_ticks) {
                    uint32_t ticks_per_ms = furi_ms_to_ticks(1);
                    uint32_t remain_ms =
                        ticks_per_ms ? (min_ticks - elapsed_ticks) / ticks_per_ms : 0;
                    if(remain_ms > 0 && remain_ms <= SUBGHZ_TX_MIN_MS) {
                        furi_delay_ms(remain_ms);
                    }
                }

                subghz_txrx_stop(app->subghz_txrx);
                notification_message(app->notifications, &sequence_blink_stop);
                state->is_tx = false;
                subghz_transmitter_view_set_tx(app->subghz_transmitter, false);

                FlipperFormat* fff_data = subghz_txrx_get_fff_data(app->subghz_txrx);
                SubGhzProtocolDecoderBase* decoder = subghz_txrx_get_decoder(app->subghz_txrx);
                if(fff_data && decoder) {
                    flipper_format_rewind(fff_data);
                    subghz_protocol_decoder_base_deserialize(decoder, fff_data);
                }
                scene_subghz_transmitter_update_view(app);
            }
        }
    }

    return consumed;
}

void scene_subghz_transmitter_on_exit(void* context) {
    App* app = context;

    SubGhzTxRx* txrx = app->subghz_txrx;
    app->subghz_txrx = NULL;

    if(txrx) {
        subghz_txrx_stop(txrx);
        subghz_txrx_free(txrx);
    }
    notification_message(app->notifications, &sequence_blink_stop);

    SubGhzTransmitterSceneState* state = app->subghz_scene_state;
    if(state) {
        furi_string_free(state->file_path);
        free(state);
        app->subghz_scene_state = NULL;
    }

    subghz_transmitter_view_reset(app->subghz_transmitter);
    app->restore_selected_item = true;
}

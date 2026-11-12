#include <furi.h>

#include <gui/view_dispatcher.h>
#include <gui/scene_manager.h>
#include <gui/modules/button_menu.h>
#include <notification/notification_messages.h>
#include <flipper_format/flipper_format.h>

#include "julve_aio.h"
#include "scenes.h"
#include "scene_ir_remote.h"
#include "../actions/action_ir_utils.h"

typedef enum {
    IrRemoteEventStartTx = 100,
    IrRemoteEventStopTx,
    IrRemoteEventClose,
} IrRemoteEvent;

typedef struct {
    IrNameArray_t names;
    FuriThread* tx_thread;
    InfraredSignal* tx_signal;
    volatile bool tx_stop;
    int32_t pressed_index;
    bool is_transmitting;
} IrRemoteState;

static int32_t ir_remote_tx_worker(void* context) {
    IrRemoteState* state = context;
    InfraredSignal* signal = state->tx_signal;
    bool first = true;

    do {
        if(signal->is_raw) {
            infrared_send_raw_ext(
                signal->payload.raw.timings,
                signal->payload.raw.timings_size,
                true,
                signal->payload.raw.frequency,
                signal->payload.raw.duty_cycle);
            if(!state->tx_stop) {
                furi_delay_ms(50);
            }
        } else {
            InfraredMessage msg = signal->payload.message;
            msg.repeat = !first;
            infrared_send(&msg, 1);
            first = false;
        }
    } while(!state->tx_stop);

    return 0;
}

static void scene_ir_remote_stop_tx(App* app) {
    IrRemoteState* state = app->ir_remote_state;
    if(!state || !state->is_transmitting) return;

    state->tx_stop = true;
    if(state->tx_thread) {
        furi_thread_join(state->tx_thread);
        furi_thread_free(state->tx_thread);
        state->tx_thread = NULL;
    }

    if(app->settings.ir_use_ext_module) {
        action_ir_power_otg(false);
    }
    notification_message(app->notifications, &sequence_blink_stop);

    if(state->tx_signal) {
        infrared_utils_signal_free(state->tx_signal);
        state->tx_signal = NULL;
    }
    state->is_transmitting = false;
}

static void scene_ir_remote_start_tx(App* app, int32_t index) {
    IrRemoteState* state = app->ir_remote_state;
    if(!state || index < 0) return;

    if(state->is_transmitting) {
        scene_ir_remote_stop_tx(app);
    }

    InfraredSignal* signal = infrared_utils_signal_alloc();
    FlipperFormat* fff_data_file = flipper_format_file_alloc(app->storage);
    FuriString* name = furi_string_alloc();
    bool read_ok = false;

    if(flipper_format_file_open_existing(fff_data_file, furi_string_get_cstr(app->temp_str))) {
        read_ok =
            infrared_utils_read_signal_at_index(fff_data_file, (uint32_t)index, signal, name);
    }

    flipper_format_file_close(fff_data_file);
    flipper_format_free(fff_data_file);
    furi_string_free(name);

    if(!read_ok) {
        FURI_LOG_E(TAG, "IR Remote: Failed to read signal %ld", index);
        infrared_utils_signal_free(signal);
        notification_message(app->notifications, &sequence_error);
        return;
    }

    state->tx_signal = signal;
    state->tx_stop = false;
    state->is_transmitting = true;

    if(app->settings.ir_use_ext_module) {
        action_ir_power_otg(true);
    }
    notification_message(app->notifications, &sequence_blink_start_blue);

    state->tx_thread = furi_thread_alloc_ex("JulveAioIrTx", 2048, ir_remote_tx_worker, state);
    furi_thread_start(state->tx_thread);
}

static void scene_ir_remote_button_callback(void* context, int32_t index, InputType type) {
    App* app = context;
    IrRemoteState* state = app->ir_remote_state;
    if(!state) return;

    if(type == InputTypePress) {
        state->pressed_index = index;
        view_dispatcher_send_custom_event(app->view_dispatcher, IrRemoteEventStartTx);
    } else if(type == InputTypeRelease) {
        view_dispatcher_send_custom_event(app->view_dispatcher, IrRemoteEventStopTx);
    }
}

void scene_ir_remote_on_enter(void* context) {
    App* app = context;

    IrRemoteState* state = malloc(sizeof(IrRemoteState));
    memset(state, 0, sizeof(IrRemoteState));
    IrNameArray_init(state->names);
    app->ir_remote_state = state;

    FlipperFormat* fff_data_file = flipper_format_file_alloc(app->storage);
    bool loaded = false;
    if(flipper_format_file_open_existing(fff_data_file, furi_string_get_cstr(app->temp_str))) {
        loaded = infrared_utils_read_names(fff_data_file, state->names);
    }
    flipper_format_file_close(fff_data_file);
    flipper_format_free(fff_data_file);

    ButtonMenu* menu = app->button_menu;
    button_menu_reset(menu);
    button_menu_set_header(menu, furi_string_get_cstr(app->active_item_name));

    if(!loaded || IrNameArray_size(state->names) == 0) {
        FURI_LOG_E(TAG, "IR Remote: Empty or invalid file %s", furi_string_get_cstr(app->temp_str));
        notification_message(app->notifications, &sequence_error);
        dialog_message_show_storage_error(app->dialog, "Invalid or empty IR file!");
        view_dispatcher_send_custom_event(app->view_dispatcher, IrRemoteEventClose);
    } else {
        size_t count = IrNameArray_size(state->names);
        for(size_t i = 0; i < count; i++) {
            FuriString* btn_name = *IrNameArray_get(state->names, i);
            button_menu_add_item(
                menu,
                furi_string_get_cstr(btn_name),
                (int32_t)i,
                scene_ir_remote_button_callback,
                ButtonMenuItemTypeCommon,
                app);
        }
    }

    view_dispatcher_switch_to_view(app->view_dispatcher, QView_ButtonMenu);
}

bool scene_ir_remote_on_event(void* context, SceneManagerEvent event) {
    App* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        IrRemoteState* state = app->ir_remote_state;
        switch(event.event) {
        case IrRemoteEventStartTx:
            consumed = true;
            if(state) {
                scene_ir_remote_start_tx(app, state->pressed_index);
            }
            break;
        case IrRemoteEventStopTx:
            consumed = true;
            scene_ir_remote_stop_tx(app);
            break;
        case IrRemoteEventClose:
            consumed = true;
            scene_manager_previous_scene(app->scene_manager);
            break;
        default:
            break;
        }
    }

    return consumed;
}

void scene_ir_remote_on_exit(void* context) {
    App* app = context;
    scene_ir_remote_stop_tx(app);

    button_menu_reset(app->button_menu);

    IrRemoteState* state = app->ir_remote_state;
    if(state) {
        IrNameArray_clear(state->names);
        free(state);
        app->ir_remote_state = NULL;
    }

    app->restore_selected_item = true;
}


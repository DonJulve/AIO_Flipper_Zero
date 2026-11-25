#include <furi.h>
#include <gui/view_dispatcher.h>
#include <gui/scene_manager.h>
#include <notification/notification_messages.h>
#include <lib/lfrfid/lfrfid_worker.h>
#include <toolbox/protocols/protocol_dict.h>
#include <lfrfid/protocols/lfrfid_protocols.h>
#include <flipper_format/flipper_format.h>

#include "julve_aio.h"
#include "scenes.h"
#include "scene_rfid_emulate.h"
#include "../views/rfid_emulate.h"

#define RFID_FILE_TYPE    "Flipper RFID key"
#define RFID_FILE_VERSION 1

typedef enum {
    RfidEmulateSceneEventStop = 100,
} RfidEmulateSceneEvent;

typedef struct {
    ProtocolDict* dict;
    LFRFIDWorker* worker;
    ProtocolId protocol;
    FuriString* file_path;
    bool is_emulating;
} RfidEmulateSceneState;

static void scene_rfid_emulate_view_callback(RfidEmulateViewEvent event, void* context) {
    App* app = context;
    if(event == RfidEmulateViewEventStop) {
        view_dispatcher_send_custom_event(app->view_dispatcher, RfidEmulateSceneEventStop);
    }
}

void scene_rfid_emulate_on_enter(void* context) {
    App* app = context;
    FURI_LOG_I(TAG, "RFID Emulate: Loading %s", furi_string_get_cstr(app->temp_str));

    RfidEmulateSceneState* state = malloc(sizeof(RfidEmulateSceneState));
    state->dict = protocol_dict_alloc(lfrfid_protocols, LFRFIDProtocolMax);
    state->worker = NULL;
    state->protocol = PROTOCOL_NO;
    state->file_path = furi_string_alloc_set(app->temp_str);
    state->is_emulating = false;
    app->rfid_scene_state = state;

    FlipperFormat* fff_file = flipper_format_file_alloc(app->storage);
    FuriString* temp_str = furi_string_alloc();
    uint32_t temp_data32 = 0;
    bool successful_read = false;
    size_t data_size = protocol_dict_get_max_data_size(state->dict);
    uint8_t* data = malloc(data_size);

    do {
        if(!flipper_format_file_open_existing(fff_file, furi_string_get_cstr(state->file_path))) {
            FURI_LOG_E(TAG, "RFID Emulate: Error opening %s", furi_string_get_cstr(state->file_path));
            break;
        }
        if(!flipper_format_read_header(fff_file, temp_str, &temp_data32)) {
            FURI_LOG_E(TAG, "RFID Emulate: Missing header");
            break;
        }
        if(strcmp(furi_string_get_cstr(temp_str), RFID_FILE_TYPE) || temp_data32 != RFID_FILE_VERSION) {
            FURI_LOG_E(TAG, "RFID Emulate: Type or version mismatch");
            break;
        }
        if(!flipper_format_read_string(fff_file, "Key type", temp_str)) {
            FURI_LOG_E(TAG, "RFID Emulate: Error reading protocol");
            break;
        }
        state->protocol = protocol_dict_get_protocol_by_name(state->dict, furi_string_get_cstr(temp_str));
        if(state->protocol == PROTOCOL_NO) {
            FURI_LOG_E(TAG, "RFID Emulate: Unknown protocol: %s", furi_string_get_cstr(temp_str));
            break;
        }
        size_t required_size = protocol_dict_get_data_size(state->dict, state->protocol);
        if(!flipper_format_read_hex(fff_file, "Data", data, required_size)) {
            FURI_LOG_E(TAG, "RFID Emulate: Error reading data");
            break;
        }
        protocol_dict_set_data(state->dict, state->protocol, data, data_size);
        successful_read = true;
    } while(false);

    flipper_format_file_close(fff_file);
    flipper_format_free(fff_file);

    if(!successful_read) {
        free(data);
        furi_string_free(temp_str);
        notification_message(app->notifications, &sequence_error);
        scene_manager_previous_scene(app->scene_manager);
        return;
    }

    const char* proto_name = protocol_dict_get_name(state->dict, state->protocol);

    FuriString* data_str = furi_string_alloc();
    size_t required_size = protocol_dict_get_data_size(state->dict, state->protocol);
    if(required_size > 0) {
        furi_string_set_str(data_str, "Data: ");
        for(size_t i = 0; i < required_size; i++) {
            furi_string_cat_printf(data_str, "%02X%s", data[i], (i + 1 < required_size) ? " " : "");
        }
    }

    free(data);
    furi_string_free(temp_str);

    rfid_emulate_view_set_data(
        app->rfid_emulate_view,
        furi_string_get_cstr(app->active_item_name),
        proto_name,
        furi_string_get_cstr(data_str));
    furi_string_free(data_str);

    rfid_emulate_view_set_callback(app->rfid_emulate_view, scene_rfid_emulate_view_callback, app);

    state->worker = lfrfid_worker_alloc(state->dict);
    lfrfid_worker_start_thread(state->worker);
    lfrfid_worker_emulate_start(state->worker, state->protocol);
    state->is_emulating = true;

    notification_message(app->notifications, &sequence_blink_start_blue);
    view_dispatcher_switch_to_view(app->view_dispatcher, QView_RfidEmulate);
}

bool scene_rfid_emulate_on_event(void* context, SceneManagerEvent event) {
    App* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == RfidEmulateSceneEventStop) {
            scene_manager_previous_scene(app->scene_manager);
            consumed = true;
        }
    } else if(event.type == SceneManagerEventTypeBack) {
        consumed = false; // Let Scene Manager navigate back
    }

    return consumed;
}

void scene_rfid_emulate_on_exit(void* context) {
    App* app = context;
    RfidEmulateSceneState* state = app->rfid_scene_state;
    if(!state) return;

    if(state->is_emulating && state->worker) {
        FURI_LOG_I(TAG, "RFID Emulate: Stopping worker...");
        lfrfid_worker_stop(state->worker);
        lfrfid_worker_stop_thread(state->worker);
        lfrfid_worker_free(state->worker);
        state->worker = NULL;
        state->is_emulating = false;
    }

    notification_message(app->notifications, &sequence_blink_stop);

    if(state->dict) {
        protocol_dict_free(state->dict);
        state->dict = NULL;
    }
    if(state->file_path) {
        furi_string_free(state->file_path);
    }
    rfid_emulate_view_reset(app->rfid_emulate_view);

    free(state);
    app->rfid_scene_state = NULL;
    app->restore_selected_item = true;
}

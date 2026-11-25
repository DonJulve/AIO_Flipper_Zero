#include <furi.h>
#include <gui/view_dispatcher.h>
#include <gui/scene_manager.h>
#include <notification/notification_messages.h>
#include <nfc/nfc.h>
#include <nfc/nfc_device.h>
#include <nfc/nfc_listener.h>

#include "julve_aio.h"
#include "scenes.h"
#include "scene_nfc_emulate.h"
#include "../views/nfc_emulate.h"

typedef enum {
    NfcEmulateSceneEventStop = 100,
} NfcEmulateSceneEvent;

typedef struct {
    Nfc* nfc;
    NfcDevice* device;
    NfcListener* listener;
    FuriString* file_path;
    bool is_emulating;
} NfcEmulateSceneState;

static void scene_nfc_emulate_view_callback(NfcEmulateViewEvent event, void* context) {
    App* app = context;
    if(event == NfcEmulateViewEventStop) {
        view_dispatcher_send_custom_event(app->view_dispatcher, NfcEmulateSceneEventStop);
    }
}

void scene_nfc_emulate_on_enter(void* context) {
    App* app = context;
    FURI_LOG_I(TAG, "NFC Emulate: Loading %s", furi_string_get_cstr(app->temp_str));

    NfcEmulateSceneState* state = malloc(sizeof(NfcEmulateSceneState));
    state->nfc = nfc_alloc();
    state->device = nfc_device_alloc();
    state->listener = NULL;
    state->file_path = furi_string_alloc_set(app->temp_str);
    state->is_emulating = false;
    app->nfc_scene_state = state;

    if(!nfc_device_load(state->device, furi_string_get_cstr(state->file_path))) {
        FURI_LOG_E(TAG, "NFC Emulate: Failed to load %s", furi_string_get_cstr(state->file_path));
        notification_message(app->notifications, &sequence_error);
        scene_manager_previous_scene(app->scene_manager);
        return;
    }

    NfcProtocol protocol = nfc_device_get_protocol(state->device);
    const char* dev_name = nfc_device_get_name(state->device, NfcDeviceNameTypeFull);
    const char* proto_name =
        (dev_name && dev_name[0]) ? dev_name : nfc_device_get_protocol_name(protocol);

    FuriString* uid_str = furi_string_alloc();
    size_t uid_len = 0;
    const uint8_t* uid = nfc_device_get_uid(state->device, &uid_len);
    if(uid && uid_len > 0) {
        furi_string_set_str(uid_str, "UID: ");
        for(size_t i = 0; i < uid_len; i++) {
            furi_string_cat_printf(uid_str, "%02X%s", uid[i], (i + 1 < uid_len) ? " " : "");
        }
    }

    nfc_emulate_view_set_data(
        app->nfc_emulate_view,
        furi_string_get_cstr(app->active_item_name),
        proto_name,
        furi_string_get_cstr(uid_str));
    furi_string_free(uid_str);

    nfc_emulate_view_set_callback(app->nfc_emulate_view, scene_nfc_emulate_view_callback, app);

    state->listener =
        nfc_listener_alloc(state->nfc, protocol, nfc_device_get_data(state->device, protocol));
    FURI_LOG_I(TAG, "NFC Emulate: Starting listener...");
    nfc_listener_start(state->listener, NULL, NULL);
    state->is_emulating = true;

    notification_message(app->notifications, &sequence_blink_start_blue);
    view_dispatcher_switch_to_view(app->view_dispatcher, QView_NfcEmulate);
}

bool scene_nfc_emulate_on_event(void* context, SceneManagerEvent event) {
    App* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == NfcEmulateSceneEventStop) {
            scene_manager_previous_scene(app->scene_manager);
            consumed = true;
        }
    } else if(event.type == SceneManagerEventTypeBack) {
        consumed = false; // Let Scene Manager navigate back
    }

    return consumed;
}

void scene_nfc_emulate_on_exit(void* context) {
    App* app = context;
    NfcEmulateSceneState* state = app->nfc_scene_state;
    if(!state) return;

    if(state->is_emulating && state->listener) {
        FURI_LOG_I(TAG, "NFC Emulate: Stopping listener...");
        nfc_listener_stop(state->listener);
        nfc_listener_free(state->listener);
        state->listener = NULL;
        state->is_emulating = false;
    }

    notification_message(app->notifications, &sequence_blink_stop);

    if(state->device) {
        nfc_device_free(state->device);
        state->device = NULL;
    }
    if(state->nfc) {
        nfc_free(state->nfc);
        state->nfc = NULL;
    }
    if(state->file_path) {
        furi_string_free(state->file_path);
    }
    nfc_emulate_view_reset(app->nfc_emulate_view);

    free(state);
    app->nfc_scene_state = NULL;
    app->restore_selected_item = true;
}

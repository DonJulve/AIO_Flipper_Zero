#include <furi.h>
#include <gui/view_dispatcher.h>
#include <gui/scene_manager.h>
#include <notification/notification_messages.h>
#include <ibutton/ibutton_key.h>
#include <ibutton/ibutton_worker.h>
#include <ibutton/ibutton_protocols.h>

#include "julve_aio.h"
#include "scenes.h"
#include "scene_ibutton_emulate.h"
#include "../views/ibutton_emulate.h"

typedef enum {
    iButtonEmulateSceneEventStop = 100,
} iButtonEmulateSceneEvent;

typedef struct {
    iButtonProtocols* protocols;
    iButtonKey* key;
    iButtonWorker* worker;
    FuriString* file_path;
    bool is_emulating;
} iButtonEmulateSceneState;

static void scene_ibutton_emulate_view_callback(iButtonEmulateViewEvent event, void* context) {
    App* app = context;
    if(event == iButtonEmulateViewEventStop) {
        view_dispatcher_send_custom_event(app->view_dispatcher, iButtonEmulateSceneEventStop);
    }
}

void scene_ibutton_emulate_on_enter(void* context) {
    App* app = context;
    FURI_LOG_I(TAG, "iButton Emulate: Loading %s", furi_string_get_cstr(app->temp_str));

    iButtonEmulateSceneState* state = malloc(sizeof(iButtonEmulateSceneState));
    state->protocols = ibutton_protocols_alloc();
    state->key = ibutton_key_alloc(ibutton_protocols_get_max_data_size(state->protocols));
    state->worker = NULL;
    state->file_path = furi_string_alloc_set(app->temp_str);
    state->is_emulating = false;
    app->ibutton_scene_state = state;

    if(!ibutton_protocols_load(state->protocols, state->key, furi_string_get_cstr(state->file_path))) {
        FURI_LOG_E(TAG, "iButton Emulate: Failed to load %s", furi_string_get_cstr(state->file_path));
        notification_message(app->notifications, &sequence_error);
        scene_manager_previous_scene(app->scene_manager);
        return;
    }

    iButtonProtocolId proto_id = ibutton_key_get_protocol_id(state->key);
    const char* proto_name = ibutton_protocols_get_name(state->protocols, proto_id);

    FuriString* data_str = furi_string_alloc();
    ibutton_protocols_render_uid(state->protocols, state->key, data_str);

    ibutton_emulate_view_set_data(
        app->ibutton_emulate_view,
        furi_string_get_cstr(app->active_item_name),
        proto_name,
        furi_string_get_cstr(data_str));
    furi_string_free(data_str);

    ibutton_emulate_view_set_callback(app->ibutton_emulate_view, scene_ibutton_emulate_view_callback, app);

    state->worker = ibutton_worker_alloc(state->protocols);
    ibutton_worker_start_thread(state->worker);
    ibutton_worker_emulate_start(state->worker, state->key);
    state->is_emulating = true;

    notification_message(app->notifications, &sequence_blink_start_blue);
    view_dispatcher_switch_to_view(app->view_dispatcher, QView_iButtonEmulate);
}

bool scene_ibutton_emulate_on_event(void* context, SceneManagerEvent event) {
    App* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == iButtonEmulateSceneEventStop) {
            scene_manager_previous_scene(app->scene_manager);
            consumed = true;
        }
    } else if(event.type == SceneManagerEventTypeBack) {
        consumed = false;
    }

    return consumed;
}

void scene_ibutton_emulate_on_exit(void* context) {
    App* app = context;
    iButtonEmulateSceneState* state = app->ibutton_scene_state;
    if(!state) return;

    if(state->is_emulating && state->worker) {
        FURI_LOG_I(TAG, "iButton Emulate: Stopping worker...");
        ibutton_worker_stop(state->worker);
        ibutton_worker_stop_thread(state->worker);
        ibutton_worker_free(state->worker);
        state->worker = NULL;
        state->is_emulating = false;
    }

    notification_message(app->notifications, &sequence_blink_stop);

    if(state->key) {
        ibutton_key_free(state->key);
        state->key = NULL;
    }
    if(state->protocols) {
        ibutton_protocols_free(state->protocols);
        state->protocols = NULL;
    }
    if(state->file_path) {
        furi_string_free(state->file_path);
    }
    ibutton_emulate_view_reset(app->ibutton_emulate_view);

    free(state);
    app->ibutton_scene_state = NULL;
    app->restore_selected_item = true;
}

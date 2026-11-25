#include <furi.h>
#include <gui/view_dispatcher.h>
#include <gui/scene_manager.h>
#include <notification/notification_messages.h>

#include "julve_aio.h"
#include "scenes.h"
#include "scene_picopass_emulate.h"
#include "../views/picopass_emulate.h"
#include "../actions/action_i.h"

typedef enum {
    PicopassEmulateSceneEventStop = 100,
} PicopassEmulateSceneEvent;

typedef struct {
    PicopassEmulator* emu;
    FuriString* file_path;
    bool is_emulating;
} PicopassEmulateSceneState;

static void scene_picopass_emulate_view_callback(PicopassEmulateViewEvent event, void* context) {
    App* app = context;
    if(event == PicopassEmulateViewEventStop) {
        view_dispatcher_send_custom_event(app->view_dispatcher, PicopassEmulateSceneEventStop);
    }
}

void scene_picopass_emulate_on_enter(void* context) {
    App* app = context;
    FURI_LOG_I(TAG, "Picopass Emulate: Loading %s", furi_string_get_cstr(app->temp_str));

    PicopassEmulateSceneState* state = malloc(sizeof(PicopassEmulateSceneState));
    state->emu = NULL;
    state->file_path = furi_string_alloc_set(app->temp_str);
    state->is_emulating = false;
    app->picopass_scene_state = state;

    FuriString* error = furi_string_alloc();
    state->emu = action_picopass_emulator_start(app, furi_string_get_cstr(state->file_path), error);
    if(!state->emu) {
        FURI_LOG_E(TAG, "%s", furi_string_get_cstr(error));
        furi_string_free(error);
        notification_message(app->notifications, &sequence_error);
        scene_manager_previous_scene(app->scene_manager);
        return;
    }
    furi_string_free(error);

    const uint8_t* csn = action_picopass_emulator_get_csn(state->emu);
    FuriString* csn_str = furi_string_alloc();
    if(csn) {
        furi_string_set_str(csn_str, "CSN: ");
        for(size_t i = 0; i < 8; i++) {
            furi_string_cat_printf(csn_str, "%02X%s", csn[i], (i + 1 < 8) ? " " : "");
        }
    }

    picopass_emulate_view_set_data(
        app->picopass_emulate_view,
        furi_string_get_cstr(app->active_item_name),
        "Picopass / iClass",
        furi_string_get_cstr(csn_str));
    furi_string_free(csn_str);

    picopass_emulate_view_set_callback(app->picopass_emulate_view, scene_picopass_emulate_view_callback, app);
    state->is_emulating = true;

    notification_message(app->notifications, &sequence_blink_start_blue);
    view_dispatcher_switch_to_view(app->view_dispatcher, QView_PicopassEmulate);
}

bool scene_picopass_emulate_on_event(void* context, SceneManagerEvent event) {
    App* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == PicopassEmulateSceneEventStop) {
            scene_manager_previous_scene(app->scene_manager);
            consumed = true;
        }
    } else if(event.type == SceneManagerEventTypeBack) {
        consumed = false;
    }

    return consumed;
}

void scene_picopass_emulate_on_exit(void* context) {
    App* app = context;
    PicopassEmulateSceneState* state = app->picopass_scene_state;
    if(!state) return;

    if(state->is_emulating && state->emu) {
        FURI_LOG_I(TAG, "Picopass Emulate: Stopping emulator...");
        action_picopass_emulator_stop(state->emu);
        state->emu = NULL;
        state->is_emulating = false;
    }

    notification_message(app->notifications, &sequence_blink_stop);

    if(state->file_path) {
        furi_string_free(state->file_path);
    }
    picopass_emulate_view_reset(app->picopass_emulate_view);

    free(state);
    app->picopass_scene_state = NULL;
    app->restore_selected_item = true;
}

#include <furi.h>

#include <gui/view_dispatcher.h>
#include <gui/scene_manager.h>
#include <flipper_application/flipper_application.h>

#include "julve_aio.h"
#include "scenes.h"
#include "scene_about.h"
#include "julve_aio_icons.h"

enum {
    SceneActionAboutEvent,
};

void scene_about_callback(void* context) {
    App* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, SceneActionAboutEvent);
}

void scene_about_on_enter(void* context) {
    App* app = context;

    Popup* popup = app->popup;
    popup_set_header(popup, JULVE_AIO_NAME, 72, 1, AlignCenter, AlignTop);
    popup_set_text(popup, JULVE_AIO_ABOUT, 0, 15, AlignLeft, AlignTop);
    popup_set_icon(popup, 24, 0, &I_julve_aio);
    view_dispatcher_switch_to_view(app->view_dispatcher, QView_Popup);
}

bool scene_about_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    return false;
}

void scene_about_on_exit(void* context) {
    App* app = context;
    popup_reset(app->popup);
}
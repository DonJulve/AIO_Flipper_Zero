#include "ibutton_emulate.h"

#include <gui/canvas.h>
#include <gui/elements.h>
#include <input/input.h>
#include <julve_aio_icons.h>

struct iButtonEmulateView {
    View* view;
    iButtonEmulateViewCallback callback;
    void* callback_context;
};

typedef struct {
    FuriString* key_name;
    FuriString* protocol_name;
    FuriString* data_str;
} iButtonEmulateViewModel;

static void ibutton_emulate_view_draw_callback(Canvas* canvas, void* context) {
    furi_assert(canvas);
    iButtonEmulateViewModel* model = context;

    canvas_clear(canvas);
    canvas_set_color(canvas, ColorBlack);

    // Header: iButton icon + key name
    canvas_draw_icon(canvas, 3, 2, &I_iButton_10px);
    canvas_set_font(canvas, FontSecondary);
    if(model->key_name && !furi_string_empty(model->key_name)) {
        canvas_draw_str_aligned(
            canvas, 16, 10, AlignLeft, AlignBottom, furi_string_get_cstr(model->key_name));
    } else {
        canvas_draw_str_aligned(canvas, 16, 10, AlignLeft, AlignBottom, "iButton");
    }
    canvas_draw_line(canvas, 0, 13, 127, 13);

    // Main status text: "Emulating..."
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str_aligned(canvas, 64, 27, AlignCenter, AlignBottom, "Emulating...");

    // Protocol / Device Type (e.g. Dallas, Cyfral, Metakom)
    canvas_set_font(canvas, FontSecondary);
    if(model->protocol_name && !furi_string_empty(model->protocol_name)) {
        canvas_draw_str_aligned(
            canvas, 64, 39, AlignCenter, AlignBottom, furi_string_get_cstr(model->protocol_name));
    }

    // Data / Key ID
    if(model->data_str && !furi_string_empty(model->data_str)) {
        canvas_draw_str_aligned(
            canvas, 64, 50, AlignCenter, AlignBottom, furi_string_get_cstr(model->data_str));
    }

    // Exit button in bottom-left corner
    elements_button_left(canvas, "Exit");
}

static bool ibutton_emulate_view_input_callback(InputEvent* event, void* context) {
    iButtonEmulateView* instance = context;
    furi_assert(instance);

    if(event->type == InputTypeShort) {
        if(event->key == InputKeyBack || event->key == InputKeyLeft || event->key == InputKeyOk) {
            if(instance->callback) {
                instance->callback(iButtonEmulateViewEventStop, instance->callback_context);
                return true;
            }
        }
    }
    return false;
}

iButtonEmulateView* ibutton_emulate_view_alloc(void) {
    iButtonEmulateView* instance = malloc(sizeof(iButtonEmulateView));
    instance->view = view_alloc();
    view_allocate_model(instance->view, ViewModelTypeLocking, sizeof(iButtonEmulateViewModel));
    view_set_context(instance->view, instance);
    view_set_draw_callback(instance->view, ibutton_emulate_view_draw_callback);
    view_set_input_callback(instance->view, ibutton_emulate_view_input_callback);

    with_view_model(
        instance->view,
        iButtonEmulateViewModel * model,
        {
            model->key_name = furi_string_alloc();
            model->protocol_name = furi_string_alloc();
            model->data_str = furi_string_alloc();
        },
        true);

    return instance;
}

void ibutton_emulate_view_free(iButtonEmulateView* instance) {
    furi_assert(instance);

    with_view_model(
        instance->view,
        iButtonEmulateViewModel * model,
        {
            furi_string_free(model->key_name);
            furi_string_free(model->protocol_name);
            furi_string_free(model->data_str);
        },
        false);

    view_free(instance->view);
    free(instance);
}

View* ibutton_emulate_view_get_view(iButtonEmulateView* instance) {
    furi_assert(instance);
    return instance->view;
}

void ibutton_emulate_view_reset(iButtonEmulateView* instance) {
    furi_assert(instance);
    with_view_model(
        instance->view,
        iButtonEmulateViewModel * model,
        {
            furi_string_reset(model->key_name);
            furi_string_reset(model->protocol_name);
            furi_string_reset(model->data_str);
        },
        true);
}

void ibutton_emulate_view_set_callback(
    iButtonEmulateView* instance,
    iButtonEmulateViewCallback callback,
    void* context) {
    furi_assert(instance);
    instance->callback = callback;
    instance->callback_context = context;
}

void ibutton_emulate_view_set_data(
    iButtonEmulateView* instance,
    const char* key_name,
    const char* protocol_name,
    const char* data_str) {
    furi_assert(instance);
    with_view_model(
        instance->view,
        iButtonEmulateViewModel * model,
        {
            if(key_name) {
                furi_string_set_str(model->key_name, key_name);
            } else {
                furi_string_reset(model->key_name);
            }
            if(protocol_name) {
                furi_string_set_str(model->protocol_name, protocol_name);
            } else {
                furi_string_reset(model->protocol_name);
            }
            if(data_str) {
                furi_string_set_str(model->data_str, data_str);
            } else {
                furi_string_reset(model->data_str);
            }
        },
        true);
}

#include "rfid_emulate.h"

#include <gui/canvas.h>
#include <gui/elements.h>
#include <input/input.h>
#include <julve_aio_icons.h>

struct RfidEmulateView {
    View* view;
    RfidEmulateViewCallback callback;
    void* callback_context;
};

typedef struct {
    FuriString* card_name;
    FuriString* protocol_name;
    FuriString* data_str;
} RfidEmulateViewModel;

static void rfid_emulate_view_draw_callback(Canvas* canvas, void* context) {
    furi_assert(canvas);
    RfidEmulateViewModel* model = context;

    canvas_clear(canvas);
    canvas_set_color(canvas, ColorBlack);

    // Header: RFID icon + card name
    canvas_draw_icon(canvas, 3, 2, &I_RFID_10px);
    canvas_set_font(canvas, FontSecondary);
    if(model->card_name && !furi_string_empty(model->card_name)) {
        canvas_draw_str_aligned(
            canvas, 16, 10, AlignLeft, AlignBottom, furi_string_get_cstr(model->card_name));
    } else {
        canvas_draw_str_aligned(canvas, 16, 10, AlignLeft, AlignBottom, "RFID");
    }
    canvas_draw_line(canvas, 0, 13, 127, 13);

    // Main status text: "Emulating..."
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str_aligned(canvas, 64, 27, AlignCenter, AlignBottom, "Emulating...");

    // Protocol / Device Type (e.g. EM4100, HIDProx)
    canvas_set_font(canvas, FontSecondary);
    if(model->protocol_name && !furi_string_empty(model->protocol_name)) {
        canvas_draw_str_aligned(
            canvas, 64, 39, AlignCenter, AlignBottom, furi_string_get_cstr(model->protocol_name));
    }

    // Data / ID
    if(model->data_str && !furi_string_empty(model->data_str)) {
        canvas_draw_str_aligned(
            canvas, 64, 50, AlignCenter, AlignBottom, furi_string_get_cstr(model->data_str));
    }

    // Exit button in bottom-left corner
    elements_button_left(canvas, "Exit");
}

static bool rfid_emulate_view_input_callback(InputEvent* event, void* context) {
    RfidEmulateView* instance = context;
    furi_assert(instance);

    if(event->type == InputTypeShort) {
        if(event->key == InputKeyBack || event->key == InputKeyLeft || event->key == InputKeyOk) {
            if(instance->callback) {
                instance->callback(RfidEmulateViewEventStop, instance->callback_context);
                return true;
            }
        }
    }
    return false;
}

RfidEmulateView* rfid_emulate_view_alloc(void) {
    RfidEmulateView* instance = malloc(sizeof(RfidEmulateView));
    instance->view = view_alloc();
    view_allocate_model(instance->view, ViewModelTypeLocking, sizeof(RfidEmulateViewModel));
    view_set_context(instance->view, instance);
    view_set_draw_callback(instance->view, rfid_emulate_view_draw_callback);
    view_set_input_callback(instance->view, rfid_emulate_view_input_callback);

    with_view_model(
        instance->view,
        RfidEmulateViewModel * model,
        {
            model->card_name = furi_string_alloc();
            model->protocol_name = furi_string_alloc();
            model->data_str = furi_string_alloc();
        },
        true);

    return instance;
}

void rfid_emulate_view_free(RfidEmulateView* instance) {
    furi_assert(instance);

    with_view_model(
        instance->view,
        RfidEmulateViewModel * model,
        {
            furi_string_free(model->card_name);
            furi_string_free(model->protocol_name);
            furi_string_free(model->data_str);
        },
        false);

    view_free(instance->view);
    free(instance);
}

View* rfid_emulate_view_get_view(RfidEmulateView* instance) {
    furi_assert(instance);
    return instance->view;
}

void rfid_emulate_view_reset(RfidEmulateView* instance) {
    furi_assert(instance);
    with_view_model(
        instance->view,
        RfidEmulateViewModel * model,
        {
            furi_string_reset(model->card_name);
            furi_string_reset(model->protocol_name);
            furi_string_reset(model->data_str);
        },
        true);
}

void rfid_emulate_view_set_callback(
    RfidEmulateView* instance,
    RfidEmulateViewCallback callback,
    void* context) {
    furi_assert(instance);
    instance->callback = callback;
    instance->callback_context = context;
}

void rfid_emulate_view_set_data(
    RfidEmulateView* instance,
    const char* card_name,
    const char* protocol_name,
    const char* data_str) {
    furi_assert(instance);
    with_view_model(
        instance->view,
        RfidEmulateViewModel * model,
        {
            if(card_name) {
                furi_string_set_str(model->card_name, card_name);
            } else {
                furi_string_reset(model->card_name);
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

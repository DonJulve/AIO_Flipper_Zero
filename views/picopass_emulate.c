#include "picopass_emulate.h"

#include <gui/canvas.h>
#include <gui/elements.h>
#include <input/input.h>
#include <julve_aio_icons.h>

struct PicopassEmulateView {
    View* view;
    PicopassEmulateViewCallback callback;
    void* callback_context;
};

typedef struct {
    FuriString* card_name;
    FuriString* protocol_name;
    FuriString* csn_str;
} PicopassEmulateViewModel;

static void picopass_emulate_view_draw_callback(Canvas* canvas, void* context) {
    furi_assert(canvas);
    PicopassEmulateViewModel* model = context;

    canvas_clear(canvas);
    canvas_set_color(canvas, ColorBlack);

    // Header: Picopass icon + card name
    canvas_draw_icon(canvas, 3, 2, &I_Picopass_10px);
    canvas_set_font(canvas, FontSecondary);
    if(model->card_name && !furi_string_empty(model->card_name)) {
        canvas_draw_str_aligned(
            canvas, 16, 10, AlignLeft, AlignBottom, furi_string_get_cstr(model->card_name));
    } else {
        canvas_draw_str_aligned(canvas, 16, 10, AlignLeft, AlignBottom, "Picopass");
    }
    canvas_draw_line(canvas, 0, 13, 127, 13);

    // Main status text: "Emulating..."
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str_aligned(canvas, 64, 27, AlignCenter, AlignBottom, "Emulating...");

    // Protocol / Device Type (e.g. Picopass / iClass)
    canvas_set_font(canvas, FontSecondary);
    if(model->protocol_name && !furi_string_empty(model->protocol_name)) {
        canvas_draw_str_aligned(
            canvas, 64, 39, AlignCenter, AlignBottom, furi_string_get_cstr(model->protocol_name));
    }

    // CSN
    if(model->csn_str && !furi_string_empty(model->csn_str)) {
        canvas_draw_str_aligned(
            canvas, 64, 50, AlignCenter, AlignBottom, furi_string_get_cstr(model->csn_str));
    }

    // Exit button in bottom-left corner
    elements_button_left(canvas, "Exit");
}

static bool picopass_emulate_view_input_callback(InputEvent* event, void* context) {
    PicopassEmulateView* instance = context;
    furi_assert(instance);

    if(event->type == InputTypeShort) {
        if(event->key == InputKeyBack || event->key == InputKeyLeft || event->key == InputKeyOk) {
            if(instance->callback) {
                instance->callback(PicopassEmulateViewEventStop, instance->callback_context);
                return true;
            }
        }
    }
    return false;
}

PicopassEmulateView* picopass_emulate_view_alloc(void) {
    PicopassEmulateView* instance = malloc(sizeof(PicopassEmulateView));
    instance->view = view_alloc();
    view_allocate_model(instance->view, ViewModelTypeLocking, sizeof(PicopassEmulateViewModel));
    view_set_context(instance->view, instance);
    view_set_draw_callback(instance->view, picopass_emulate_view_draw_callback);
    view_set_input_callback(instance->view, picopass_emulate_view_input_callback);

    with_view_model(
        instance->view,
        PicopassEmulateViewModel * model,
        {
            model->card_name = furi_string_alloc();
            model->protocol_name = furi_string_alloc();
            model->csn_str = furi_string_alloc();
        },
        true);

    return instance;
}

void picopass_emulate_view_free(PicopassEmulateView* instance) {
    furi_assert(instance);

    with_view_model(
        instance->view,
        PicopassEmulateViewModel * model,
        {
            furi_string_free(model->card_name);
            furi_string_free(model->protocol_name);
            furi_string_free(model->csn_str);
        },
        false);

    view_free(instance->view);
    free(instance);
}

View* picopass_emulate_view_get_view(PicopassEmulateView* instance) {
    furi_assert(instance);
    return instance->view;
}

void picopass_emulate_view_reset(PicopassEmulateView* instance) {
    furi_assert(instance);
    with_view_model(
        instance->view,
        PicopassEmulateViewModel * model,
        {
            furi_string_reset(model->card_name);
            furi_string_reset(model->protocol_name);
            furi_string_reset(model->csn_str);
        },
        true);
}

void picopass_emulate_view_set_callback(
    PicopassEmulateView* instance,
    PicopassEmulateViewCallback callback,
    void* context) {
    furi_assert(instance);
    instance->callback = callback;
    instance->callback_context = context;
}

void picopass_emulate_view_set_data(
    PicopassEmulateView* instance,
    const char* card_name,
    const char* protocol_name,
    const char* csn_str) {
    furi_assert(instance);
    with_view_model(
        instance->view,
        PicopassEmulateViewModel * model,
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
            if(csn_str) {
                furi_string_set_str(model->csn_str, csn_str);
            } else {
                furi_string_reset(model->csn_str);
            }
        },
        true);
}

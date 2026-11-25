#include "nfc_emulate.h"

#include <gui/canvas.h>
#include <gui/elements.h>
#include <input/input.h>
#include <julve_aio_icons.h>

struct NfcEmulateView {
    View* view;
    NfcEmulateViewCallback callback;
    void* callback_context;
};

typedef struct {
    FuriString* card_name;
    FuriString* protocol_name;
    FuriString* uid_str;
} NfcEmulateViewModel;

static void nfc_emulate_view_draw_callback(Canvas* canvas, void* context) {
    furi_assert(canvas);
    NfcEmulateViewModel* model = context;

    canvas_clear(canvas);
    canvas_set_color(canvas, ColorBlack);

    // Header: NFC icon + card name
    canvas_draw_icon(canvas, 3, 2, &I_NFC_10px);
    canvas_set_font(canvas, FontSecondary);
    if(model->card_name && !furi_string_empty(model->card_name)) {
        canvas_draw_str_aligned(
            canvas, 16, 10, AlignLeft, AlignBottom, furi_string_get_cstr(model->card_name));
    } else {
        canvas_draw_str_aligned(canvas, 16, 10, AlignLeft, AlignBottom, "NFC");
    }
    canvas_draw_line(canvas, 0, 13, 127, 13);

    // Main status text: "Emulating..."
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str_aligned(canvas, 64, 27, AlignCenter, AlignBottom, "Emulating...");

    // Protocol / Device Type
    canvas_set_font(canvas, FontSecondary);
    if(model->protocol_name && !furi_string_empty(model->protocol_name)) {
        canvas_draw_str_aligned(
            canvas, 64, 39, AlignCenter, AlignBottom, furi_string_get_cstr(model->protocol_name));
    }

    // UID
    if(model->uid_str && !furi_string_empty(model->uid_str)) {
        canvas_draw_str_aligned(
            canvas, 64, 50, AlignCenter, AlignBottom, furi_string_get_cstr(model->uid_str));
    }

    // Exit button in bottom-left corner
    elements_button_left(canvas, "Exit");
}

static bool nfc_emulate_view_input_callback(InputEvent* event, void* context) {
    NfcEmulateView* instance = context;
    furi_assert(instance);

    if(event->type == InputTypeShort) {
        if(event->key == InputKeyBack || event->key == InputKeyLeft || event->key == InputKeyOk) {
            if(instance->callback) {
                instance->callback(NfcEmulateViewEventStop, instance->callback_context);
                return true;
            }
        }
    }
    return false;
}

NfcEmulateView* nfc_emulate_view_alloc(void) {
    NfcEmulateView* instance = malloc(sizeof(NfcEmulateView));
    instance->view = view_alloc();
    view_allocate_model(instance->view, ViewModelTypeLocking, sizeof(NfcEmulateViewModel));
    view_set_context(instance->view, instance);
    view_set_draw_callback(instance->view, nfc_emulate_view_draw_callback);
    view_set_input_callback(instance->view, nfc_emulate_view_input_callback);

    with_view_model(
        instance->view,
        NfcEmulateViewModel * model,
        {
            model->card_name = furi_string_alloc();
            model->protocol_name = furi_string_alloc();
            model->uid_str = furi_string_alloc();
        },
        true);

    return instance;
}

void nfc_emulate_view_free(NfcEmulateView* instance) {
    furi_assert(instance);

    with_view_model(
        instance->view,
        NfcEmulateViewModel * model,
        {
            furi_string_free(model->card_name);
            furi_string_free(model->protocol_name);
            furi_string_free(model->uid_str);
        },
        false);

    view_free(instance->view);
    free(instance);
}

View* nfc_emulate_view_get_view(NfcEmulateView* instance) {
    furi_assert(instance);
    return instance->view;
}

void nfc_emulate_view_reset(NfcEmulateView* instance) {
    furi_assert(instance);
    with_view_model(
        instance->view,
        NfcEmulateViewModel * model,
        {
            furi_string_reset(model->card_name);
            furi_string_reset(model->protocol_name);
            furi_string_reset(model->uid_str);
        },
        true);
}

void nfc_emulate_view_set_callback(
    NfcEmulateView* instance,
    NfcEmulateViewCallback callback,
    void* context) {
    furi_assert(instance);
    instance->callback = callback;
    instance->callback_context = context;
}

void nfc_emulate_view_set_data(
    NfcEmulateView* instance,
    const char* card_name,
    const char* protocol_name,
    const char* uid_str) {
    furi_assert(instance);
    with_view_model(
        instance->view,
        NfcEmulateViewModel * model,
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
            if(uid_str) {
                furi_string_set_str(model->uid_str, uid_str);
            } else {
                furi_string_reset(model->uid_str);
            }
        },
        true);
}

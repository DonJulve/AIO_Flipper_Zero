#include "subghz_transmitter.h"

#include <gui/canvas.h>
#include <gui/elements.h>
#include <input/input.h>
#include <m-array.h>

#define SUBGHZ_TX_VISIBLE_LINES 5
#define SUBGHZ_TX_LINE_HEIGHT   9

ARRAY_DEF(SubGhzLineArray, FuriString*, FURI_STRING_OPLIST);

struct SubGhzTransmitterView {
    View* view;
    SubGhzTransmitterViewCallback callback;
    void* callback_context;
};

typedef struct {
    SubGhzLineArray_t lines;
    FuriString* frequency;
    FuriString* modulation;
    size_t scroll_offset;
    bool can_tx;
    bool is_tx;
} SubGhzTransmitterViewModel;

static void subghz_transmitter_view_split_lines(SubGhzLineArray_t lines, const char* text) {
    SubGhzLineArray_reset(lines);
    if(!text || !text[0]) return;

    FuriString* current = furi_string_alloc();
    for(const char* p = text; *p != '\0'; p++) {
        if(*p == '\r') continue;
        if(*p == '\n') {
            SubGhzLineArray_push_back(lines, current);
            furi_string_reset(current);
        } else {
            furi_string_push_back(current, *p);
        }
    }
    if(!furi_string_empty(current)) {
        SubGhzLineArray_push_back(lines, current);
    }
    furi_string_free(current);
}

static void subghz_transmitter_view_draw_callback(Canvas* canvas, void* context) {
    furi_assert(canvas);
    SubGhzTransmitterViewModel* model = context;

    canvas_clear(canvas);
    canvas_set_color(canvas, ColorBlack);
    canvas_set_font(canvas, FontSecondary);

    const size_t total_lines = SubGhzLineArray_size(model->lines);
    for(size_t i = 0; i < SUBGHZ_TX_VISIBLE_LINES; i++) {
        size_t line_idx = model->scroll_offset + i;
        if(line_idx >= total_lines) break;

        FuriString* line = *SubGhzLineArray_get(model->lines, line_idx);
        canvas_draw_str_aligned(
            canvas, 0, 1 + (int32_t)(i * SUBGHZ_TX_LINE_HEIGHT), AlignLeft, AlignTop, furi_string_get_cstr(line));
    }

    if(total_lines > SUBGHZ_TX_VISIBLE_LINES) {
        elements_scrollbar_pos(
            canvas,
            128,
            0,
            SUBGHZ_TX_VISIBLE_LINES * SUBGHZ_TX_LINE_HEIGHT,
            model->scroll_offset,
            total_lines);
    }

    // Bottom info bar: frequency left, Send button center, modulation right
    canvas_draw_str_aligned(
        canvas, 0, 62, AlignLeft, AlignBottom, furi_string_get_cstr(model->frequency));

    if(model->can_tx) {
        elements_button_center(canvas, model->is_tx ? "Sending" : "Send");
    } else {
        canvas_draw_str_aligned(canvas, 64, 62, AlignCenter, AlignBottom, "Read-only");
    }

    canvas_draw_str_aligned(
        canvas, 127, 62, AlignRight, AlignBottom, furi_string_get_cstr(model->modulation));
}

static bool subghz_transmitter_view_input_callback(InputEvent* event, void* context) {
    furi_assert(event);
    SubGhzTransmitterView* instance = context;
    bool consumed = false;

    if(event->key == InputKeyOk) {
        if(event->type == InputTypePress) {
            bool can_tx = false;
            with_view_model(
                instance->view,
                SubGhzTransmitterViewModel * model,
                { can_tx = model->can_tx; },
                false);
            if(can_tx && instance->callback) {
                instance->callback(SubGhzTransmitterViewEventSendStart, instance->callback_context);
                consumed = true;
            }
        } else if(event->type == InputTypeRelease) {
            bool is_tx = false;
            with_view_model(
                instance->view,
                SubGhzTransmitterViewModel * model,
                { is_tx = model->is_tx; },
                false);
            if(is_tx && instance->callback) {
                instance->callback(SubGhzTransmitterViewEventSendStop, instance->callback_context);
                consumed = true;
            }
        }
    } else if(event->type == InputTypeShort || event->type == InputTypeRepeat) {
        if(event->key == InputKeyUp) {
            with_view_model(
                instance->view,
                SubGhzTransmitterViewModel * model,
                {
                    if(model->scroll_offset > 0) {
                        model->scroll_offset--;
                    }
                },
                true);
            consumed = true;
        } else if(event->key == InputKeyDown) {
            with_view_model(
                instance->view,
                SubGhzTransmitterViewModel * model,
                {
                    const size_t total = SubGhzLineArray_size(model->lines);
                    if(total > SUBGHZ_TX_VISIBLE_LINES &&
                       model->scroll_offset < total - SUBGHZ_TX_VISIBLE_LINES) {
                        model->scroll_offset++;
                    }
                },
                true);
            consumed = true;
        }
    }

    return consumed;
}

SubGhzTransmitterView* subghz_transmitter_view_alloc(void) {
    SubGhzTransmitterView* instance = malloc(sizeof(SubGhzTransmitterView));
    instance->view = view_alloc();
    instance->callback = NULL;
    instance->callback_context = NULL;

    view_set_orientation(instance->view, ViewOrientationHorizontal);
    view_set_context(instance->view, instance);
    view_allocate_model(instance->view, ViewModelTypeLocking, sizeof(SubGhzTransmitterViewModel));
    view_set_draw_callback(instance->view, subghz_transmitter_view_draw_callback);
    view_set_input_callback(instance->view, subghz_transmitter_view_input_callback);

    with_view_model(
        instance->view,
        SubGhzTransmitterViewModel * model,
        {
            SubGhzLineArray_init(model->lines);
            model->frequency = furi_string_alloc();
            model->modulation = furi_string_alloc();
            model->scroll_offset = 0;
            model->can_tx = false;
            model->is_tx = false;
        },
        false);

    return instance;
}

void subghz_transmitter_view_free(SubGhzTransmitterView* instance) {
    furi_assert(instance);

    with_view_model(
        instance->view,
        SubGhzTransmitterViewModel * model,
        {
            SubGhzLineArray_clear(model->lines);
            furi_string_free(model->frequency);
            furi_string_free(model->modulation);
        },
        false);

    view_free(instance->view);
    free(instance);
}

View* subghz_transmitter_view_get_view(SubGhzTransmitterView* instance) {
    furi_assert(instance);
    return instance->view;
}

void subghz_transmitter_view_reset(SubGhzTransmitterView* instance) {
    furi_assert(instance);
    instance->callback = NULL;
    instance->callback_context = NULL;

    with_view_model(
        instance->view,
        SubGhzTransmitterViewModel * model,
        {
            SubGhzLineArray_reset(model->lines);
            furi_string_reset(model->frequency);
            furi_string_reset(model->modulation);
            model->scroll_offset = 0;
            model->can_tx = false;
            model->is_tx = false;
        },
        false);
}

void subghz_transmitter_view_set_callback(
    SubGhzTransmitterView* instance,
    SubGhzTransmitterViewCallback callback,
    void* context) {
    furi_assert(instance);
    instance->callback = callback;
    instance->callback_context = context;
}

void subghz_transmitter_view_set_data(
    SubGhzTransmitterView* instance,
    const char* key_info,
    const char* frequency,
    const char* modulation,
    bool can_tx) {
    furi_assert(instance);

    with_view_model(
        instance->view,
        SubGhzTransmitterViewModel * model,
        {
            subghz_transmitter_view_split_lines(model->lines, key_info);
            furi_string_set_str(model->frequency, frequency ? frequency : "");
            furi_string_set_str(model->modulation, modulation ? modulation : "");
            model->can_tx = can_tx;

            const size_t total = SubGhzLineArray_size(model->lines);
            if(total <= SUBGHZ_TX_VISIBLE_LINES) {
                model->scroll_offset = 0;
            } else if(model->scroll_offset > total - SUBGHZ_TX_VISIBLE_LINES) {
                model->scroll_offset = total - SUBGHZ_TX_VISIBLE_LINES;
            }
        },
        true);
}

void subghz_transmitter_view_set_tx(SubGhzTransmitterView* instance, bool is_tx) {
    furi_assert(instance);
    with_view_model(
        instance->view,
        SubGhzTransmitterViewModel * model,
        { model->is_tx = is_tx; },
        true);
}


// Methods for Sub-GHz transmission

#include <flipper_format/flipper_format_i.h>

#include "helpers/subghz_txrx.h"

#include "action_i.h"
#include "julve_aio.h"

void action_subghz_need_save_callback(void* context) {
    FURI_LOG_I(TAG, "Saving udpated subghz signal");
    SubGhzNeedSaveContext* savectx = (SubGhzNeedSaveContext*)context;
    FlipperFormat* ff = subghz_txrx_get_fff_data(savectx->txrx);

    Stream* ff_stream = flipper_format_get_raw_stream(ff);
    flipper_format_delete_key(ff, "Repeat");
    flipper_format_delete_key(ff, "Manufacture");

    do {
        storage_simply_remove(
            savectx->app->storage, furi_string_get_cstr(savectx->file_path));
        stream_seek(ff_stream, 0, StreamOffsetFromStart);
        if(!stream_save_to_file(
               ff_stream,
               savectx->app->storage,
               furi_string_get_cstr(savectx->file_path),
               FSOM_CREATE_ALWAYS)) {
            FURI_LOG_E(
                TAG, "Failed to save subghz file: %s", furi_string_get_cstr(savectx->file_path));
            break;
        }
        if(storage_common_stat(
               savectx->app->storage, furi_string_get_cstr(savectx->file_path), NULL) != FSE_OK) {
            FURI_LOG_E(TAG, "Error verifying new subghz file after re-save");
            break;
        }
    } while(0);
}

static void action_subghz_raw_end_callback(void* context) {
    FURI_LOG_I(TAG, "Stopping TX on RAW");
    furi_assert(context);
    FuriThread* thread = context;

    furi_thread_flags_set(furi_thread_get_id(thread), 0);
}

bool action_subghz_load_file(
    SubGhzTxRx* txrx,
    Storage* storage,
    const char* file_name,
    bool* is_raw,
    FuriString* error) {
    FlipperFormat* fff_data_file = flipper_format_file_alloc(storage);
    Stream* fff_data_stream = flipper_format_get_raw_stream(subghz_txrx_get_fff_data(txrx));
    stream_clean(fff_data_stream);

    FuriString* preset_name = furi_string_alloc();
    FuriString* protocol_name = furi_string_alloc();
    FuriString* temp_str = furi_string_alloc();
    uint32_t temp_data32;
    uint32_t frequency = 0;

    *is_raw = false;
    bool loaded = false;

    do {
        if(!flipper_format_file_open_existing(fff_data_file, file_name)) {
            FURI_LOG_E(TAG, "Error opening %s", file_name);
            ACTION_SET_ERROR("SUBGHZ: Error opening %s", file_name);
            break;
        }

        if(!flipper_format_read_header(fff_data_file, temp_str, &temp_data32)) {
            FURI_LOG_E(TAG, "Missing or incorrect header");
            ACTION_SET_ERROR("SUBGHZ: Missing or incorrect header");
            break;
        }

        if(((!strcmp(furi_string_get_cstr(temp_str), SUBGHZ_KEY_FILE_TYPE)) ||
            (!strcmp(furi_string_get_cstr(temp_str), SUBGHZ_RAW_FILE_TYPE))) &&
           temp_data32 == SUBGHZ_KEY_FILE_VERSION) {
        } else {
            FURI_LOG_E(TAG, "Type or version mismatch");
            ACTION_SET_ERROR("SUBGHZ: Type or version mismatch");
            break;
        }

        SubGhzSetting* setting = subghz_txrx_get_setting(txrx);
        if(!flipper_format_read_uint32(fff_data_file, "Frequency", &frequency, 1)) {
            FURI_LOG_W(TAG, "Missing Frequency. Setting default frequency");
            frequency = subghz_setting_get_default_frequency(setting);
        } else if(!subghz_txrx_radio_device_is_frequecy_valid(txrx, frequency)) {
            FURI_LOG_E(TAG, "Frequency not supported on the chosen radio module");
            ACTION_SET_ERROR("SUBGHZ: Frequency not supported");
            break;
        }

        if(!flipper_format_read_string(fff_data_file, "Preset", temp_str)) {
            FURI_LOG_E(TAG, "Missing Preset");
            ACTION_SET_ERROR("SUBGHZ: Missing preset");
            break;
        }

        furi_string_set_str(
            temp_str, subghz_txrx_get_preset_name(txrx, furi_string_get_cstr(temp_str)));
        if(!strcmp(furi_string_get_cstr(temp_str), "")) {
            FURI_LOG_E(TAG, "Unknown preset");
            ACTION_SET_ERROR("SUBGHZ: Unknown preset");
            break;
        }

        if(!strcmp(furi_string_get_cstr(temp_str), "CUSTOM")) {
            subghz_setting_delete_custom_preset(setting, furi_string_get_cstr(temp_str));
            if(!subghz_setting_load_custom_preset(
                   setting, furi_string_get_cstr(temp_str), fff_data_file)) {
                FURI_LOG_E(TAG, "Missing Custom preset");
                ACTION_SET_ERROR("SUBGHZ: Missing Custom preset");
                break;
            }
        }
        furi_string_set(preset_name, temp_str);
        size_t preset_index =
            subghz_setting_get_inx_preset_by_name(setting, furi_string_get_cstr(preset_name));
        subghz_txrx_set_preset(
            txrx,
            furi_string_get_cstr(preset_name),
            frequency,
            subghz_setting_get_preset_data(setting, preset_index),
            subghz_setting_get_preset_data_size(setting, preset_index));

        // Load Protocol
        if(!flipper_format_read_string(fff_data_file, "Protocol", protocol_name)) {
            FURI_LOG_E(TAG, "Missing protocol");
            ACTION_SET_ERROR("SUBGHZ: Missing protocol");
            break;
        }

        FlipperFormat* fff_data = subghz_txrx_get_fff_data(txrx);
        if(!strcmp(furi_string_get_cstr(protocol_name), "RAW")) {
            subghz_protocol_raw_gen_fff_data(
                fff_data, file_name, subghz_txrx_radio_device_get_name(txrx));
            *is_raw = true;
        } else {
            stream_copy_full(
                flipper_format_get_raw_stream(fff_data_file),
                flipper_format_get_raw_stream(fff_data));
        }

        if(subghz_txrx_load_decoder_by_name_protocol(txrx, furi_string_get_cstr(protocol_name))) {
            SubGhzProtocolStatus status =
                subghz_protocol_decoder_base_deserialize(subghz_txrx_get_decoder(txrx), fff_data);
            if(status != SubGhzProtocolStatusOk) {
                ACTION_SET_ERROR("SUBGHZ: Failed to deserialize protocol");
                break;
            }
        } else {
            FURI_LOG_E(TAG, "Protocol not found: %s", furi_string_get_cstr(protocol_name));
            ACTION_SET_ERROR("SUBGHZ: Protocol not found");
            break;
        }

        loaded = true;
    } while(false);

    flipper_format_file_close(fff_data_file);
    flipper_format_free(fff_data_file);
    furi_string_free(preset_name);
    furi_string_free(protocol_name);
    furi_string_free(temp_str);

    return loaded;
}

void action_subghz_tx_loaded(
    void* context,
    SubGhzTxRx* txrx,
    const FuriString* action_path,
    bool is_raw,
    FuriString* error) {
    App* app = context;
    SubGhzNeedSaveContext save_context = {app, txrx, action_path};
    subghz_txrx_set_need_save_callback(txrx, action_subghz_need_save_callback, &save_context);

    bool tx_started = subghz_txrx_tx_start(txrx, subghz_txrx_get_fff_data(txrx)) ==
                      SubGhzTxRxStartTxStateOk;
    if(!tx_started) {
        FURI_LOG_E(TAG, "Failed to start TX");
        ACTION_SET_ERROR("SUBGHZ: Failed to start TX");
    } else {
        if(is_raw) {
            subghz_txrx_set_raw_file_encoder_worker_callback_end(
                txrx, action_subghz_raw_end_callback, furi_thread_get_current());
            furi_thread_flags_wait(0, FuriFlagWaitAll, FuriWaitForever);
        } else {
            furi_delay_ms(app->settings.subghz_duration);
        }
    }

    FURI_LOG_I(TAG, "SUBGHZ: Action complete.");

    // This will call need_save_callback, if necessary
    subghz_txrx_stop(txrx);
}

void action_subghz_tx(void* context, const FuriString* action_path, FuriString* error) {
    App* app = context;
    const char* file_name = furi_string_get_cstr(action_path);

    SubGhzTxRx* txrx = subghz_txrx_alloc();
    if(app->settings.subghz_use_ext_module) {
        subghz_txrx_radio_device_set(txrx, SubGhzRadioDeviceTypeExternalCC1101);
    }
    bool is_raw = false;

    FURI_LOG_I(TAG, "SUBGHZ: Action starting...");

    if(action_subghz_load_file(txrx, app->storage, file_name, &is_raw, error)) {
        action_subghz_tx_loaded(app, txrx, action_path, is_raw, error);
    }

    subghz_txrx_free(txrx);
}

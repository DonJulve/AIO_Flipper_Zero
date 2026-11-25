#pragma once

#include <furi.h>
#include <furi_hal.h>
#include <storage/storage.h>

#define ACTION_SET_ERROR(_msg_fmt, ...) furi_string_printf(error, _msg_fmt, ##__VA_ARGS__)

struct App;
typedef struct SubGhzTxRx SubGhzTxRx;

typedef struct SubGhzNeedSaveContext {
    struct App* app;
    SubGhzTxRx* txrx;
    const FuriString* file_path;
} SubGhzNeedSaveContext;

typedef struct PicopassEmulator PicopassEmulator;

PicopassEmulator*
    action_picopass_emulator_start(void* context, const char* path, FuriString* error);
void action_picopass_emulator_stop(PicopassEmulator* emu);
const uint8_t* action_picopass_emulator_get_csn(PicopassEmulator* emu);

void action_ql_resolve(
    void* context,
    const FuriString* action_path,
    FuriString* new_path,
    FuriString* error);

void action_subghz_need_save_callback(void* context);
bool action_subghz_load_file(
    SubGhzTxRx* txrx,
    Storage* storage,
    const char* file_name,
    bool* is_raw,
    FuriString* error);
void action_subghz_tx_loaded(
    void* context,
    SubGhzTxRx* txrx,
    const FuriString* action_path,
    bool is_raw,
    FuriString* error);

void action_subghz_tx(void* context, const FuriString* action_path, FuriString* error);
void action_rfid_tx(void* context, const FuriString* action_path, FuriString* error);
void action_ir_tx(void* context, const FuriString* action_path, FuriString* error);
void action_nfc_tx(void* context, const FuriString* action_path, FuriString* error);
void action_ibutton_tx(void* context, const FuriString* action_path, FuriString* error);
void action_qpl_tx(void* context, const FuriString* action_path, FuriString* error);
void action_picopass_tx(void* context, const FuriString* action_path, FuriString* error);

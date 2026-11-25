#pragma once

#include <gui/gui.h>
#include <gui/scene_manager.h>
#include <gui/view_dispatcher.h>
#include <gui/modules/variable_item_list.h>
#include <gui/modules/submenu.h>
#include <gui/modules/text_input.h>
#include <gui/modules/popup.h>
#include <gui/modules/button_menu.h>
#include <dialogs/dialogs.h>

#include <storage/storage.h>
#include <notification/notification_messages.h>

#include "views/action_menu.h"
#include "views/subghz_transmitter.h"
#include "views/nfc_emulate.h"
#include "views/rfid_emulate.h"
#include "views/ibutton_emulate.h"
#include "views/picopass_emulate.h"
#include "item.h"

#define JULVE_AIO_NAME "Julve AIO"
#define JULVE_AIO_ABOUT                               \
    "Quick Action remote control\nv" FAP_VERSION "\n" \
    "Julve AIO\n"                                     \
    "GitHub: @DonJulve"
#define TAG "Julve AIO" // log statement id

// Location of our actions and folders
#define JULVE_AIO_SETTINGS_FILENAME ".julve_aio.conf"
#define JULVE_AIO_SETTINGS_PATH     APP_DATA_PATH(JULVE_AIO_SETTINGS_FILENAME)

typedef enum {
    JULVE_AIO_APP_PORTRAIT,
    JULVE_AIO_APP_LANDSCAPE
} JulveAioAppLayout;

typedef struct SubGhzTxRx SubGhzTxRx;

typedef struct App {
    SceneManager* scene_manager;
    ViewDispatcher* view_dispatcher;

    ActionMenu* action_menu;
    ButtonMenu* button_menu;
    SubGhzTransmitterView* subghz_transmitter;
    NfcEmulateView* nfc_emulate_view;
    RfidEmulateView* rfid_emulate_view;
    iButtonEmulateView* ibutton_emulate_view;
    PicopassEmulateView* picopass_emulate_view;
    VariableItemList* vil_settings;
    DialogsApp* dialog;
    Submenu* sub_menu;
    TextInput* text_input;
    Popup* popup;

    Storage* storage;
    NotificationApp* notifications;

    ItemsView* items_view;
    int depth;
    int selected_item;
    bool restore_selected_item;

    FuriString* temp_str; // used for renames/etc
    FuriString* active_item_name; // display name for active remote/item scene
    char temp_cstr[MAX_NAME_LEN]; // used for renames/etc
    uint32_t temp_u32;

    SubGhzTxRx* subghz_txrx;
    void* subghz_scene_state;
    void* ir_remote_state;
    void* nfc_scene_state;
    void* rfid_scene_state;
    void* ibutton_scene_state;
    void* picopass_scene_state;

    struct {
        JulveAioAppLayout layout; // Defaults to Portrait
        bool show_icons; // Defaults to True
        bool show_headers; // Defaults to True
        uint32_t subghz_duration; // Defaults to 1500 ms
        uint32_t rfid_duration; // Defaults to 2500 ms
        uint32_t nfc_duration; // Defaults to 1000 ms
        uint32_t ibutton_duration; // Defaults to 1000 ms
        uint32_t picopass_duration; // Defaults to 1000 ms (used on tap & as playlist default)
        bool ir_use_ext_module; // Defaults to False
        bool subghz_use_ext_module; // Defaults to False
        bool show_hidden; // Defaults to False
    } settings;

} App;

App* app_alloc();
void app_free(App* app);

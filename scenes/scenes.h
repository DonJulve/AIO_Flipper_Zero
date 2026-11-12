#pragma once

#include <gui/scene_manager.h>
typedef enum {
    QScene_Items,
    QScene_Settings,
    QScene_ActionSettings,
    QScene_ActionRename,
    QScene_ActionCreateGroup,
    QScene_IRRemote,
    QScene_SubGhzTransmitter,
    QScene_About,
    QScene_count
} appScenes;

typedef enum {
    QView_ActionMenu, // main UI
    QView_Settings, // Variable Item List for App Settings
    QView_SubMenu, // Action: Rename, Delete, Import
    QView_TextInput, // Action: Rename, Create Group
    QView_Popup, // About screen
    QView_ButtonMenu, // IR Remote screen
    QView_SubGhzTransmitter, // SubGHz Rolling Code info & TX screen
} appView;

typedef enum {
    Event_DeviceSelected,
    Event_ButtonPressed,
    Event_ButtonPressedLong
} AppCustomEvents;

extern void (*const app_on_enter_handlers[])(void*);
extern bool (*const app_on_event_handlers[])(void*, SceneManagerEvent);
extern void (*const app_on_exit_handlers[])(void*);
extern const SceneManagerHandlers app_scene_handlers;

extern bool app_scene_custom_callback(void* context, uint32_t custom_event_id);
extern bool app_back_event_callback(void* context);

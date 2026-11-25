#pragma once

#include "scenes.h"

void scene_nfc_emulate_on_enter(void* context);
bool scene_nfc_emulate_on_event(void* context, SceneManagerEvent event);
void scene_nfc_emulate_on_exit(void* context);

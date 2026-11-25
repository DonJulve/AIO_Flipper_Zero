#pragma once

#include "scenes.h"

void scene_rfid_emulate_on_enter(void* context);
bool scene_rfid_emulate_on_event(void* context, SceneManagerEvent event);
void scene_rfid_emulate_on_exit(void* context);

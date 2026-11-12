#pragma once

#include <gui/scene_manager.h>

void scene_ir_remote_on_enter(void* context);
bool scene_ir_remote_on_event(void* context, SceneManagerEvent event);
void scene_ir_remote_on_exit(void* context);


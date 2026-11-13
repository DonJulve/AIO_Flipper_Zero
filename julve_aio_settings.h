#pragma once

#include "julve_aio.h"

/** Set the default Settings for Julve AIO */
void julve_aio_set_default_settings(App* app);

/** Load the Settings from the .julve_aio.conf file. If not found,
 * then load the defaults.
*/
void julve_aio_load_settings(App* app);

/** Save the current settings to the .julve_aio.conf file */
void julve_aio_save_settings(App* app);

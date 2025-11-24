/*
 * OBS WebRTC Link Plugin
 * Copyright (C) 2024 OBS-WebRTC-Link Contributors
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <https://www.gnu.org/licenses/>
 */

#include <obs-module.h>
#include "output/obs-webrtc-output.hpp"
#include "source/obs-webrtc-source.hpp"

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE("obs-webrtc-link", "en-US")

/**
 * @brief Plugin module information
 */
MODULE_EXPORT const char *obs_module_name(void)
{
	return "OBS WebRTC Link";
}

/**
 * @brief Plugin module description
 */
MODULE_EXPORT const char *obs_module_description(void)
{
	return "WebRTC plugin for OBS Studio that provides universal WebRTC input & output. "
	       "Supports both SFU Relay (WHIP/WHEP) and Direct P2P connections.";
}

/**
 * @brief Plugin module load function
 *
 * This function is called when the plugin is loaded by OBS Studio.
 * Register sources, outputs, and other plugin components here.
 *
 * @return true if plugin loaded successfully, false otherwise
 */
bool obs_module_load(void)
{
	blog(LOG_INFO, "[OBS WebRTC Link] Plugin version %s loaded", PLUGIN_VERSION);
	blog(LOG_INFO, "[OBS WebRTC Link] Project structure initialized");

	// Register WebRTC Output (Issue #11)
	register_webrtc_output();

	// Register WebRTC Source (Issue #12)
	register_webrtc_source();

	return true;
}

/**
 * @brief Plugin module unload function
 *
 * This function is called when the plugin is unloaded by OBS Studio.
 * Clean up resources here.
 */
void obs_module_unload(void)
{
	blog(LOG_INFO, "[OBS WebRTC Link] Plugin unloaded");
}

/**
 * @brief Plugin module author
 */
MODULE_EXPORT const char *obs_module_author(void)
{
	return "OBS-WebRTC-Link Contributors";
}

/**
 * @brief Get plugin version
 */
const char *obs_webrtc_link_version(void)
{
	return PLUGIN_VERSION;
}

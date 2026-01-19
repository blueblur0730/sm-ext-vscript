/**
 * =============================================================================
 * SourceMod VScript Extension
 * Copyright 2025-2026 ProjectSky
 * =============================================================================
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _INCLUDE_SOURCEMOD_EXTENSION_CONFIG_H_
#define _INCLUDE_SOURCEMOD_EXTENSION_CONFIG_H_

#define SMEXT_CONF_NAME         "SourceMod VScript Extension"
#define SMEXT_CONF_DESCRIPTION  "Provide VScript Native"
#define SMEXT_CONF_VERSION      "1.0.0"
#define SMEXT_CONF_VERSION_FILE	 1,0,0,0
#define SMEXT_CONF_AUTHOR       "ProjectSky"
#define SMEXT_CONF_URL          "https://github.com/ProjectSky/sm-ext-vscript"
#define SMEXT_CONF_LOGTAG       "VSCRIPT"
#define SMEXT_CONF_LICENSE      "GPL"
#define SMEXT_CONF_DATESTRING   __DATE__

#define SMEXT_LINK(name) SDKExtension *g_pExtensionIface = name;

#define SMEXT_CONF_METAMOD
#define SMEXT_ENABLE_HANDLESYS

#endif // _INCLUDE_SOURCEMOD_EXTENSION_CONFIG_H_

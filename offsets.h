#define ORIGIN 1
#define STEAM 2
#include "item.h"
#include "keycode.h"
#define VERSION STEAM
#define METER_TO_FLOAT 3000.0f / 70.0f // meter to float unit
#if VERSION == STEAM

#define OFFSET_VISIBLE_TIME 0x1a70    // CPlayer!lastVisibleTime
#define OFFSET_BULLET_SPEED 0x1f3c    // CWeaponX!m_flProjectileSpeed
#define OFFSET_BULLET_SCALE 0x1f44    // CWeaponX!m_flProjectileScale
#define OFFSET_ZOOM_FOV 0x16b0 + 0xb8 // [RecvTable.DT_WeaponX]->m_playerData + [DataMap.WeaponPlayerData]->m_curZoomFOV
#define OFFSET_MATRIX 0x11a350
#define OFFSET_RENDER 0x743AB20

#define OFFSET_INPUTSYSTEM 0x17a4400         // [Miscellaneous]->InputSystem
#define OFFSET_ENTITYLIST 0x1e53c68          // [Miscellaneous]->cl_entitylist
#define OFFSET_GLOBALVAR 0x16f9d80           // [Miscellaneous]->GlobalVars
#define OFFSET_LOCAL_ENT 0x22036C0 + 0x8     // [Globals]->.?AVC_GameMovement@@ + 0x8
#define OFFSET_GAMEMODE 0x0223c7e0 + 0x58    // [ConVars]->mp_gamemode + 0x58
#define OFFSET_NAME_LIST 0xbe94be0           // [Miscellaneous]->NameList
#define OFFSET_THIRDPERSON 0x01de35b0 + 0x6c // [ConVars]->thirdperson_override + 0x6c
#define OFFSET_TIMESCALE 0x01798b90          // [ConVars]->host_timescale
#define OFFSET_LEVEL_NAME 0x16fa230          // [Miscellaneous]->LevelName
#define OFFSET_TEAM 0x044c                   // [RecvTable.DT_BaseEntity]->m_iTeamNum
#define OFFSET_HEALTH 0x043c                 // [RecvTable.DT_Player]->m_iHealth
#define OFFSET_SHIELD 0x0170                 // [RecvTable.DT_BaseEntity]->m_shieldHealth
#define OFFSET_SHIELD_TYPE 0x4654            // [RecvTable.DT_Player]->m_armorType
#define OFFSET_NAME 0x0589                   // [RecvTable.DT_BaseEntity]->m_iName
#define OFFSET_MODEL_NAME 0x0030             // [DataMap.C_BaseEntity]->m_ModelName
#define OFFSET_SIGN_NAME 0x0580              // [RecvTable.DT_BaseEntity]->m_iSignifierName
#define OFFSET_ABS_VELOCITY 0x0140           // [DataMap.C_BaseEntity]->m_vecAbsVelocity
#define OFFSET_ZOOMING 0x1c51                // [DataMap.C_Player]->m_bZooming
#define OFFSET_THIRDPERSON_SV 0x36e8         // [RecvTable.DT_LocalPlayerExclusive]->m_thirdPersonShoulderView
#define OFFSET_YAW 0x22bc - 0x8              // [DataMap.C_Player]->m_currentFramePlayer.m_ammoPoolCount - 0x8
#define OFFSET_LIFE_STATE 0x0798             // [RecvTable.DT_Player]->m_lifeState          //>0 = dead
#define OFFSET_BLEED_OUT_STATE 0x2750        // [RecvTable.DT_Player]->m_bleedoutState //>0 = knocked
#define OFFSET_ORIGIN 0x014c                 // [DataMap.C_BaseEntity]->m_vecAbsOrigin
#define OFFSET_BONES 0x0e98 + 0x48           // [RecvTable.DT_BaseAnimating]->m_nForceBone + 0x48
#define OFFSET_STUDIOHDR 0x10e8              // [Miscellaneous]->CBaseAnimating!m_pStudioHdr
#define OFFSET_AIMPUNCH 0x24b8               // [DataMap.C_Player]->m_currentFrameLocalPlayer.m_vecPunchWeapon_Angle
#define OFFSET_CAMERAPOS 0x1f50              // [Miscellaneous]->CPlayer!camera_origin
#define OFFSET_VIEWANGLES 0x25b4 - 0x14      // [RecvTable.DT_Player]->m_ammoPoolCapacity - 0x14
#define OFFSET_OBSERVER_MODE 0x34f4          // [RecvTable.DT_LocalPlayerExclusive]->m_iObserverMode
#define OFFSET_OBSERVING_TARGET 0x3500       // [RecvTable.DT_LocalPlayerExclusive]->m_hObserverTarget
#define OFFSET_WEAPON 0x1a14                 // [RecvTable.DT_BaseCombatCharacter]->m_latestPrimaryWeapons
#define OFFSET_AMMO 0x1660                   // [DataMap.CWeaponX]->m_ammoInClip
#define OFFSET_ITEM_ID 0x1638                // [RecvTable.DT_PropSurvival]->m_customScriptInt
#define OFFSET_ITEM_GLOW 0x02c0              // [RecvTable.DT_HighlightSettings]->m_highlightFunctionBits
#define GLOW_TYPE 0x02c0 + 0x4               // [RecvTable.DT_HighlightSettings]->m_highlightFunctionBits + 0x4
#define GLOW_DISTANCE 0x0380 + 0x34          // [RecvTable.DT_HighlightSettings]->m_highlightServerFadeEndTimes + 0x34
#define OFFSET_VIEWMODEL 0x2d80              // [DataMap.C_Player]->m_hViewModels

#define OFFSET_GLOW_T1 0x262                          // 16256 = enabled, 0 = disabled
#define OFFSET_GLOW_T2 0x2dc                          // 1193322764 = enabled, 0 = disabled
#define OFFSET_GLOW_ENABLE 0x3c8                      // 7 = enabled, 2 = disabled
#define OFFSET_GLOW_THROUGH_WALLS 0x3d0               // 2 = enabled, 5 = disabled
#define OFFSET_ITEM_COLOR_R 0x01b8                    // [RecvTable.DT_HighlightSettings]->m_highlightParams				//get default colro for red
#define OFFSET_ITEM_COLOR_G OFFSET_ITEM_COLOR_R + 0x4 // OFFSET_ITEM_COLOR_R+0x4
#define OFFSET_ITEM_COLOR_B OFFSET_ITEM_COLOR_R + 0x8 // OFFSET_ITEM_COLOR_R+0x8

#define OFFSET_GLOW_COLOR_R OFFSET_ITEM_COLOR_R + 0x18 // glow color for Red			//m_highlightParams + 24 (0x18)
#define OFFSET_GLOW_COLOR_G OFFSET_GLOW_COLOR_R + 0x4  // glow color for Green
#define OFFSET_GLOW_COLOR_B OFFSET_GLOW_COLOR_R + 0x8  // glow color for Blue
#define OFFSET_BREATH_ANGLES OFFSET_VIEWANGLES - 0x10

#elif VERSION == ORIGIN

#define OFFSET_ENTITYLIST 0x1d88fc8
#define OFFSET_LOCAL_ENT 0x0213a5c0 + 0x8 //.?AVC_GameMovement@@ + 0x8
#define OFFSET_NAME_LIST 0xbd28ca0
#define OFFSET_THIRDPERSON 0x01d18740 + 0x6c // thirdperson_override + 0x6c
#define OFFSET_TIMESCALE 0x016cdc80          // host_timescale

#define OFFSET_TEAM 0x044c           // m_iTeamNum
#define OFFSET_HEALTH 0x043c         // m_iHealth
#define OFFSET_SHIELD 0x170          // m_shieldHealth
#define OFFSET_NAME 0x589            // m_iName
#define OFFSET_SIGN_NAME 0x580       // m_iSignifierName
#define OFFSET_ABS_VELOCITY 0x140    // m_vecAbsVelocity
#define OFFSET_VISIBLE_TIME 0x1a80   // CPlayer!lastVisibleTime
#define OFFSET_ZOOMING 0x1c61        // m_bZooming
#define OFFSET_THIRDPERSON_SV 0x36e0 // m_thirdPersonShoulderView
#define OFFSET_YAW 0x22c4 - 0x8      // m_currentFramePlayer.m_ammoPoolCount - 0x8

#define OFFSET_LIFE_STATE 0x798       // m_lifeState, >0 = dead
#define OFFSET_BLEED_OUT_STATE 0x2750 // m_bleedoutState, >0 = knocked

#define OFFSET_ORIGIN 0x014c            // m_vecAbsOrigin
#define OFFSET_BONES 0x0e98 + 0x48      // m_nForceBone + 0x48
#define OFFSET_STUDIOHDR 0x10f0         // CBaseAnimating!m_pStudioHdr
#define OFFSET_AIMPUNCH 0x24c0          // m_currentFrameLocalPlayer.m_vecPunchWeapon_Angle
#define OFFSET_CAMERAPOS 0x1f58         // CPlayer!camera_origin
#define OFFSET_VIEWANGLES 0x25bc - 0x14 // m_ammoPoolCapacity - 0x14
#define OFFSET_BREATH_ANGLES OFFSET_VIEWANGLES - 0x10
#define OFFSET_OBSERVER_MODE 0x34ec    // m_iObserverMode
#define OFFSET_OBSERVING_TARGET 0x34f8 // m_hObserverTarget

#define OFFSET_MATRIX 0x11a350
#define OFFSET_RENDER 0x7401cd0

#define OFFSET_WEAPON 0x1a24          // m_latestPrimaryWeapons
#define OFFSET_BULLET_SPEED 0x1f3c    // CWeaponX!m_flProjectileSpeed
#define OFFSET_BULLET_SCALE 0x1f44    // CWeaponX!m_flProjectileScale
#define OFFSET_ZOOM_FOV 0x16c0 + 0xb8 // m_playerData + m_curZoomFOV
#define OFFSET_AMMO 0x1644            // m_ammoInClip

#define OFFSET_ITEM_GLOW 0x2c0 // m_highlightFunctionBits

#define OFFSET_GLOW_T1 0x262            // 16256 = enabled, 0 = disabled
#define OFFSET_GLOW_T2 0x2dc            // 1193322764 = enabled, 0 = disabled
#define OFFSET_GLOW_ENABLE 0x3c8        // 7 = enabled, 2 = disabled
#define OFFSET_GLOW_THROUGH_WALLS 0x3d0 // 2 = enabled, 5 = disabled

#endif
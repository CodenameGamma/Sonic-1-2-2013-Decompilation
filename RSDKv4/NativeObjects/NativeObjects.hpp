#ifndef NATIVE_OBJECTS_H
#define NATIVE_OBJECTS_H

#define RSDK_THIS(type)     NativeEntity_##type *entity = (NativeEntity_##type *)objPtr
#define CREATE_ENTITY(type) ((NativeEntity_##type *)CreateNativeObject(type##_Create, type##_Main))

extern bool usePhysicalControls;

#include "MenuBG.hpp"
#include "TextLabel.hpp"
#include "PushButton.hpp"
#include "SubMenuButton.hpp"
#include "DialogPanel.hpp"
#include "FadeScreen.hpp"
#include "VirtualDPad.hpp"
#include "VirtualDPadM.hpp"
#include "RetroGameLoop.hpp"
#include "PauseMenu.hpp"
#include "SegaSplash.hpp"
#include "CWSplash.hpp"
#include "TitleScreen.hpp"
#include "StartGameButton.hpp"
#include "TimeAttackButton.hpp"
#include "AchievementsButton.hpp"
#include "MultiplayerButton.hpp"
#include "LeaderboardsButton.hpp"
#if RETRO_USE_MOD_LOADER
#include "ModsButton.hpp"
#endif
#include "OptionsButton.hpp"
#include "BackButton.hpp"
#include "MenuControl.hpp"

#endif // !NATIVE_OBJECTS_H
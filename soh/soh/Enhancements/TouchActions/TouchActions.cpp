// Android touch extras: a dedicated Roll button (separate from A) and a Sprint button with stamina.
#include <libultraship/bridge.h>
#include "soh/Enhancements/game-interactor/GameInteractor.h"
#include "soh/OTRGlobals.h"
#include "soh/ShipInit.hpp"
#include <SDL.h>
#include <atomic>
#include <cmath>
#ifdef __ANDROID__
#include <jni.h>
#endif

extern "C" {
#include "functions.h"
#include "variables.h"
#include "macros.h"
#include "sfx.h"
extern PlayState* gPlayState;

void Player_Action_80842180(Player* player, PlayState* play); // untargeted walking/running
void Player_PlayVoiceSfx(Player* player, u16 sfxId);
}

#define CVAR_SEPARATE_ROLL CVAR_ENHANCEMENT("SeparateRollButton")
#define CVAR_TOUCH_SPRINT CVAR_ENHANCEMENT("TouchSprint")
#define CVAR_TOUCH_ITEM_SELECT CVAR_ENHANCEMENT("TouchItemSelect")

static bool ChangesAllowed() {
    return !CVarGetInteger(CVAR_SETTING("DisableChanges"), 0);
}

static bool SeparateRollEnabled() {
    return CVarGetInteger(CVAR_SEPARATE_ROLL, 0) && ChangesAllowed();
}

static bool SprintEnabled() {
    return CVarGetInteger(CVAR_TOUCH_SPRINT, 0) && ChangesAllowed();
}

static bool ItemSelectEnabled() {
    return CVarGetInteger(CVAR_TOUCH_ITEM_SELECT, 0) && ChangesAllowed();
}

// ---------------------------------------------------------------------------------------------
// Tap an item icon to select it, then A uses it (hold to aim, release to fire for ranged items).
// -1 = nothing selected (A behaves normally), 0 = C-left, 1 = C-down, 2 = C-right.
// ---------------------------------------------------------------------------------------------
static std::atomic<int> sSelectedItem{ -1 };

// Called at the start of the player's update with the player's own copy of the input, so menus,
// textboxes and everything else still see the real A button. While an item is selected, A is
// turned into the matching C button (pressed / held / released all follow A).
extern "C" void TouchActions_RemapPlayerInput(Input* input) {
    if (input == nullptr || !ItemSelectEnabled()) {
        return;
    }
    const int slot = sSelectedItem.load();
    if (slot < 0 || slot > 2) {
        return;
    }
    static const u16 cButtons[3] = { BTN_CLEFT, BTN_CDOWN, BTN_CRIGHT };
    OSContPad* pads[] = { &input->cur, &input->prev, &input->press, &input->rel };
    for (OSContPad* pad : pads) {
        if (pad->button & BTN_A) {
            pad->button = (pad->button & ~BTN_A) | cButtons[slot];
        }
    }
}

// ---------------------------------------------------------------------------------------------
// Separate roll button
// ---------------------------------------------------------------------------------------------
static std::atomic<bool> sRollPending{ false };
static std::atomic<uint32_t> sRollPendingTime{ 0 };
static bool sRollThisFrame = false;
static constexpr uint32_t ROLL_PENDING_MAX_MS = 250;

// ---------------------------------------------------------------------------------------------
// Sprint + stamina (one game update = one tick, the game runs at 20 updates per second)
// ---------------------------------------------------------------------------------------------
static std::atomic<bool> sSprintHeld{ false };
static std::atomic<int> sStaminaUi{ -1 }; // 0-100, +1000 while tired, -1 when sprint is disabled
static float sStamina = 1.0f;
static bool sTired = false;
static bool sSprintedThisFrame = false;

static constexpr float SPRINT_SPEED_MULT = 1.5f;     // faster than a normal run
static constexpr float TIRED_SPEED_MULT = 0.8f;      // slower than normal while out of breath
static constexpr float DRAIN_PER_FRAME = 1.0f / 100; // about 5 seconds of sprinting from full
static constexpr float REGEN_PER_FRAME = 1.0f / 200; // about 10 seconds to refill while moving
static constexpr float RECOVER_THRESHOLD = 0.3f;     // tired until stamina is back above 30%

static void PublishStamina() {
    if (!SprintEnabled()) {
        sStaminaUi.store(-1);
        return;
    }
    int pct = (int)std::lround(sStamina * 100.0f);
    sStaminaUi.store(pct + (sTired ? 1000 : 0));
}

static void TickStamina() {
    if (!SprintEnabled()) {
        sStamina = 1.0f;
        sTired = false;
        sSprintedThisFrame = false;
        PublishStamina();
        return;
    }

    if (sSprintedThisFrame) {
        sStamina -= DRAIN_PER_FRAME;
        if (sStamina <= 0.0f) {
            sStamina = 0.0f;
            if (!sTired) {
                sTired = true;
                Player* player = (gPlayState != nullptr) ? GET_PLAYER(gPlayState) : nullptr;
                if (player != nullptr) {
                    Player_PlayVoiceSfx(player, NA_SE_VO_LI_BREATH_REST);
                }
            }
        }
    } else {
        Player* player = (gPlayState != nullptr) ? GET_PLAYER(gPlayState) : nullptr;
        bool standingStill = player != nullptr && std::fabs(player->linearVelocity) < 1.0f;
        sStamina += REGEN_PER_FRAME * (standingStill ? 2.0f : 1.0f);
        if (sStamina > 1.0f) {
            sStamina = 1.0f;
        }
    }

    if (sTired && sStamina >= RECOVER_THRESHOLD) {
        sTired = false;
    }
    sSprintedThisFrame = false;
    PublishStamina();
}

// Runs at the end of every player update (not while paused).
static void OnPlayerUpdateTouchActions() {
    const bool pending = sRollPending.exchange(false);
    sRollThisFrame = pending && uint32_t(SDL_GetTicks() - sRollPendingTime.load()) <= ROLL_PENDING_MAX_MS;
    TickStamina();
}

static void RegisterTouchActions() {
    sRollThisFrame = false;
    sSprintedThisFrame = false;
    PublishStamina();

    COND_HOOK(OnPlayerUpdate, SeparateRollEnabled() || SprintEnabled(), OnPlayerUpdateTouchActions);

    // A no longer rolls: only the touch Roll button does. A keeps its other jobs (put away sword, Navi).
    COND_VB_SHOULD(VB_PLAYER_ROLL_BUTTON_PRESSED, CVarGetInteger(CVAR_SEPARATE_ROLL, 0), {
        if (SeparateRollEnabled()) {
            *should = sRollThisFrame;
        }
    });

    // Scale the run target speed while the sprint button is held.
    COND_VB_SHOULD(VB_PLAYER_MODIFY_RUN_SPEED, CVarGetInteger(CVAR_TOUCH_SPRINT, 0), {
        Player* player = va_arg(args, Player*);
        f32* speedTarget = va_arg(args, f32*);
        if (SprintEnabled() && player != nullptr && speedTarget != nullptr && *speedTarget > 0.0f &&
            player->actionFunc == Player_Action_80842180) {
            if (sTired) {
                *speedTarget *= TIRED_SPEED_MULT;
            } else if (sSprintHeld.load() && sStamina > 0.0f) {
                *speedTarget *= SPRINT_SPEED_MULT;
                sSprintedThisFrame = true;
            }
        }
    });
}

static RegisterShipInitFunc initFunc(RegisterTouchActions, { CVAR_SEPARATE_ROLL, CVAR_TOUCH_SPRINT });

#ifdef __ANDROID__
extern "C" JNIEXPORT void JNICALL Java_com_dishii_soh_MainActivity_nativeTouchRoll(JNIEnv*, jobject) {
    sRollPendingTime.store(SDL_GetTicks());
    sRollPending.store(true);
}

extern "C" JNIEXPORT jboolean JNICALL Java_com_dishii_soh_MainActivity_nativeIsSeparateRoll(JNIEnv*, jobject) {
    return SeparateRollEnabled() ? JNI_TRUE : JNI_FALSE;
}

extern "C" JNIEXPORT void JNICALL Java_com_dishii_soh_MainActivity_nativeSetSelectedItem(JNIEnv*, jobject,
                                                                                         jint slot) {
    sSelectedItem.store((int)slot);
}

extern "C" JNIEXPORT jboolean JNICALL Java_com_dishii_soh_MainActivity_nativeIsItemSelectEnabled(JNIEnv*, jobject) {
    return ItemSelectEnabled() ? JNI_TRUE : JNI_FALSE;
}

extern "C" JNIEXPORT void JNICALL Java_com_dishii_soh_MainActivity_nativeTouchSprint(JNIEnv*, jobject, jboolean down) {
    sSprintHeld.store(down == JNI_TRUE);
}

// -1: sprint disabled. Otherwise stamina percent (0-100), plus 1000 while tired.
extern "C" JNIEXPORT jint JNICALL Java_com_dishii_soh_MainActivity_nativeGetStamina(JNIEnv*, jobject) {
    return sStaminaUi.load();
}
#endif

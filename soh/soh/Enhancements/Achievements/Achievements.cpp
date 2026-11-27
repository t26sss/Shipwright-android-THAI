// Achievements: a small RetroAchievements-style system for Ship of Harkinian.
//
// - Each achievement has an id, title, description and point value.
// - Unlocks are stored globally (not per save file) as CVars: gAchievements.<id> = unix time.
// - "Polled" achievements are state based and are checked a few times per second while in-game.
// - "Event" achievements (boss fights etc.) are unlocked by hooks registered in RegisterAchievementHooks.

#include "Achievements.h"

#include <ctime>
#include <string>
#include <vector>
#include <functional>

#include <libultraship/bridge/consolevariablebridge.h>
#include <ship/Context.h>
#include <ship/window/Window.h>
#include <ship/window/gui/Gui.h>

#include "soh/Enhancements/game-interactor/GameInteractor.h"
#include "soh/Notification/Notification.h"
#include "soh/ShipInit.hpp"
#include "soh/cvar_prefixes.h"

extern "C" {
#include "z64.h"
#include "macros.h"
#include "functions.h"
#include "variables.h"
#include "src/overlays/actors/ovl_Boss_Goma/z_boss_goma.h"
extern PlayState* gPlayState;
// Gohma action functions (non-static in z_boss_goma.c)
extern void BossGoma_FloorMain(BossGoma*, PlayState*);
extern void BossGoma_WallClimb(BossGoma*, PlayState*);
extern void BossGoma_CeilingMoveToCenter(BossGoma*, PlayState*);
extern void BossGoma_CeilingIdle(BossGoma*, PlayState*);
extern void BossGoma_CeilingPrepareSpawnGohmas(BossGoma*, PlayState*);
extern void BossGoma_CeilingSpawnGohmas(BossGoma*, PlayState*);
}

namespace Achievements {

struct Def {
    const char* id;
    const char* title;
    const char* description;
    int points;
    std::function<bool()> poll; // nullptr for event driven achievements
};

static bool InGame() {
    return gPlayState != nullptr && gSaveContext.fileNum >= 0 && gSaveContext.fileNum < 3;
}

static const std::vector<Def>& GetDefs() {
    static const std::vector<Def> defs = {
        { "first_blade", "First Blade", "Obtain a sword for the first time", 5,
          [] {
              return CHECK_OWNED_EQUIP(EQUIP_TYPE_SWORD, EQUIP_INV_SWORD_KOKIRI) ||
                     CHECK_OWNED_EQUIP(EQUIP_TYPE_SWORD, EQUIP_INV_SWORD_MASTER) ||
                     CHECK_OWNED_EQUIP(EQUIP_TYPE_SWORD, EQUIP_INV_SWORD_BIGGORON);
          } },
        { "entrusted_mission", "Entrusted Mission", "Receive the Spiritual Stone of the Forest", 5,
          [] { return CHECK_QUEST_ITEM(QUEST_KOKIRI_EMERALD) != 0; } },
        { "soul_brothers", "Soul Brothers", "Receive the Spiritual Stone of Fire", 5,
          [] { return CHECK_QUEST_ITEM(QUEST_GORON_RUBY) != 0; } },
        { "rutos_promise", "Ruto's Promise", "Receive the Spiritual Stone of Water", 5,
          [] { return CHECK_QUEST_ITEM(QUEST_ZORA_SAPPHIRE) != 0; } },
        { "royal_autograph", "A Royal Autograph", "Meet Princess Zelda for the first time", 5,
          [] { return Flags_GetEventChkInf(EVENTCHKINF_OBTAINED_ZELDAS_LETTER) != 0; } },
        { "rise_and_shine", "Rise and Shine!!", "Help Malon locate her father", 5,
          [] { return Flags_GetEventChkInf(EVENTCHKINF_TALON_WOKEN_IN_CASTLE) != 0; } },
        { "dance_into_the_fire", "Dance Into the Fire", "Visit Darunia in Goron City and obtain a gift", 5,
          [] { return CUR_UPG_VALUE(UPG_STRENGTH) >= 1; } },
        { "spider_net", "Spider Net", "Defeat Queen Gohma before she can retreat to the ceiling", 10, nullptr },
        { "bombalicious", "Bombalicious", "Defeat King Dodongo without using Bombs from your bag", 10, nullptr },
    };
    return defs;
}

static const Def* FindDef(const char* id) {
    for (const auto& d : GetDefs()) {
        if (std::string(d.id) == id) {
            return &d;
        }
    }
    return nullptr;
}

static std::string CvarName(const char* id) {
    return std::string("gAchievements.") + id;
}

bool IsUnlocked(const char* id) {
    return CVarGetInteger(CvarName(id).c_str(), 0) != 0;
}

void Unlock(const char* id) {
    const Def* def = FindDef(id);
    if (def == nullptr || IsUnlocked(id)) {
        return;
    }
    CVarSetInteger(CvarName(id).c_str(), static_cast<int32_t>(std::time(nullptr)));
    Ship::Context::GetRawInstance()->GetWindow()->GetGui()->SaveConsoleVariablesNextFrame();

    Notification::Emit({
        .itemIcon = "__OTR__textures/icon_item_24_static/gQuestIconGoldSkulltulaTex",
        .prefix = "Achievement Unlocked:",
        .message = def->title,
        .suffix = "(" + std::to_string(def->points) + ")",
        .remainingTime = 10.0f,
    });
}

void ResetAll() {
    for (const auto& d : GetDefs()) {
        CVarClear(CvarName(d.id).c_str());
    }
    Ship::Context::GetRawInstance()->GetWindow()->GetGui()->SaveConsoleVariablesNextFrame();
}

// ---------------------------------------------------------------------------------------------
// Window
// ---------------------------------------------------------------------------------------------

void Window::DrawElement() {
    int earned = 0;
    int total = 0;
    int earnedPts = 0;
    int totalPts = 0;
    for (const auto& d : GetDefs()) {
        total++;
        totalPts += d.points;
        if (IsUnlocked(d.id)) {
            earned++;
            earnedPts += d.points;
        }
    }

    ImGui::Text("%d / %d unlocked   (%d / %d points)", earned, total, earnedPts, totalPts);
    ImGui::ProgressBar(total ? static_cast<float>(earned) / total : 0.0f, ImVec2(-1, 0));
    ImGui::Separator();

    for (const auto& d : GetDefs()) {
        bool unlocked = IsUnlocked(d.id);
        ImVec4 titleColor = unlocked ? ImVec4(1.0f, 0.8f, 0.2f, 1.0f) : ImVec4(0.6f, 0.6f, 0.6f, 1.0f);
        ImGui::PushID(d.id);
        ImGui::TextColored(titleColor, "%s (%d)", d.title, d.points);
        ImGui::TextWrapped("%s", d.description);
        if (unlocked) {
            time_t t = static_cast<time_t>(CVarGetInteger(CvarName(d.id).c_str(), 0));
            char buf[32] = "";
            std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M", std::localtime(&t));
            ImGui::TextColored(ImVec4(0.4f, 0.9f, 0.4f, 1.0f), "Unlocked %s", buf);
        } else {
            ImGui::TextDisabled("Locked");
        }
        ImGui::Separator();
        ImGui::PopID();
    }

    if (ImGui::Button("Reset all achievements")) {
        ImGui::OpenPopup("ResetAchievementsConfirm");
    }
    if (ImGui::BeginPopupModal("ResetAchievementsConfirm", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Really reset every achievement?");
        if (ImGui::Button("Yes")) {
            ResetAll();
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

// ---------------------------------------------------------------------------------------------
// Hooks
// ---------------------------------------------------------------------------------------------

// Queen Gohma: fail the run once she goes back up to the ceiling after the fight has started.
static bool sGohmaFightStarted = false;
static bool sGohmaRetreated = false;

// King Dodongo: fail the run if the number of bombs in the bag ever goes down inside the boss room.
static bool sDodongoBombUsed = false;
static int sLastBombCount = 0;

static void PollAchievements() {
    static int frameCounter = 0;
    if (!InGame()) {
        return;
    }
    // Check ~4 times a second
    if (++frameCounter < 5) {
        return;
    }
    frameCounter = 0;

    for (const auto& d : GetDefs()) {
        if (d.poll && !IsUnlocked(d.id) && d.poll()) {
            Unlock(d.id);
        }
    }

    if (gPlayState->sceneNum == SCENE_DODONGOS_CAVERN_BOSS) {
        int bombs = AMMO(ITEM_BOMB);
        if (bombs < sLastBombCount) {
            sDodongoBombUsed = true;
        }
        sLastBombCount = bombs;
    }
}

static void RegisterAchievementHooks() {
    GameInteractor::Instance->RegisterGameHook<GameInteractor::OnGameFrameUpdate>(PollAchievements);

    GameInteractor::Instance->RegisterGameHook<GameInteractor::OnSceneInit>([](int16_t sceneNum) {
        if (sceneNum == SCENE_DEKU_TREE_BOSS) {
            sGohmaFightStarted = false;
            sGohmaRetreated = false;
        } else if (sceneNum == SCENE_DODONGOS_CAVERN_BOSS) {
            sDodongoBombUsed = false;
            sLastBombCount = AMMO(ITEM_BOMB);
        }
    });

    // Gohma
    GameInteractor::Instance->RegisterGameHookForID<GameInteractor::OnActorUpdate>(ACTOR_BOSS_GOMA, [](void* refActor) {
        BossGoma* gohma = static_cast<BossGoma*>(refActor);
        if (gohma->actionFunc == BossGoma_FloorMain) {
            sGohmaFightStarted = true;
        } else if (sGohmaFightStarted &&
                   (gohma->actionFunc == BossGoma_WallClimb || gohma->actionFunc == BossGoma_CeilingMoveToCenter ||
                    gohma->actionFunc == BossGoma_CeilingIdle ||
                    gohma->actionFunc == BossGoma_CeilingPrepareSpawnGohmas ||
                    gohma->actionFunc == BossGoma_CeilingSpawnGohmas)) {
            sGohmaRetreated = true;
        }
    });
    GameInteractor::Instance->RegisterGameHookForID<GameInteractor::OnBossDefeat>(ACTOR_BOSS_GOMA, [](void* refActor) {
        if (sGohmaFightStarted && !sGohmaRetreated) {
            Unlock("spider_net");
        }
    });

    // King Dodongo
    GameInteractor::Instance->RegisterGameHookForID<GameInteractor::OnBossDefeat>(ACTOR_BOSS_DODONGO,
                                                                                  [](void* refActor) {
        if (!sDodongoBombUsed) {
            Unlock("bombalicious");
        }
    });
}

static RegisterShipInitFunc initFunc(RegisterAchievementHooks);

} // namespace Achievements

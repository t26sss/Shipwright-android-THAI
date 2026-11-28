// Achievements: a RetroAchievements-style system for Ship of Harkinian (Thai edition).
//
// - The list of achievements (names/descriptions in English and Thai, points) lives in AchievementData.inc
// - Icons live in soh/assets/custom/textures/achievements/achNNN.rgba32.png (+ achNNN_lock.rgba32.png)
// - Unlocks are stored globally (not per save file) as CVars: gAchievements.<key> = unix time.
// - "Polled" achievements are state based and are checked a few times per second while in-game.
// - "Event" achievements (boss fights etc.) are unlocked by hooks registered in RegisterAchievementHooks.
// - Achievements without a detection rule yet are still listed (locked) and can't be unlocked.

#include "Achievements.h"

#include <ctime>
#include <string>
#include <vector>
#include <functional>
#include <unordered_map>
#include <unordered_set>

#include <libultraship/bridge/consolevariablebridge.h>
#include <ship/Context.h>
#include <ship/window/Window.h>
#include <ship/window/gui/Gui.h>
#include <fast/Fast3dGui.h>

#include "soh/Enhancements/game-interactor/GameInteractor.h"
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
    int num;
    const char* nameEN;
    const char* nameTH;
    const char* descEN;
    const char* descTH;
    int points;
};

static const std::vector<Def>& GetDefs() {
    static const std::vector<Def> defs = {
#define ACH(num, nameEN, nameTH, descEN, descTH, points) { num, nameEN, nameTH, descEN, descTH, points },
#include "AchievementData.inc"
#undef ACH
    };
    return defs;
}

static const Def* FindDef(int num) {
    for (const auto& d : GetDefs()) {
        if (d.num == num) {
            return &d;
        }
    }
    return nullptr;
}

static bool InGame() {
    return gPlayState != nullptr && gSaveContext.fileNum >= 0 && gSaveContext.fileNum < 3;
}

#define HAS_ITEM(item) (INV_CONTENT(item) == (item))

// State based achievements: num -> condition. Checked a few times per second while playing.
static const std::unordered_map<int, std::function<bool()>>& GetPolls() {
    static const std::unordered_map<int, std::function<bool()>> polls = {
        // Story / quest items
        { 2, [] { return CHECK_QUEST_ITEM(QUEST_KOKIRI_EMERALD) != 0; } },
        { 7, [] { return CHECK_QUEST_ITEM(QUEST_GORON_RUBY) != 0; } },
        { 9, [] { return CHECK_QUEST_ITEM(QUEST_ZORA_SAPPHIRE) != 0; } },
        { 4, [] { return Flags_GetEventChkInf(EVENTCHKINF_OBTAINED_ZELDAS_LETTER) != 0; } },
        { 3, [] { return Flags_GetEventChkInf(EVENTCHKINF_TALON_WOKEN_IN_CASTLE) != 0; } },
        { 5, [] { return CUR_UPG_VALUE(UPG_STRENGTH) >= 1; } },
        { 10, [] { return HAS_ITEM(ITEM_OCARINA_TIME); } },
        { 11, [] { return gSaveContext.linkAge == LINK_AGE_ADULT; } },
        // Medallions
        { 14, [] { return CHECK_QUEST_ITEM(QUEST_MEDALLION_FOREST) != 0; } },
        { 16, [] { return CHECK_QUEST_ITEM(QUEST_MEDALLION_FIRE) != 0; } },
        { 20, [] { return CHECK_QUEST_ITEM(QUEST_MEDALLION_WATER) != 0; } },
        { 22, [] { return CHECK_QUEST_ITEM(QUEST_MEDALLION_SHADOW) != 0; } },
        { 27, [] { return CHECK_QUEST_ITEM(QUEST_MEDALLION_SPIRIT) != 0; } },
        // Equipment
        { 17, [] { return CHECK_OWNED_EQUIP(EQUIP_TYPE_BOOTS, EQUIP_INV_BOOTS_IRON) != 0; } },
        { 18, [] { return CHECK_OWNED_EQUIP(EQUIP_TYPE_TUNIC, EQUIP_INV_TUNIC_ZORA) != 0; } },
        { 65, [] { return CHECK_OWNED_EQUIP(EQUIP_TYPE_TUNIC, EQUIP_INV_TUNIC_GORON) != 0; } },
        { 70, [] { return CHECK_OWNED_EQUIP(EQUIP_TYPE_SWORD, EQUIP_INV_SWORD_BIGGORON) != 0; } },
        { 23, [] { return CHECK_QUEST_ITEM(QUEST_GERUDO_CARD) != 0; } },
        { 25, [] { return CUR_UPG_VALUE(UPG_STRENGTH) >= 2; } },
        { 77, [] { return gSaveContext.isDoubleDefenseAcquired != 0; } },
        // Items
        { 48, [] { return HAS_ITEM(ITEM_BOMBCHU); } },
        { 49, [] { return HAS_ITEM(ITEM_DINS_FIRE); } },
        { 51, [] { return HAS_ITEM(ITEM_BEAN); } },
        { 53, [] { return HAS_ITEM(ITEM_FARORES_WIND); } },
        { 58, [] { return HAS_ITEM(ITEM_HOOKSHOT) || HAS_ITEM(ITEM_LONGSHOT); } },
        { 67, [] { return HAS_ITEM(ITEM_ARROW_FIRE); } },
        { 69, [] { return HAS_ITEM(ITEM_LENS); } },
        { 73, [] { return HAS_ITEM(ITEM_NAYRUS_LOVE); } },
        { 63,
          [] {
              return gSaveContext.inventory.items[SLOT_BOTTLE_1] != ITEM_NONE &&
                     gSaveContext.inventory.items[SLOT_BOTTLE_2] != ITEM_NONE &&
                     gSaveContext.inventory.items[SLOT_BOTTLE_3] != ITEM_NONE &&
                     gSaveContext.inventory.items[SLOT_BOTTLE_4] != ITEM_NONE;
          } },
        { 74,
          [] {
              for (int q = QUEST_SONG_MINUET; q <= QUEST_SONG_STORMS; q++) {
                  if (!CHECK_QUEST_ITEM(q)) {
                      return false;
                  }
              }
              return true;
          } },
        { 75, [] { return gSaveContext.healthCapacity >= 20 * 16; } },
        // Gold Skulltula tokens (counts collected tokens)
        { 78, [] { return gSaveContext.inventory.gsTokens >= 10; } },
        { 79, [] { return gSaveContext.inventory.gsTokens >= 20; } },
        { 80, [] { return gSaveContext.inventory.gsTokens >= 30; } },
        { 81, [] { return gSaveContext.inventory.gsTokens >= 40; } },
        { 82, [] { return gSaveContext.inventory.gsTokens >= 50; } },
        { 83, [] { return gSaveContext.inventory.gsTokens >= 100; } },
        // Bonus
        { 98,
          [] {
              return CHECK_OWNED_EQUIP(EQUIP_TYPE_SWORD, EQUIP_INV_SWORD_KOKIRI) ||
                     CHECK_OWNED_EQUIP(EQUIP_TYPE_SWORD, EQUIP_INV_SWORD_MASTER) ||
                     CHECK_OWNED_EQUIP(EQUIP_TYPE_SWORD, EQUIP_INV_SWORD_BIGGORON);
          } },
    };
    return polls;
}

// Achievements unlocked by game hooks (boss fights)
static bool IsEventDriven(int num) {
    return num == 1 || num == 6;
}

static bool IsSupported(int num) {
    return IsEventDriven(num) || GetPolls().count(num) > 0;
}

// Keep the old key for the bonus achievement so earlier unlocks are not lost.
static std::string CvarName(int num) {
    return num == 98 ? "gAchievements.first_blade" : "gAchievements.ach_" + std::to_string(num);
}

// Achievements waiting to be shown by the ToastWindow (front = currently on screen).
static std::vector<const Def*> sToastQueue;

bool IsUnlocked(int num) {
    return CVarGetInteger(CvarName(num).c_str(), 0) != 0;
}

void Unlock(int num) {
    const Def* def = FindDef(num);
    if (def == nullptr || IsUnlocked(num)) {
        return;
    }
    CVarSetInteger(CvarName(num).c_str(), static_cast<int32_t>(std::time(nullptr)));
    Ship::Context::GetRawInstance()->GetWindow()->GetGui()->SaveConsoleVariablesNextFrame();

    sToastQueue.push_back(def);
}

void ResetAll() {
    for (const auto& d : GetDefs()) {
        CVarClear(CvarName(d.num).c_str());
    }
    Ship::Context::GetRawInstance()->GetWindow()->GetGui()->SaveConsoleVariablesNextFrame();
}

// ---------------------------------------------------------------------------------------------
// Icons
// ---------------------------------------------------------------------------------------------

// Icons are loaded the first time they are needed. Names: ACH_001, ACH_001_LOCK ...
static ImTextureID GetIcon(const Def& d, bool unlocked) {
    auto gui = std::dynamic_pointer_cast<Fast::Fast3dGui>(Ship::Context::GetRawInstance()->GetWindow()->GetGui());
    static std::unordered_set<std::string> loaded;

    char buf[48];
    snprintf(buf, sizeof(buf), unlocked ? "ACH_%03d" : "ACH_%03d_LOCK", d.num);
    std::string name = buf;

    if (loaded.insert(name).second) {
        if (d.num == 98) {
            // The bonus achievement has no custom icon: use the in-game Kokiri Sword (dimmed when locked)
            gui->LoadGuiTexture(name, "__OTR__textures/icon_item_static/gItemIconSwordKokiriTex", "",
                                unlocked ? ImVec4(1, 1, 1, 1) : ImVec4(0.3f, 0.3f, 0.3f, 1));
        } else {
            snprintf(buf, sizeof(buf), unlocked ? "__OTR__textures/achievements/ach%03d"
                                                : "__OTR__textures/achievements/ach%03d_lock",
                     d.num);
            gui->LoadGuiTexture(name, buf, "", ImVec4(1, 1, 1, 1));
        }
    }
    return gui->GetTextureByName(name);
}

// ---------------------------------------------------------------------------------------------
// Window (list of all achievements)
// ---------------------------------------------------------------------------------------------

void Window::DrawElement() {
    int earned = 0;
    int total = 0;
    int earnedPts = 0;
    int totalPts = 0;
    int supported = 0;
    for (const auto& d : GetDefs()) {
        total++;
        totalPts += d.points;
        if (IsSupported(d.num)) {
            supported++;
        }
        if (IsUnlocked(d.num)) {
            earned++;
            earnedPts += d.points;
        }
    }

    ImGui::Text("%d / %d  (%d / %d pts)", earned, total, earnedPts, totalPts);
    ImGui::ProgressBar(total ? static_cast<float>(earned) / total : 0.0f, ImVec2(-1, 0));
    ImGui::TextDisabled("Detection implemented for %d of %d achievements", supported, total);

    static bool sHideLocked = false;
    static bool sOnlySupported = false;
    ImGui::Checkbox("Hide locked", &sHideLocked);
    ImGui::SameLine();
    ImGui::Checkbox("Only detectable", &sOnlySupported);
    ImGui::Separator();

    const float iconSize = ImGui::GetFontSize() * 2.8f;

    for (const auto& d : GetDefs()) {
        bool unlocked = IsUnlocked(d.num);
        bool supportedNow = IsSupported(d.num);
        if ((sHideLocked && !unlocked) || (sOnlySupported && !supportedNow)) {
            continue;
        }

        ImGui::PushID(d.num);
        ImGui::Image(GetIcon(d, unlocked), ImVec2(iconSize, iconSize));
        ImGui::SameLine();
        ImGui::BeginGroup();
        ImVec4 titleColor = unlocked ? ImVec4(1.0f, 0.8f, 0.2f, 1.0f) : ImVec4(0.7f, 0.7f, 0.7f, 1.0f);
        ImGui::TextColored(titleColor, "%s (%d)", d.nameTH, d.points);
        ImGui::TextWrapped("%s", d.descTH);
        ImGui::TextDisabled("%s", d.nameEN);
        if (unlocked) {
            time_t t = static_cast<time_t>(CVarGetInteger(CvarName(d.num).c_str(), 0));
            char buf[32] = "";
            std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M", std::localtime(&t));
            ImGui::TextColored(ImVec4(0.4f, 0.9f, 0.4f, 1.0f), "%s", buf);
        } else if (!supportedNow) {
            ImGui::TextDisabled("(not detectable yet)");
        }
        ImGui::EndGroup();
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
// Toast (GTA style card, top center)
// ---------------------------------------------------------------------------------------------

void ToastWindow::Draw() {
    if (sToastQueue.empty()) {
        return;
    }

    static float elapsed = 0.0f;
    const float slideIn = 0.4f;
    const float hold = 5.0f;
    const float slideOut = 0.5f;
    const float total = slideIn + hold + slideOut;

    const Def* def = sToastQueue.front();
    elapsed += ImGui::GetIO().DeltaTime;
    if (elapsed >= total) {
        elapsed = 0.0f;
        sToastQueue.erase(sToastQueue.begin());
        return;
    }

    // 0 -> 1 while sliding in, 1 while holding, 1 -> 0 while sliding out
    float t = 1.0f;
    if (elapsed < slideIn) {
        t = elapsed / slideIn;
    } else if (elapsed > slideIn + hold) {
        t = 1.0f - (elapsed - slideIn - hold) / slideOut;
    }
    float ease = 1.0f - (1.0f - t) * (1.0f - t); // ease-out

    auto vp = ImGui::GetMainViewport();
    const float unit = ImGui::GetFontSize();
    const float width = unit * 22.0f;
    const float height = unit * 4.6f;
    const float iconSize = unit * 3.0f;
    const float pad = unit * 0.8f;
    const float margin = unit * 1.0f;

    float y = vp->Pos.y + margin - (1.0f - ease) * (height + margin);
    ImGui::SetNextWindowViewport(vp->ID);
    ImGui::SetNextWindowPos(ImVec2(vp->Pos.x + vp->Size.x * 0.5f, y), ImGuiCond_Always, ImVec2(0.5f, 0.0f));
    ImGui::SetNextWindowSize(ImVec2(width, height));

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.06f, 0.06f, 0.08f, 0.92f));
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.0f, 0.8f, 0.2f, 0.9f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, unit * 0.4f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 2.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(pad, pad));
    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ease);

    ImGui::Begin("AchievementToast", nullptr,
                 ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                     ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings |
                     ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoDocking |
                     ImGuiWindowFlags_NoBringToFrontOnFocus);

    // Icon (vertically centered)
    ImGui::SetCursorPos(ImVec2(pad, (height - iconSize) * 0.5f));
    ImGui::Image(GetIcon(*def, true), ImVec2(iconSize, iconSize));

    // Text block
    const float textX = pad + iconSize + pad;
    ImGui::SetCursorPos(ImVec2(textX, pad * 0.9f));
    ImGui::PushTextWrapPos(width - pad);
    ImGui::SetWindowFontScale(0.75f);
    ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "ปลดล็อกความสำเร็จ");
    ImGui::SetWindowFontScale(1.0f);
    ImGui::SetCursorPosX(textX);
    ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "%s  (%d)", def->nameTH, def->points);
    ImGui::SetCursorPosX(textX);
    ImGui::SetWindowFontScale(0.75f);
    ImGui::TextColored(ImVec4(0.75f, 0.75f, 0.75f, 1.0f), "%s", def->descTH);
    ImGui::SetWindowFontScale(1.0f);
    ImGui::PopTextWrapPos();

    ImGui::End();
    ImGui::PopStyleVar(4);
    ImGui::PopStyleColor(2);
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

    for (const auto& [num, check] : GetPolls()) {
        if (!IsUnlocked(num) && check()) {
            Unlock(num);
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
            Unlock(1); // Spider Net
        }
    });

    // King Dodongo
    GameInteractor::Instance->RegisterGameHookForID<GameInteractor::OnBossDefeat>(ACTOR_BOSS_DODONGO,
                                                                                  [](void* refActor) {
        if (!sDodongoBombUsed) {
            Unlock(6); // Bombalicious
        }
    });
}

static RegisterShipInitFunc initFunc(RegisterAchievementHooks);

} // namespace Achievements

#include "SohMenu.h"
#include "ThaiText.h"
#include <soh/Enhancements/enhancementTypes.h>
#include "soh/Enhancements/SwitchAge.h"
#include "soh/Enhancements/AdultMasks.h"
#include "soh/Enhancements/BunnyHood.h"
#include "soh/Enhancements/FileSelectEnhancements.h"
#include <soh/Enhancements/game-interactor/GameInteractor.h>
#include <soh/OTRGlobals.h>
#include <soh/Enhancements/cosmetics/authenticGfxPatches.h>
#include <soh/Enhancements/TimeDisplay/TimeDisplay.h>
#include "soh/Enhancements/Restorations/GetItemManipulation.h"
#include "soh/Enhancements/randomizer/SeedContext.h"
#include <ship/Context.h>
#include <libultraship/bridge/consolevariablebridge.h>
#include <soh/ResourceManagerHelpers.h>
#include "soh/ShipInit.hpp"

extern "C" {
#include "functions.h"
#include "variables.h"
#include "macros.h"
extern PlayState* gPlayState;
}

#define CVAR_INT_SHIP_INIT(cvar, val) \
    CVarSetInteger(cvar, val);        \
    ShipInit::Init(cvar);

static std::string comboboxTooltip = T("");
bool isBetaQuestEnabled = false;

extern "C" {
void enableBetaQuest() {
    isBetaQuestEnabled = true;
}
void disableBetaQuest() {
    isBetaQuestEnabled = false;
}
}

namespace SohGui {

extern std::shared_ptr<SohMenu> mSohMenu;
using namespace UIWidgets;

static std::map<int32_t, const char*> bunnyHoodEffectMap = {
    { BUNNY_HOOD_VANILLA, T("Vanilla") },
    { BUNNY_HOOD_FAST, T("Faster Run") },
    { BUNNY_HOOD_FAST_AND_JUMP, T("Faster + Longer Jump") },
};

static const std::map<int32_t, const char*> dekuStickCheat = {
    { DEKU_STICK_NORMAL, T("Normal") },
    { DEKU_STICK_UNBREAKABLE, T("Unbreakable") },
    { DEKU_STICK_UNBREAKABLE_AND_ALWAYS_ON_FIRE, T("Unbreakable + Always on Fire") },
};

static const std::map<int32_t, const char*> skipForcedDialogOptions = {
    { FORCED_DIALOG_SKIP_NONE, T("None") },
    { FORCED_DIALOG_SKIP_NAVI, T("Navi") },
    { FORCED_DIALOG_SKIP_NPC, T("NPCs") },
    { FORCED_DIALOG_SKIP_ALL, T("All") },
};

static const std::map<int32_t, const char*> ingoRaceOptions = {
    { INGO_RACE_TWICE, T("Twice") },
    { INGO_RACE_ONCE, T("Once") },
    { INGO_RACE_NONE, T("None") },
};

static const std::map<int32_t, const char*> timeTravelOptions = {
    { TIME_TRAVEL_DISABLED, T("Disabled") },
    { TIME_TRAVEL_OOT, T("Ocarina of Time") },
    { TIME_TRAVEL_OOT_MS, T("Ocarina of Time + Master Sword") },
    { TIME_TRAVEL_ANY, T("Any Ocarina") },
    { TIME_TRAVEL_ANY_MS, T("Any Ocarina + Master Sword") },
};

static const std::map<int32_t, const char*> getItemManipulationOptions = {
    { GIM_DISABLED, T("Disabled") },   { GIM_NTSC_1_0, T("NTSC 1.0") },
    { GIM_NTSC_1_1, T("NTSC 1.1") },   { GIM_NTSC_1_2, T("NTSC 1.2") },
    { GIM_PAL_1_0, T("PAL 1.0") },     { GIM_PAL_1_1, T("PAL 1.1") },
    { GIM_GC_U, T("GC U") },           { GIM_GC_E, T("GC E") },
    { GIM_GC_J, T("GC J") },           { GIM_MQ_U, T("MQ U") },
    { GIM_MQ_E, T("MQ E") },           { GIM_MQ_J, T("MQ J") },
    { GIM_IQUE_CHN, T("IQUE CHN") },   { GIM_IQUE_TWN, T("IQUE TWN") },
    { GIM_MQ_DEBUG, T("MQ DEBUG") },   { GIM_MZX_NTSC, T("MZX NTSC 1.0") },
    { GIM_MZX_PAL, T("MZX PAL 1.1") },
};

static const std::map<int32_t, const char*> sleepingWaterfallOptions = {
    { WATERFALL_ALWAYS, T("Always") },
    { WATERFALL_ONCE, T("Once") },
    { WATERFALL_NEVER, T("Never") },
};

static const std::map<int32_t, const char*> bombchuBowlingFirstPrizeOptions = {
    { BOWLING_FIRST_PRIZE_RANDOM, T("Random") },
    { BOWLING_FIRST_PRIZE_BOMB_BAG, T("Bomb Bag") },
    { BOWLING_FIRST_PRIZE_PURPLE_RUPEE, T("Purple Rupee") },
    { BOWLING_FIRST_PRIZE_BOMBCHUS, T("Bombchus") },
    { BOWLING_FIRST_PRIZE_HEART_PIECE, T("Heart Piece") },
    { BOWLING_FIRST_PRIZE_BOMBS, T("Bombs") },
};

static const std::map<int32_t, const char*> allPowers = {
    { DAMAGE_VANILLA, T("Vanilla (1x)") },      { DAMAGE_DOUBLE, T("Double (2x)") },
    { DAMAGE_QUADRUPLE, T("Quadruple (4x)") },  { DAMAGE_OCTUPLE, T("Octuple (8x)") },
    { DAMAGE_FOOLISH, T("Foolish (16x)") },     { DAMAGE_RIDICULOUS, T("Ridiculous (32x)") },
    { DAMAGE_MERCILESS, T("Merciless (64x)") }, { DAMAGE_TORTURE, T("Pure Torture (128x)") },
    { DAMAGE_OHKO, T("OHKO (256x)") },
};

static const std::map<int32_t, const char*> subPowers = {
    { DAMAGE_VANILLA, T("Vanilla (1x)") },      { DAMAGE_DOUBLE, T("Double (2x)") },
    { DAMAGE_QUADRUPLE, T("Quadruple (4x)") },  { DAMAGE_OCTUPLE, T("Octuple (8x)") },
    { DAMAGE_FOOLISH, T("Foolish (16x)") },     { DAMAGE_RIDICULOUS, T("Ridiculous (32x)") },
    { DAMAGE_MERCILESS, T("Merciless (64x)") }, { DAMAGE_TORTURE, T("Pure Torture (128x)") },
};

static const std::map<int32_t, const char*> subSubPowers = {
    { DAMAGE_VANILLA, T("Vanilla (1x)") },      { DAMAGE_DOUBLE, T("Double (2x)") },
    { DAMAGE_QUADRUPLE, T("Quadruple (4x)") },  { DAMAGE_OCTUPLE, T("Octuple (8x)") },
    { DAMAGE_FOOLISH, T("Foolish (16x)") },     { DAMAGE_RIDICULOUS, T("Ridiculous (32x)") },
    { DAMAGE_MERCILESS, T("Merciless (64x)") },
};

static const std::map<int32_t, const char*> bonkDamageValues = {
    { BONK_DAMAGE_NONE, T("No Damage") },        { BONK_DAMAGE_QUARTER_HEART, T("0.25 Hearts") },
    { BONK_DAMAGE_HALF_HEART, T("0.5 Hearts") }, { BONK_DAMAGE_1_HEART, T("1 Heart") },
    { BONK_DAMAGE_2_HEARTS, T("2 Hearts") },     { BONK_DAMAGE_4_HEARTS, T("4 Hearts") },
    { BONK_DAMAGE_8_HEARTS, T("8 Hearts") },     { BONK_DAMAGE_OHKO, T("OHKO") },
};

static const std::map<int32_t, const char*> dampeDropRates = {
    { DAMPE_NONE, T("None") },
    { DAMPE_NORMAL, T("Vanilla") },
    { DAMPE_JALAPENO, T("Jalapeño") },
    { DAMPE_CHIPOTLE, T("Serrano") },
    { DAMPE_SCOTCH_BONNET, T("Habanero") },
    { DAMPE_GHOST_PEPPER, T("Ghost Pepper") },
    { DAMPE_INFERNO, T("Dampe's Inferno") },
};

static const std::map<int32_t, const char*> cursorAnywhereValues = {
    { PAUSE_ANY_CURSOR_RANDO_ONLY, T("Only in Rando") },
    { PAUSE_ANY_CURSOR_ALWAYS_ON, T("Always") },
    { PAUSE_ANY_CURSOR_ALWAYS_OFF, T("Never") },
};

static const std::map<int32_t, const char*> zFightingOptions = {
    { ZFIGHT_FIX_DISABLED, T("Disabled") },
    { ZFIGHT_FIX_CONSISTENT_VANISH, T("Consistent Vanish") },
    { ZFIGHT_FIX_NO_VANISH, T("No Vanish") },
};

static const std::map<int32_t, const char*> teleportTrapModes = {
    { TELEPORT_TRAP_OFF, T("Off") },
    { TELEPORT_TRAP_SIMPLE, T("Simple") },
    { TELEPORT_TRAP_ADVANCED, T("Advanced") },
};

static const std::map<int32_t, const char*> swordToggleModes = {
    { SWORD_TOGGLE_NONE, T("None") },
    { SWORD_TOGGLE_CHILD, T("Child Toggle") },
    { SWORD_TOGGLE_BOTH_AGES, T("Both Ages") },
};

static const std::map<int32_t, const char*> mirroredWorldModes = {
    { MIRRORED_WORLD_OFF, T("Disabled") },
    { MIRRORED_WORLD_ALWAYS, T("Always") },
    { MIRRORED_WORLD_RANDOM, T("Random") },
    { MIRRORED_WORLD_RANDOM_SEEDED, T("Random (Seeded)") },
    { MIRRORED_WORLD_DUNGEONS_ALL, T("Dungeons") },
    { MIRRORED_WORLD_DUNGEONS_VANILLA, T("Dungeons (Vanilla)") },
    { MIRRORED_WORLD_DUNGEONS_MQ, T("Dungeons (MQ)") },
    { MIRRORED_WORLD_DUNGEONS_RANDOM, T("Dungeons Random") },
    { MIRRORED_WORLD_DUNGEONS_RANDOM_SEEDED, T("Dungeons Random (Seeded)") },
};

// Disables a File Select "Hide" checkbox when its O2R is missing, or when it would hide the last visible quest
static WidgetFunc HideQuestPreFunc(Quest quest) {
    return [quest](WidgetInfo& info) {
        if ((quest == QUEST_NORMAL || quest == QUEST_SPEEDRUN) && !ResourceMgr_GameHasOriginal()) {
            info.options->disabled = true;
            info.options->disabledTooltip = T("This option requires a loaded original O2R.");
        } else if ((quest == QUEST_MASTER || quest == QUEST_SPEEDRUN_MASTER) && !ResourceMgr_GameHasMasterQuest()) {
            info.options->disabled = true;
            info.options->disabledTooltip = T("This option requires a loaded Master Quest O2R.");
        } else if (!SohFileSelect_IsQuestHidden(quest) && SohFileSelect_CountVisibleQuests() <= 1) {
            info.options->disabled = true;
            info.options->disabledTooltip = T("At least one quest type must remain visible.");
        }
    };
}

void SohMenu::AddMenuEnhancements() {
    // Add Enhancements Menu
    AddMenuEntry("Enhancements", CVAR_SETTING("Menu.EnhancementsSidebarSection"));

    // Quality of Life
    WidgetPath path = { "Enhancements", "Quality of Life", SECTION_COLUMN_1 };
    AddSidebarEntry("Enhancements", path.sidebarName, 3);
    path.column = SECTION_COLUMN_1;

    AddWidget(path, T("Saving"), WIDGET_SEPARATOR_TEXT);
    AddWidget(path, T("Autosave"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("Autosave"))
        .RaceDisable(false)
        .Options(CheckboxOptions().Tooltip(
            T("Save the game automatically on a 3 minute interval and when soft-resetting the game. The interval "
            "autosave will wait if the game is paused in any way (dialogue, pause screen up, cutscenes, "
            "etc.).\n\n"
            "The soft-reset save will *not* trigger in cutscene maps like the Chamber of Sages!")));
    AddWidget(path, T("Notification on Autosave"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("AutosaveNotification"))
        .PreFunc([](WidgetInfo& info) { info.isHidden = !CVarGetInteger(CVAR_ENHANCEMENT("Autosave"), 0); })
        .RaceDisable(false)
        .Options(CheckboxOptions().DefaultValue(true).Tooltip(T("Show a notification when the game is autosaved.")));

    AddWidget(path, T("Remember Save Location"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("RememberSaveLocation"))
        .Options(CheckboxOptions().Tooltip(
            T("When loading a save, places Link at the last entrance he went through.\n"
            "This doesn't work if the save was made in grottos, fairy fountains, or dungeons.")));

    AddWidget(path, T("Better Save Menu"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("BetterSaveMenu"))
        .Options(CheckboxOptions().Tooltip(
            T("Replaces the authentic save menu with a textbox that asks Yes or No for saving,\n"
            "then asks if you want to Continue, Reset, or Reset to Spawn.")));

    AddWidget(path, T("Containers Match Contents"), WIDGET_SEPARATOR_TEXT);
    AddWidget(path, T("Containers Match Contents"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("ChestSizeAndTextureMatchContents"))
        .Callback([](WidgetInfo& info) {
            if (!CVarGetInteger(CVAR_ENHANCEMENT("ChestSizeAndTextureMatchContents"), 0)) {
                CVarSetInteger(CVAR_ENHANCEMENT("ChestSizeDependsStoneOfAgony"), 0);
            }
        })
        .Options(CheckboxOptions().DefaultValue(false).Tooltip(
            T("Toggle to change container textures to match their contents in randomizer games.\n"
            "Categories: Major items, Lesser items, Junk items, Small keys, Boss keys, Skulltula Tokens.")));
    AddWidget(path, T("Containers of Agony"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("ChestSizeDependsStoneOfAgony"))
        .PreFunc([](WidgetInfo& info) {
            info.isHidden = !CVarGetInteger(CVAR_ENHANCEMENT("ChestSizeAndTextureMatchContents"), 0);
        })
        .Options(CheckboxOptions().Tooltip(T("Only change the texture of containers if you have the Stone of Agony.")));

    AddWidget(path, T("Time of Day"), WIDGET_SEPARATOR_TEXT);
    AddWidget(path, T("Nighttime GS Always Spawn"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("NightGSAlwaysSpawn"))
        .Options(CheckboxOptions().Tooltip(T("Nighttime Skulltulas will spawn during both day and night.")));
    AddWidget(path, T("Pull Grave During the Day"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("DayGravePull"))
        .Options(CheckboxOptions().Tooltip(T("Allows graves to be pulled when child during the day.")));
    AddWidget(path, T("Dampe Appears All Night"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("DampeAllNight"))
        .Options(CheckboxOptions().Tooltip(
            T("Makes Dampe appear anytime during the night, not just his usual working hours.")));
    AddWidget(path, T("Exit Market at Night"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("MarketSneak"))
        .Options(CheckboxOptions().Tooltip(
            T("Allows exiting Hyrule Castle Market Town to Hyrule Field at night by speaking to the guard "
            "next to the gate.")));
    AddWidget(path, T("Shops and Games Always Open"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("OpenAllHours"))
        .PreFunc([](WidgetInfo& info) {
            info.options->disabled =
                IS_RANDO && OTRGlobals::Instance->gRandoContext->GetOption(RSK_LOCK_OVERWORLD_DOORS).Is(RO_GENERIC_ON);
        })
        .Options(
            CheckboxOptions()
                .Tooltip(T("Shops and Minigames are open both day and night. Requires a scene reload to take effect."))
                .DisabledTooltip(T("This is not compatible with the Locked Overworld Doors Randomizer option.")));

    AddWidget(path, T("Pause Menu"), WIDGET_SEPARATOR_TEXT);
    AddWidget(path, T("Allow the Cursor to be on Any Slot"), WIDGET_CVAR_COMBOBOX)
        .CVar(CVAR_ENHANCEMENT("PauseAnyCursor"))
        .Options(
            ComboboxOptions()
                .ComboMap(cursorAnywhereValues)
                .DefaultIndex(PAUSE_ANY_CURSOR_RANDO_ONLY)
                .Tooltip(T("Allows the cursor on the pause menu to be over any slot. Sometimes required in Randomizer "
                         "to select certain items.")));
    AddWidget(path, T("Pause Warp"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("PauseWarp"))
        .Options(CheckboxOptions().Tooltip(
            T("Selection of warp song in pause menu initiates a warp. Disables song playback.")));

    path.column = SECTION_COLUMN_2;
    AddWidget(path, T("Controls"), WIDGET_SEPARATOR_TEXT);
    AddWidget(path, T("Answer Navi Prompt with L Button"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("NaviOnL"))
        .RaceDisable(false)
        .Options(CheckboxOptions().Tooltip(T("Speak to Navi with L but enter First-Person Camera with C-Up.")));
    AddWidget(path, T("Don't Require Input for Credits Sequence"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("NoInputForCredits"))
        .RaceDisable(false)
        .Options(CheckboxOptions().Tooltip(
            T("Removes the Input Requirement on Text boxes after defeating Ganon, allowing the Credits "
            "Sequence to continue to progress.")));
    AddWidget(path, T("Include Held Inputs at the Start of Pause Buffer Input Window"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("IncludeHeldInputsBufferWindow"))
        .Options(CheckboxOptions().Tooltip(
            T("Typically, inputs that are held prior to the buffer window are not included in the buffer. This "
            "setting changes that behavior to include them. This may cause some inputs to be re-triggered "
            "undesireably, for instance Z-Targeting something you might not want to.")));
    AddWidget(path, T("Pause Buffer Input Window: %d frames"), WIDGET_CVAR_SLIDER_INT)
        .CVar(CVAR_ENHANCEMENT("PauseBufferWindow"))
        .Options(IntSliderOptions()
                     .Min(0)
                     .Max(40)
                     .DefaultValue(0)
                     .Format(T("%d frames"))
                     .Tooltip(T("Adds back in a delay after unpausing before the game resumes playing again, "
                              "where inputs can be held prematurely to be input immediately after the game resumes. "
                              "This essentially brings back behaviour from console releases which are lost on default "
                              "because SoH isn't limited to N64 hardware.")));
    AddWidget(path, T("Simulated Input Lag: %d frames"), WIDGET_CVAR_SLIDER_INT)
        .CVar(CVAR_SIMULATED_INPUT_LAG)
        .Options(IntSliderOptions()
                     .Min(0)
                     .Max(6)
                     .DefaultValue(0)
                     .Format(T("%d frames"))
                     .Tooltip(T("Buffers your inputs to be executed a specified amount of frames later.")));
    AddWidget(path, T("Reworked Targeting"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("ReworkedTargeting.Enabled"))
        .Options(CheckboxOptions().Tooltip(T("Reworks targeting functionality\n"
                                           "- Press Z while locked always untargets (in Toggle mode)\n"
                                           "- Use the configured button combo to switch between targets")));
    AddWidget(path, T("Target Switch Button Combination:"), WIDGET_CVAR_BTN_SELECTOR)
        .PreFunc([](WidgetInfo& info) {
            info.isHidden = CVarGetInteger(CVAR_ENHANCEMENT("ReworkedTargeting.Enabled"), 0) == 0;
        })
        .CVar(CVAR_ENHANCEMENT("ReworkedTargeting.Btn"))
        .Options(BtnSelectorOptions().Tooltip(T("Buttons to activate target switching.")));

    AddWidget(path, T("Item Count Messages"), WIDGET_SEPARATOR_TEXT);
    AddWidget(path, T("Gold Skulltula Tokens"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("InjectItemCounts.GoldSkulltula"))
        .RaceDisable(false);
    AddWidget(path, T("Pieces of Heart"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("InjectItemCounts.HeartPiece"))
        .RaceDisable(false);
    AddWidget(path, T("Heart Containers"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("InjectItemCounts.HeartContainer"))
        .RaceDisable(false);

    path.column = SECTION_COLUMN_3;
    AddWidget(path, T("Misc"), WIDGET_SEPARATOR_TEXT);
    AddWidget(path, T("Disable Crit Wiggle"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("DisableCritWiggle"))
        .RaceDisable(false)
        .Options(CheckboxOptions().Tooltip(T("Disable Random Camera Wiggle at Low Health.")));
    AddWidget(path, T("Better Owl"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("BetterOwl"))
        .Options(CheckboxOptions().Tooltip(
            T("The default response to Kaepora Gaebora is always that you understood what he said.")));
    AddWidget(path, T("Easy Butterfly Fairies"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("EasyButterflyFairies"))
        .PreFunc([](WidgetInfo& info) {
            info.options->disabled =
                IS_RANDO &&
                OTRGlobals::Instance->gRandoContext->GetOption(RSK_SHUFFLE_BUTTERFLY_FAIRIES).Is(RO_GENERIC_ON);
            info.options->disabledTooltip = T("This setting is forcefully enabled because a randomizer savefile with "
                                            "\"Butterfly Fairies Shuffle\" is loaded.");
        })
        .Options(CheckboxOptions().Tooltip(
            T("Butterflies will transform into a fairy as soon as you approach them with a Deku Stick, "
            "skipping the need to stand still and let the butterfly land on your stick.")));

    AddWidget(path, T("Allow Dropping Throw-Only Objects"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("DropThrowOnlyObjects"))
        .Options(CheckboxOptions().Tooltip(T("Allows normally throw-only objects (such as Cuccos, pots, grass, and small "
                                           "rocks) to be dropped by pressing A while standing still. Can be toggled "
                                           "while holding an object.")));

    AddWidget(path, T("Convenience"), WIDGET_SEPARATOR_TEXT);
    AddWidget(path, T("Quit Fishing at Door"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("QuitFishingAtDoor"))
        .Options(CheckboxOptions().Tooltip(
            T("Fisherman asks if you want to quit at the door if you try to leave the Fishing Pond "
            "while still holding the Fishing Rod.")));
    AddWidget(path, T("Instant Putaway"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("InstantPutaway"))
        .Options(CheckboxOptions().Tooltip(T("Allow Link to put items away without having to wait around.")));
    AddWidget(path, T("Navi Timer Resets on Scene Change"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("ResetNaviTimer"))
        .Options(
            CheckboxOptions().Tooltip(T("Resets the Navi timer on scene change. If you have already talked to her, "
                                      "she will try and talk to you again, instead of needing a save warp or death.")));
    AddWidget(path, T("Link's Cow in Both Time Periods"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("CowOfTime"))
        .Options(CheckboxOptions().Tooltip(
            T("Allows the Lon Lon Ranch Obstacle Course reward to be shared across time periods.")));
    AddWidget(path, T("Play Zelda's Lullaby to Open Sleeping Waterfall"), WIDGET_CVAR_COMBOBOX)
        .CVar(CVAR_ENHANCEMENT("TimeSavers.SleepingWaterfall"))
        .PreFunc([](WidgetInfo& info) {
            info.options->disabled =
                IS_RANDO &&
                OTRGlobals::Instance->gRandoContext->GetOption(RSK_SLEEPING_WATERFALL).Is(RO_WATERFALL_OPEN);
            info.options->disabledTooltip = T("This setting is forcefully enabled because a randomizer savefile with "
                                            "\"Sleeping Waterfall: Open\" is loaded.");
        })
        .Options(
            ComboboxOptions()
                .ComboMap(sleepingWaterfallOptions)
                .DefaultIndex(WATERFALL_ALWAYS)
                .Tooltip(
                    T("Always: Link must always play Zelda's Lullaby to open the waterfall entrance to Zora's Domain.\n"
                    "Once: Link only needs to play Zelda's Lullaby once to open the waterfall; after that, it stays "
                    "open permanently.\n"
                    "Never: Link never needs to play Zelda's Lullaby to open the waterfall. He only needs to have "
                    "learned it and have an Ocarina.")));
    AddWidget(path, T("Skip Feeding Jabu-Jabu"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("TimeSavers.SkipJabuJabuFish"))
        .PreFunc([](WidgetInfo& info) {
            info.options->disabled = IS_RANDO;
            info.options->disabledTooltip =
                T("This setting is disabled because a randomizer savefile with \"Jabu-Jabu: Open\" is loaded.");
        })
        .Options(CheckboxOptions().Tooltip(T("Allow Link to enter Jabu-Jabu without feeding him a fish.")));

    // Skips & Speed-ups
    path.sidebarName = "Skips & Speed-ups";
    AddSidebarEntry("Enhancements", path.sidebarName, 3);
    path.column = SECTION_COLUMN_1;

    AddWidget(path, T("Cutscenes"), WIDGET_SEPARATOR_TEXT);
    AddWidget(path, T("All##Skips"), WIDGET_BUTTON)
        .Options(ButtonOptions().Size(Sizes::Inline))
        .Callback([](WidgetInfo& info) {
            CVAR_INT_SHIP_INIT(CVAR_ENHANCEMENT("TimeSavers.SkipCutscene.Intro"), true);
            CVAR_INT_SHIP_INIT(CVAR_ENHANCEMENT("TimeSavers.SkipCutscene.Entrances"), true);
            CVAR_INT_SHIP_INIT(CVAR_ENHANCEMENT("TimeSavers.SkipCutscene.Story"), true);
            CVAR_INT_SHIP_INIT(CVAR_ENHANCEMENT("TimeSavers.SkipCutscene.LearnSong"), true);
            CVAR_INT_SHIP_INIT(CVAR_ENHANCEMENT("WarpSongSkipAnimation"), true);
            CVAR_INT_SHIP_INIT(CVAR_ENHANCEMENT("TimeSavers.SkipCutscene.BossIntro"), true);
            CVAR_INT_SHIP_INIT(CVAR_ENHANCEMENT("TimeSavers.SkipCutscene.QuickBossDeaths"), true);
            CVAR_INT_SHIP_INIT(CVAR_ENHANCEMENT("TimeSavers.SkipCutscene.OnePoint"), true);
            CVAR_INT_SHIP_INIT(CVAR_ENHANCEMENT("TimeSavers.SkipOwlInteractions"), true);
            CVAR_INT_SHIP_INIT(CVAR_ENHANCEMENT("TimeSavers.SkipMiscInteractions"), true);
            CVAR_INT_SHIP_INIT(CVAR_ENHANCEMENT("TimeSavers.DisableTitleCard"), true);

            Ship::Context::GetRawInstance()->GetWindow()->GetGui()->SaveConsoleVariablesNextFrame();
        });
    AddWidget(path, T("None##Skips"), WIDGET_BUTTON)
        .SameLine(true)
        .Options(ButtonOptions().Size(Sizes::Inline))
        .Callback([](WidgetInfo& info) {
            CVAR_INT_SHIP_INIT(CVAR_ENHANCEMENT("TimeSavers.SkipCutscene.Intro"), false);
            CVAR_INT_SHIP_INIT(CVAR_ENHANCEMENT("TimeSavers.SkipCutscene.Entrances"), false);
            CVAR_INT_SHIP_INIT(CVAR_ENHANCEMENT("TimeSavers.SkipCutscene.Story"), false);
            CVAR_INT_SHIP_INIT(CVAR_ENHANCEMENT("TimeSavers.SkipCutscene.LearnSong"), false);
            CVAR_INT_SHIP_INIT(CVAR_ENHANCEMENT("WarpSongSkipAnimation"), false);
            CVAR_INT_SHIP_INIT(CVAR_ENHANCEMENT("TimeSavers.SkipCutscene.BossIntro"), false);
            CVAR_INT_SHIP_INIT(CVAR_ENHANCEMENT("TimeSavers.SkipCutscene.QuickBossDeaths"), false);
            CVAR_INT_SHIP_INIT(CVAR_ENHANCEMENT("TimeSavers.SkipCutscene.OnePoint"), false);
            CVAR_INT_SHIP_INIT(CVAR_ENHANCEMENT("TimeSavers.SkipOwlInteractions"), false);
            CVAR_INT_SHIP_INIT(CVAR_ENHANCEMENT("TimeSavers.SkipMiscInteractions"), false);
            CVAR_INT_SHIP_INIT(CVAR_ENHANCEMENT("TimeSavers.DisableTitleCard"), false);

            Ship::Context::GetRawInstance()->GetWindow()->GetGui()->SaveConsoleVariablesNextFrame();
        });
    AddWidget(path, T("Skip Intro"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("TimeSavers.SkipCutscene.Intro"))
        .Options(CheckboxOptions().DefaultValue(IS_RANDO));
    AddWidget(path, T("Skip Entrance Cutscenes"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("TimeSavers.SkipCutscene.Entrances"))
        .Options(CheckboxOptions().DefaultValue(IS_RANDO));
    AddWidget(path, T("Skip Story Cutscenes"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("TimeSavers.SkipCutscene.Story"))
        .Options(CheckboxOptions().DefaultValue(IS_RANDO));
    AddWidget(path, T("Skip Song Cutscenes"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("TimeSavers.SkipCutscene.LearnSong"))
        .Options(CheckboxOptions().DefaultValue(IS_RANDO));
    AddWidget(path, T("Skip Warp Cutscenes"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("WarpSongSkipAnimation"))
        .Options(CheckboxOptions().DefaultValue(IS_RANDO).Tooltip(
            T("Warp songs skip the departure and arrival cutscenes, fading immediately to the destination.")));
    AddWidget(path, T("Skip Boss Introductions"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("TimeSavers.SkipCutscene.BossIntro"))
        .Options(CheckboxOptions().DefaultValue(IS_RANDO));
    AddWidget(path, T("Quick Boss Deaths"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("TimeSavers.SkipCutscene.QuickBossDeaths"))
        .Options(CheckboxOptions().DefaultValue(IS_RANDO));
    AddWidget(path, T("Skip One Point Cutscenes (Chests, Door Unlocks, etc.)"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("TimeSavers.SkipCutscene.OnePoint"))
        .Options(CheckboxOptions().DefaultValue(IS_RANDO));
    AddWidget(path, T("Skip Owl Interactions"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("TimeSavers.SkipOwlInteractions"))
        .Options(CheckboxOptions().DefaultValue(IS_RANDO));
    AddWidget(path, T("Skip Misc Interactions"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("TimeSavers.SkipMiscInteractions"))
        .Options(CheckboxOptions().DefaultValue(IS_RANDO));
    AddWidget(path, T("Disable Title Card"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("TimeSavers.DisableTitleCard"))
        .Options(CheckboxOptions().DefaultValue(IS_RANDO));
    AddWidget(path, T("Exclude Glitch-Aiding Cutscenes"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("TimeSavers.SkipCutscene.GlitchAiding"))
        .Options(
            CheckboxOptions().Tooltip(T("Don't skip cutscenes that are associated with useful glitches. Currently, it is "
                                      "only the Fire Temple Darunia CS, Forest Temple Poe Sisters CS, Dodongo Boss "
                                      "Door Switch CS, Water Temple Dragon Switch CS, the Box Skip One Point in Jabu, "
                                      "Early Hammer Switch CS in MQ Spirit, and Cow Switch Chest CS in MQ Jabu.")));

    AddWidget(path, T("Text"), WIDGET_SEPARATOR_TEXT);
    AddWidget(path, T("Skip Bottle Pickup Messages"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("FastBottles"))
        .Options(CheckboxOptions().Tooltip(T("Skip Pickup Messages for Bottle Swipes.")));
    AddWidget(path, T("Skip Consumable Item Pickup Messages"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("FastDrops"))
        .Options(CheckboxOptions().Tooltip(T("Skip Pickup Messages for new Consumable Items.")));
    AddWidget(path, T("Skip Forced Dialog"), WIDGET_CVAR_COMBOBOX)
        .CVar(CVAR_ENHANCEMENT("TimeSavers.SkipForcedDialog"))
        .Options(ComboboxOptions()
                     .ComboMap(skipForcedDialogOptions)
                     .DefaultIndex(FORCED_DIALOG_SKIP_NONE)
                     .Tooltip(T("Prevent forced conversations with Navi and/or other NPCs.")));
    AddWidget(path, T("Skip Text"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("SkipText"))
        .Options(CheckboxOptions().Tooltip(T("Holding down B skips text.")));
    AddWidget(path, T("Text Speed"), WIDGET_CVAR_SLIDER_INT)
        .CVar(CVAR_ENHANCEMENT("TextSpeed"))
        // Top notch (max) fills the whole text box in one frame
        .Callback([](WidgetInfo& info) {
            auto options = std::static_pointer_cast<IntSliderOptions>(info.options);
            options->format = CVarGetInteger(info.cVar, 1) >= options->max ? "Instant" : "%dx";
        })
        .Options(IntSliderOptions().Min(1).Max(6).DefaultValue(1).Format(
            CVarGetInteger(CVAR_ENHANCEMENT("TextSpeed"), 1) >= 6 ? "Instant" : "%dx"));
    AddWidget(path, T("Slow Text Speed: %dx"), WIDGET_CVAR_SLIDER_INT)
        .CVar(CVAR_ENHANCEMENT("SlowTextSpeed"))
        .Options(IntSliderOptions().Min(1).Max(5).DefaultValue(1).Format("%dx").Tooltip(
            T("Changes the speed of sections of text that normally are paced slower than the text surrounding it.")));

    path.column = SECTION_COLUMN_2;
    AddWidget(path, T("Animations"), WIDGET_SEPARATOR_TEXT);
    AddWidget(path, T("Faster Heavy Block Lift"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("FasterHeavyBlockLift"))
        .Options(CheckboxOptions().Tooltip(T("Speeds up lifting Silver Rocks and Obelisks.")));
    AddWidget(path, T("Faster Shadow Ship"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("FasterShadowShip"))
        .Options(CheckboxOptions().Tooltip(T("Speeds up ship in Shadow Temple.")));
    AddWidget(path, T("Fast Chests"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("FastChests"))
        .Options(CheckboxOptions().Tooltip(T("Makes Link always kick the chest to open it, instead of doing the longer "
                                           "chest opening animation for major items.")));
    AddWidget(path, T("Improved Roll"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("ImprovedRoll"))
        .Options(CheckboxOptions().Tooltip(
            T("Allows Link to chain a new roll by pressing A during a roll, maintaining maximum roll speed.")));
    AddWidget(path, T("Improved Roll Steering"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("ImprovedRollSteering"))
        .PreFunc([](WidgetInfo& info) { info.isHidden = !CVarGetInteger(CVAR_ENHANCEMENT("ImprovedRoll"), 0); })
        .Options(CheckboxOptions().Tooltip(
            T("Allows slight directional steering with the control stick while rolling. "
            "Steering is automatically disabled while Z is held, preserving Z-target roll glitch setups.")));
    AddWidget(path, T("Skip Water Take Breath Animation"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("SkipSwimDeepEndAnim"))
        .Options(CheckboxOptions().Tooltip(T("Skips Link's taking breath animation after coming up from water. "
                                           "This setting does not interfere with getting items from underwater.")));
    AddWidget(path, T("Empty Bottles Faster"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("FasterBottleEmpty"))
        .Options(CheckboxOptions().Tooltip(T("Speeds up emptying animation when dumping out the contents of a bottle.")));
    AddWidget(path, T("Vine/Ladder Climb Speed +%d"), WIDGET_CVAR_SLIDER_INT)
        .CVar(CVAR_ENHANCEMENT("ClimbSpeed"))
        .Options(IntSliderOptions().Min(0).Max(12).DefaultValue(0).Format("+%d"));
    AddWidget(path, T("Block Pushing Speed +%d"), WIDGET_CVAR_SLIDER_INT)
        .CVar(CVAR_ENHANCEMENT("FasterBlockPush"))
        .Options(IntSliderOptions().Min(0).Max(5).DefaultValue(0).Format("+%d"));
    AddWidget(path, T("Crawl Speed %dx"), WIDGET_CVAR_SLIDER_INT)
        .CVar(CVAR_ENHANCEMENT("CrawlSpeed"))
        .Options(IntSliderOptions().Min(1).Max(5).DefaultValue(1).Format("%dx"));
    AddWidget(path, T("Exclude Glitch-Aiding Crawlspaces"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("GlitchAidingCrawlspaces"))
        .PreFunc([](WidgetInfo& info) { info.isHidden = CVarGetInteger(CVAR_ENHANCEMENT("CrawlSpeed"), 0) == 1; })
        .Options(CheckboxOptions().Tooltip(T("Don't increase crawl speed when exiting glitch-useful crawlspaces.\n"
                                           "Currently it is only the BOTW crawlspace to a locked door.")));
    AddWidget(path, T("King Zora Speed: %.2fx"), WIDGET_CVAR_SLIDER_FLOAT)
        .CVar(CVAR_ENHANCEMENT("MweepSpeed"))
        .Options(FloatSliderOptions().Min(0.1f).Max(5.0f).DefaultValue(1.0f).Format("%.2fx").Tooltip(
            T("Increase the speed of King Zora's move animation (\"mweep\")")));
    AddWidget(path, T("Faster Pause Menu"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("FasterPauseMenu"))
        .Options(CheckboxOptions().Tooltip(T("Speeds up animation of the pause menu, similar to Majora's Mask")));

    path.column = SECTION_COLUMN_3;
    AddWidget(path, T("Misc"), WIDGET_SEPARATOR_TEXT);
    AddWidget(path, T("Skip Child Stealth##Enhancement"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("TimeSavers.SkipChildStealth"))
        .Options(CheckboxOptions().Tooltip(
            T("The crawlspace into Hyrule Castle goes straight to Zelda, skipping the guards.")));
    AddWidget(path, T("Skip Tower Escape"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("TimeSavers.SkipTowerEscape"))
        .Options(CheckboxOptions().Tooltip(T("Skip the tower escape sequence between Ganondorf and Ganon.")));
    AddWidget(path, T("Skip Playing Scarecrow's Song"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("InstantScarecrow"))
        .PreFunc([](WidgetInfo& info) {
            info.options->disabled =
                IS_RANDO && (OTRGlobals::Instance->gRandoContext->GetOption(RSK_STARTING_SCARECROWS_SONG) ||
                             OTRGlobals::Instance->gRandoContext->GetOption(RSK_SHUFFLE_SCARECROWS_SONG));
            info.options->disabledTooltip =
                T("This setting is controlled by the randomizer because a randomized save file with the option "
                "\"Start with Scarecrow's Song\" or \"Shuffle Scarecrow's Song\" is currently loaded.");
        })
        .Options(CheckboxOptions().Tooltip(
            T("Pierre appears when an Ocarina is pulled out. Requires learning the Scarecrow's Song first.\n"
            "Without the randomizer option \"Start with Scarecrow's Song\" enabled for a seed, this still requires you "
            "to teach the scarecrow the song as both ages before summoning.")));
    AddWidget(path, T("Faster Rupee Accumulator"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("FasterRupeeAccumulator"))
        .Options(CheckboxOptions().Tooltip(T("Causes your Wallet to fill and empty faster when you gain or lose money.")));
    AddWidget(path, T("No Skulltula Freeze"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("SkulltulaFreeze"))
        .PreFunc([](WidgetInfo& info) {
            info.options->disabled = IS_RANDO && GameInteractor::IsSaveLoaded(true);
            info.options->disabledTooltip =
                T("This setting is disabled because a randomizer savefile is loaded. Please use the "
                "\"Skip Get Item Animation\" option within the randomizer enhancements instead.");
        })
        .Options(CheckboxOptions().Tooltip(
            T("Stops the game from freezing the player when picking up Gold Skulltula Tokens. Does not "
            "apply in randomizer savefiles.")));
    AddWidget(path, T("Skip Save Confirmation"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("SkipSaveConfirmation"))
        .Options(CheckboxOptions().Tooltip(T("Skip the \"Game Saved\" confirmation screen.")));
    AddWidget(path, T("Link as Default File Name"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("LinkDefaultName"))
        .Options(CheckboxOptions().Tooltip(T("Allows you to have \"Link\" as a premade file name.")));
    AddWidget(path, T("Spawn Bean Skulltula Faster"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("FasterBeanSkull"))
        .Options(CheckboxOptions().Tooltip(
            T("Makes Gold Skulltulas come out of bean patches faster after bugs dig into the center.")));
    AddWidget(path, T("Biggoron Forge Time: %d days"), WIDGET_CVAR_SLIDER_INT)
        .CVar(CVAR_ENHANCEMENT("ForgeTime"))
        .Options(IntSliderOptions().Min(0).Max(3).DefaultValue(3).Format(T("%d days")).Tooltip(
            T("Allows you to change the number of days it takes for "
            "Biggoron to forge the Biggoron's Sword.")));

    // Graphics
    path.sidebarName = "Graphics";
    AddSidebarEntry("Enhancements", path.sidebarName, 3);
    path.column = SECTION_COLUMN_1;

    AddWidget(path, T("Mods"), WIDGET_SEPARATOR_TEXT);
    AddWidget(path, T("Disable Bomb Billboarding"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("DisableBombBillboarding"))
        .RaceDisable(false)
        .Options(CheckboxOptions().Tooltip(
            T("Disables bombs always rotating to face the camera. To be used in conjunction with mods that want to "
            "replace bombs with 3D objects.")));
    AddWidget(path, T("Disable Grotto Fixed Rotation"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("DisableGrottoRotation"))
        .RaceDisable(false)
        .Options(CheckboxOptions().Tooltip(
            T("Disables Grottos rotating with the Camera. To be used in conjunction with mods that want to "
            "replace grottos with 3D objects.")));
    AddWidget(path, T("Disable Link's Sword Trail"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("DisableLinkSwordTrail"))
        .RaceDisable(false)
        .Options(CheckboxOptions().Tooltip(T("Disables the sword trail effect when swinging Link's sword. Useful when "
                                           "using mods that replace Link's sword model.")));
    AddWidget(path, T("Disable 2D Pre-Rendered Scenes"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("3DSceneRender"))
        .RaceDisable(false)
        .Options(CheckboxOptions().Tooltip(T("Disables 2D pre-rendered backgrounds. Enable this when using a mod that "
                                           "implements 3D backdrops for these areas.\n"
                                           "Requires Scene Change to alter.")));
    AddWidget(path, T("Disable Fixed Camera"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("DisableFixedCamera"))
        .RaceDisable(false)
        .PreFunc([](WidgetInfo& info) {
            if (CVarGetInteger(CVAR_ENHANCEMENT("3DSceneRender"), 0) == 0) {
                CVarSetInteger(CVAR_ENHANCEMENT("DisableFixedCamera"), 0);
                info.options->disabled = true;
            } else {
                info.options->disabled = false;
            }
            info.options->disabledTooltip = T("Requires \"Disable 2D Pre-Rendered Scenes\" to be enabled.");
        })
        .Options(CheckboxOptions().Tooltip(
            T("Disables the fixed camera in maps that use 2D pre-rendered backgrounds. Enable this when using a mod "
            "that implements 3D backdrops for these areas.\n"
            "Requires Scene Change to alter.")));
    AddWidget(path, T("Ingame Text Spacing: %d"), WIDGET_CVAR_SLIDER_INT)
        .CVar(CVAR_ENHANCEMENT("TextSpacing"))
        .RaceDisable(false)
        .Options(IntSliderOptions().Min(4).Max(6).DefaultValue(6).Tooltip(
            T("Space between text characters (useful for HD font textures).")));
    AddWidget(path, T("Models & Textures"), WIDGET_SEPARATOR_TEXT);
    AddWidget(path, T("Disable LOD"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("DisableLOD"))
        .RaceDisable(false)
        .Options(CheckboxOptions().Tooltip(
            T("Turns off the Level of Detail setting, making models use their Higher-Poly variants at any distance.")));
    AddWidget(path, T("Enemy Health Bars"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("EnemyHealthBar"))
        .Options(CheckboxOptions().Tooltip(T("Renders a health bar for Enemies when Z-Targeted.")));
    AddWidget(path, T("Enable 3D Dropped Items/Projectiles"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("NewDrops"))
        .RaceDisable(false)
        .Options(CheckboxOptions().Tooltip(
            T("Replaces most 2D items and projectiles on the overworld with their equivalent 3D models.")));
    AddWidget(path, T("Animated Link in Pause Menu"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("PauseMenuAnimatedLink"))
        .RaceDisable(false)
        .Options(CheckboxOptions().Tooltip(T("Turns the Static Image of Link in the Pause Menu's Equipment Subscreen "
                                           "into a model cycling through his idle animations.")));
    AddWidget(path, T("Show Age-Dependent Equipment"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("EquipmentAlwaysVisible"))
        .RaceDisable(false)
        .Options(CheckboxOptions().Tooltip(T("Makes all equipment visible, regardless of age.")));
    AddWidget(path, T("Scale Adult Equipment as Child"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("ScaleAdultEquipmentAsChild"))
        .RaceDisable(false)
        .PreFunc([](WidgetInfo& info) {
            info.isHidden = CVarGetInteger(CVAR_ENHANCEMENT("EquipmentAlwaysVisible"), 0) == 0;
        })
        .Options(CheckboxOptions().Tooltip(
            T("Scales all of the Adult Equipment, as well as moving some a bit, to fit on Child Link better. May "
            "not work properly with some mods.")));
    AddWidget(path, T("Show Gauntlets in First-Person"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("FirstPersonGauntlets"))
        .RaceDisable(false)
        .Options(CheckboxOptions().Tooltip(T("Renders Gauntlets when using the Bow and Hookshot like in OoT3D.")));
    AddWidget(path, T("Show Chains on Both Sides of Locked Doors"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("ShowDoorLocksOnBothSides"))
        .RaceDisable(false);
    AddWidget(path, T("Color Temple of Time's Medallions"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("ToTMedallionsColors"))
        .RaceDisable(false)
        .Options(CheckboxOptions().Tooltip(
            T("When Medallions are collected, the Medallion imprints around the Master Sword Pedestal in the Temple "
            "of Time will become colored-in.")));

    path.column = SECTION_COLUMN_2;
    AddWidget(path, T("UI"), WIDGET_SEPARATOR_TEXT);
    AddWidget(path, T("Minimal UI"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("MinimalUI"))
        .Options(CheckboxOptions().Tooltip(T("Hides most of the UI when not needed.\n"
                                           "NOTE: Doesn't activate until scene transition.")));
    AddWidget(path, T("Disable Hot/Underwater Warning Text"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("DisableTunicWarningText"))
        .Options(CheckboxOptions().Tooltip(T("Disables warning text when you don't have the Goron/Zora Tunic on "
                                           "in Hot/Underwater conditions.")));
    AddWidget(path, T("Remember Minimap State Between Areas"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("RememberMapToggleState"))
        .RaceDisable(false)
        .Options(CheckboxOptions().Tooltip(
            T("Preserve the minimap visibility state when going between areas rather than default it to \"on\" "
            "when going through loading zones.")));
    AddWidget(path, T("Visual Stone of Agony"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("VisualAgony"))
        .RaceDisable(false)
        .Options(CheckboxOptions().Tooltip(
            T("Displays an icon and plays a sound when Stone of Agony should be activated, for those without rumble.")));
    AddWidget(path, T("Disable HUD Heart Animations"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("NoHUDHeartAnimation"))
        .RaceDisable(false)
        .Options(CheckboxOptions().Tooltip(T("Disables the Beating Animation of the Hearts on the HUD.")));
    AddWidget(path, T("Glitch Line-up Tick"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("DrawLineupTick"))
        .Options(CheckboxOptions().Tooltip(
            T("Displays a tick in the top center of the screen to help with glitch line-ups in SoH, since traditional "
            "UI based line-ups do not work outside of 4:3")));
    AddWidget(path, T("Disable Black Bar Letterboxes"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("DisableBlackBars"))
        .RaceDisable(false)
        .Options(CheckboxOptions().Tooltip(
            T("Disables Black Bar Letterboxes during cutscenes and Z-Targeting. NOTE: There may be minor visual "
            "glitches that were covered up by the black bars. Please disable this setting before reporting a bug.")));
    AddWidget(path, T("Dynamic Wallet Icon"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("DynamicWalletIcon"))
        .RaceDisable(false)
        .Options(CheckboxOptions().Tooltip(
            T("Changes the Rupee in the Wallet icon to match the wallet size you currently have.")));
    AddWidget(path, T("Always Show Dungeon Entrances"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("AlwaysShowDungeonMinimapIcon"))
        .RaceDisable(false)
        .Options(CheckboxOptions().Tooltip(T("Always shows dungeon entrance icons on the Minimap.")));
    AddWidget(path, T("Better Ammo Rendering in Pause Menu"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("BetterAmmoRendering"))
        .RaceDisable(false)
        .Options(CheckboxOptions().Tooltip(
            T("Ammo counts in the pause menu will work correctly regardless of the position of items in the Inventory.")));

    AddWidget(path, T("File Select"), WIDGET_SEPARATOR_TEXT);
    AddWidget(path, T("More Info"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("FileSelect.MoreInfo"))
        .RaceDisable(false)
        .Options(CheckboxOptions().Tooltip(
            T("Shows what items you have collected in the File Select screen, like in N64 Randomizer.")));
    AddWidget(path, T("Enable Passage of Time"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("FileSelect.TimeFlow"))
        .RaceDisable(false)
        .Options(CheckboxOptions().Tooltip(T("The skybox in the background of the File Select screen will go through the "
                                           "day and night cycle over time.")));

    AddWidget(path, T("Hide Original"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("FileSelect.HideNormalQuest"))
        .RaceDisable(false)
        .PreFunc(HideQuestPreFunc(QUEST_NORMAL))
        .Options(CheckboxOptions().Tooltip(
            T("Hides the original game when selecting a quest type on the File Select screen.")));
    AddWidget(path, T("Hide Master Quest"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("FileSelect.HideMasterQuest"))
        .RaceDisable(false)
        .PreFunc(HideQuestPreFunc(QUEST_MASTER))
        .Options(CheckboxOptions().Tooltip(
            T("Hides the Master Quest option when selecting a quest type on the File Select screen.")));
    AddWidget(path, T("Hide Randomizer"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("FileSelect.HideRandomizerQuest"))
        .RaceDisable(false)
        .PreFunc(HideQuestPreFunc(QUEST_RANDOMIZER))
        .Options(CheckboxOptions().Tooltip(
            T("Hides the Randomizer option when selecting a quest type on the File Select screen.")));
    AddWidget(path, T("Hide Boss Rush"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("FileSelect.HideBossRushQuest"))
        .RaceDisable(false)
        .PreFunc(HideQuestPreFunc(QUEST_BOSSRUSH))
        .Options(CheckboxOptions().Tooltip(
            T("Hides the Boss Rush option when selecting a quest type on the File Select screen.")));
    AddWidget(path, T("Hide Speedrun"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("FileSelect.HideSpeedrunQuest"))
        .RaceDisable(false)
        .PreFunc(HideQuestPreFunc(QUEST_SPEEDRUN))
        .Options(CheckboxOptions().Tooltip(
            T("Hides the Speedrun option when selecting a quest type on the File Select screen.")));
    AddWidget(path, T("Hide Speedrun Master Quest"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("FileSelect.HideSpeedrunMasterQuest"))
        .RaceDisable(false)
        .PreFunc(HideQuestPreFunc(QUEST_SPEEDRUN_MASTER))
        .Options(CheckboxOptions().Tooltip(
            T("Hides the Speedrun Master Quest option when selecting a quest type on the File Select screen.")));

    path.column = SECTION_COLUMN_3;
    AddWidget(path, T("Misc."), WIDGET_SEPARATOR_TEXT);
    AddWidget(path, T("N64 Mode"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_LOW_RES_MODE)
        .RaceDisable(false)
        .Options(CheckboxOptions().Tooltip(
            T("Sets the aspect ratio to 4:3 and lowers resolution to 240p, the N64's native resolution.")));
    AddWidget(path, T("Remove Spin Attack Darkness"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("RemoveSpinAttackDarkness"))
        .RaceDisable(false)
        .Options(CheckboxOptions().Tooltip(T("Remove the Darkness that appears when charging a Spin Attack.")));
    AddWidget(path, T("Disable Link Spinning With Goron Pot"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("DisableLinkSpinWithGoronPot"))
        .RaceDisable(false)
        .Options(CheckboxOptions().Tooltip(T("Link will not spin when the Goron Pot starts to spin.")));
    AddWidget(path, T("Draw Distance"), WIDGET_SEPARATOR_TEXT).RaceDisable(false);
    AddWidget(path, T("Increase Actor Draw Distance: %dx"), WIDGET_CVAR_SLIDER_INT)
        .CVar(CVAR_ENHANCEMENT("DisableDrawDistance"))
        .RaceDisable(false)
        .Callback([](WidgetInfo& info) {
            if (CVarGetInteger(CVAR_ENHANCEMENT("DisableDrawDistance"), 1) <= 1) {
                CVarSetInteger(CVAR_ENHANCEMENT("DisableKokiriDrawDistance"), 0);
            }
        })
        .Options(IntSliderOptions().Min(1).Max(5).DefaultValue(1).Format("%dx").Tooltip(
            T("Increases the range in which Actors/Objects are drawn.")));
    AddWidget(path, T("Disable Kokiri Fade"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("DisableKokiriDrawDistance"))
        .RaceDisable(false)
        .PreFunc(
            [](WidgetInfo& info) { info.isHidden = CVarGetInteger(CVAR_ENHANCEMENT("DisableDrawDistance"), 1) <= 1; })
        .Options(CheckboxOptions().Tooltip(
            T("The Kokiri are mystical beings that fade into view when approached. Enabling this will remove their "
            "draw distance.")));
    AddWidget(path, T("Widescreen Actor Culling"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("WidescreenActorCulling"))
        .Options(
            CheckboxOptions().Tooltip(T("Adjusts the Horizontal Culling Plane to account for Widescreen Resolutions.")));
    AddWidget(path, T("Cull Glitch Useful Actors"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("ExtendedCullingExcludeGlitchActors"))
        .PreFunc([](WidgetInfo& info) {
            info.options->disabled = !CVarGetInteger(CVAR_ENHANCEMENT("WidescreenActorCulling"), 0) &&
                                     CVarGetInteger(CVAR_ENHANCEMENT("DisableDrawDistance"), 1) <= 1;
            info.options->disabledTooltip =
                T("Requires Actor Draw Distance to be increased or Widescreen Actor Culling to be enabled.");
        })
        .Options(CheckboxOptions().Tooltip(
            T("Exclude Actors that are useful for Glitches from the extended culling ranges. Some actors may still draw "
            "in the extended ranges, but will not \"update\" so that certain glitches that leverage the original "
            "culling requirements will still work.\n\nThe following actors are excluded:\n"
            " - White Clothed Gerudos\n"
            " - King Zora\n"
            " - Gossip Stones\n"
            " - Boulders\n"
            " - Blue Warps\n"
            " - Darunia\n"
            " - Gold Skulltulas")));

    path.sidebarName = "Items";
    AddSidebarEntry("Enhancements", path.sidebarName, 3);
    path.column = SECTION_COLUMN_1;

    AddWidget(path, T("Equipment"), WIDGET_SEPARATOR_TEXT);
    AddWidget(path, T("Equip Items on Dpad"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("DpadEquips"))
        .Options(CheckboxOptions().Tooltip(
            T("Equip items and equipment on the D-pad. If used with \"D-pad on Pause Screen\", you must "
            "hold C-Up to equip instead of navigate.")));
    AddWidget(path, T("Allow unequipping Items"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("ItemUnequip"))
        .Options(CheckboxOptions().Tooltip(T("Allows unequipping items from C-Buttons/D-pad by hovering over an equipped "
                                           "item and pressing the button it's equipped to.")));
    AddWidget(path, T("Assignable Shields, Tunics and Boots"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("AssignableTunicsAndBoots"))
        .Options(CheckboxOptions().Tooltip(T("Allows equipping Shields, Tunics and Boots to C-Buttons/D-pad.")));
    // TODO: Revist strength toggle, it's currently separate but should probably be locked behind the
    // Equipment toggle settings or be absorbed by it completely.
    AddWidget(path, T("Equipment Toggle"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("EquipmentCanBeRemoved"))
        .Options(CheckboxOptions().Tooltip(
            T("Allows equipment to be removed by toggling it on/off in the equipment subscreen.")));
    AddWidget(path, T("Allow Strength Equipment to be Toggled"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("ToggleStrength"))
        .Callback([](WidgetInfo& info) {
            if (!CVarGetInteger(CVAR_ENHANCEMENT("ToggleStrength"), 0)) {
                CVarSetInteger(CVAR_ENHANCEMENT("StrengthDisabled"), 0);
            }
        })
        .Options(CheckboxOptions().Tooltip(
            T("Allows Strength to be toggled on and off by pressing A on the Strength Upgrade "
            "in the Equipment Subscreen of the Pause Menu. This allows performing some glitches "
            "that require the player to not have Strength.")));
    AddWidget(path, T("Unsheathe Sword Without Slashing"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("UnsheatheWithoutSlashing"))
        .Options(CheckboxOptions().Tooltip(T("Allows Link to unsheathe the sword without slashing automatically.")));
    AddWidget(path, T("Sword Toggle Options"), WIDGET_CVAR_COMBOBOX)
        .CVar(CVAR_ENHANCEMENT("SwordToggle"))
        .PreFunc(
            [](WidgetInfo& info) { info.isHidden = CVarGetInteger(CVAR_ENHANCEMENT("EquipmentCanBeRemoved"), 0) == 0; })
        .Options(ComboboxOptions()
                     .ComboMap(swordToggleModes)
                     .DefaultIndex(SWORD_TOGGLE_NONE)
                     .Tooltip(T("Introduces Options for unequipping Link's sword\n\n"
                              "None: Only Biggoron's Sword/Giant's Knife can be toggled. Doing so will equip the "
                              "Master Sword.\n\n"
                              "Child Toggle: This will allow for completely unequipping any sword as Child link.\n\n"
                              "Both Ages: Any sword can be unequipped as either age. This may lead to swordless "
                              "glitches as adult.")));
    AddWidget(path, T("Ask to Equip New Items"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("AskToEquip"))
        .Options(CheckboxOptions().Tooltip(T("Adds a prompt to equip newly-obtained Swords, Shields, and Tunics.")));

    AddWidget(path, T("Ocarina"), WIDGET_SEPARATOR_TEXT);
    AddWidget(path, T("Prevent Dropped Ocarina Inputs"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("DpadNoDropOcarinaInput"))
        .Options(CheckboxOptions().Tooltip(T("Prevent dropping inputs when playing the Ocarina too quickly.")));
    AddWidget(path, T("Fast Ocarina Playback"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("FastOcarinaPlayback"))
        .Options(CheckboxOptions().Tooltip(T("Skip the part where the Ocarina Playback is called when you play a song.")));
    AddWidget(path, T("Time Travel with Song of Time"), WIDGET_CVAR_COMBOBOX)
        .CVar(CVAR_ENHANCEMENT("TimeTravel"))
        .Options(ComboboxOptions()
                     .ComboMap(timeTravelOptions)
                     .DefaultIndex(0)
                     .Tooltip(T("Allows Link to freely change age by playing the Song of Time.\n"
                              "Time Blocks can still be used properly.\n\n"
                              "Requirements:\n"
                              " - Obtained the Song of Time\n"
                              " - Obtained the Ocarina of Time (depends on selection)\n"
                              " - Obtained the Master Sword (depends on selection)\n"
                              " - Not within range of a Time Block\n"
                              " - Not within range of Ocarina Playing spots")));

    AddWidget(path, T("Masks"), WIDGET_SEPARATOR_TEXT);
    AddWidget(path, T("Bunny Hood Effect"), WIDGET_CVAR_COMBOBOX)
        .CVar(CVAR_BUNNY_HOOD_NAME)
        .PreFunc([](WidgetInfo& info) {
            info.options->disabled =
                IS_RANDO && OTRGlobals::Instance->gRandoContext->GetOption(RSK_BUNNY_HOOD).Is(RO_GENERIC_ON);
            info.options->disabledTooltip = T("This setting is forcefully enabled because a randomized savefile with "
                                            "\"Bunny Hood Effect\" is currently loaded.");
        })
        .Options(ComboboxOptions()
                     .ComboMap(bunnyHoodEffectMap)
                     .Tooltip(T("Wearing the Bunny Hood grants a speed and jump boost like in Majora's Mask.\n"
                              "Can also be limited to only the speed boost.\n"
                              "Randomizer logic only accounts for this when the seed's own \"Bunny Hood Effect\" "
                              "setting is on.\n"
                              "Also disables NPC's reactions to wearing the Bunny Hood.")));
    AddWidget(path, T("Masks Equippable as Adult"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ADULT_MASKS_NAME)
        .PreFunc([](WidgetInfo& info) {
            info.options->disabled =
                IS_RANDO && OTRGlobals::Instance->gRandoContext->GetOption(RSK_MASKS_AS_ADULT).Is(RO_GENERIC_ON);
            info.options->disabledTooltip = T("This setting is forcefully enabled because a randomized savefile with "
                                            "\"Masks as Adult\" is currently loaded.");
        })
        .Options(CheckboxOptions().Tooltip(T("Allows masks to be equipped normally from the pause menu as adult.\n"
                                           "Randomizer logic only accounts for this when the seed's own "
                                           "\"Masks as Adult\" setting is on.")));
    AddWidget(path, T("Persistent Masks"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("PersistentMasks"))
        .Options(
            CheckboxOptions().Tooltip(T("Stops masks from automatically unequipping in certain situations:\n"
                                      "- When entering a new scene\n"
                                      "- When not in any C-Button or the D-pad\n"
                                      "- When saving and quitting\n"
                                      "- When dying\n"
                                      "- When traveling through time (if \"Masks Equippable as Adult\" is activated)")));
    AddWidget(path, T("Invisible Bunny Hood"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("HideBunnyHood"))
        .Options(CheckboxOptions().Tooltip(T("Turns Bunny Hood Invisible while still maintaining its effects.")));
    AddWidget(path, T("Mask Select in Inventory"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("MaskSelect"))
        .PreFunc([](WidgetInfo& info) {
            info.options->disabled = IS_RANDO;
            info.options->disabledTooltip = T("This setting is forcefully enabled in randomizer.");
        })
        .Options(CheckboxOptions().Tooltip(
            T("After completing the mask trading sub-quest, press A and any direction on the mask "
            "slot to change masks.")));

    path.column = SECTION_COLUMN_2;
    AddWidget(path, T("Explosives"), WIDGET_SEPARATOR_TEXT);
    AddWidget(path, T("Remote Bombchu"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("RemoteBombchu"))
        .Options(CheckboxOptions().Tooltip(T("Allows you to control a Bombchu after dropping it.\n"
                                           "Control Stick: Steer\n"
                                           "B: Detonate\n"
                                           "A: Quit Control")));
    AddWidget(path, T("Deku Nuts Explode Bombs"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("NutsExplodeBombs"))
        .Options(CheckboxOptions().Tooltip(T("Make Deku Nuts explode Bombs, similar to how they interact with Bombchus. "
                                           "This does not affect Bomb Flowers.")));
    AddWidget(path, T("Remove Explosive Limit"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("RemoveExplosiveLimit"))
        .Options(CheckboxOptions().Tooltip(T("Removes the cap of 3 active explosives being deployed at once.")));
    AddWidget(path, T("Static Explosion Radius"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("StaticExplosionRadius"))
        .Options(CheckboxOptions().Tooltip(
            T("Explosions are now a static size, like in Majora's Mask and OoT3D. Makes Bombchu "
            "hovering much easier.")));
    AddWidget(path, T("Prevent Bombchus Forcing First-Person"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("DisableFirstPersonChus"))
        .Options(CheckboxOptions().Tooltip(
            T("Prevent Bombchus from forcing the camera into first-person mode when released.")));
    AddWidget(path, T("Better Bombchu Shopping"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("BetterBombchuShopping"))
        .PreFunc([](WidgetInfo& info) {
            info.options->disabled = IS_RANDO && GameInteractor::IsSaveLoaded(true);
            info.options->disabledTooltip = T("This setting is forcefully enabled when you are playing a randomizer.");
        })
        .Options(
            CheckboxOptions().Tooltip(T("Bombchus do not sell out when bought, and a 10 pack of Bombchus costs 99 rupees "
                                      "instead of 100.")));

    AddWidget(path, T("Bow / Slingshot"), WIDGET_SEPARATOR_TEXT);
    AddWidget(path, T("Equip Multiple Arrows at Once"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("SeparateArrows"))
        .Options(CheckboxOptions().Tooltip(
            T("Allow the Bow and Magic Arrows to be equipped at the same time on different slots. "
            "NOTE: This will disable the behavior of the 'Equip Dupe' glitch.")));
    AddWidget(path, T("Skip Magic Arrow Equip Animation"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("SkipArrowAnimation"));
    AddWidget(path, T("Bomb Arrows"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("BombArrows"))
        .Options(CheckboxOptions().Tooltip(T("Press A on the bomb slot to select bomb arrows. Each shot uses one arrow and one bomb.")));
    // TODO: See if a Callback could be registered to avoid the need to reload scenes for the next two options.
    AddWidget(path, T("Blue Fire Arrows##Enhancement"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("BlueFireArrows"))
        .PreFunc([](WidgetInfo& info) {
            info.options->disabled =
                OTRGlobals::Instance->gRandoContext->GetOption(RSK_BLUE_FIRE_ARROWS).Is(RO_GENERIC_ON);
            info.options->disabledTooltip = T("This setting is forcefully enabled because a randomized savefile with "
                                            "\"Blue Fire Arrows\" is currently loaded.");
        })
        .Options(CheckboxOptions().Tooltip(
            T("Allows Ice Arrows to melt Red Ice. May require a room reload if toggled during gameplay.")));
    AddWidget(path, T("Sunlight Arrows##Enhancement"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("SunlightArrows"))
        .PreFunc([](WidgetInfo& info) {
            info.options->disabled =
                OTRGlobals::Instance->gRandoContext->GetOption(RSK_SUNLIGHT_ARROWS).Is(RO_GENERIC_ON);
            info.options->disabledTooltip = T("This setting is forcefully enabled because a randomized savefile with "
                                            "\"Sunlight Arrows\" enabled is currently loaded.");
        })
        .Options(CheckboxOptions().Tooltip(
            T("Allows Light Arrows to activate Sun Switches. May require a room reload if toggled during gameplay.")));
    AddWidget(path, T("Bow as Child/Slingshot as Adult"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("BowSlingshotAmmoFix"))
        .Options(CheckboxOptions().Tooltip(T("Allows Child Link to use a Bow with Arrows.\n"
                                           "Allows Adult Link to use a Slingshot with Seeds.\n\n"
                                           "Requires glitches or the 'Timeless Equipment' cheat to equip.")));
    AddWidget(path, T("Aiming Reticle for the Bow/Slingshot"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("BowReticle"))
        .Options(CheckboxOptions().Tooltip(T("Aiming with a Bow or Slingshot will display a reticle as with the Hookshot "
                                           "when the projectile is ready to fire.")));
    AddWidget(path, T("Arrow Cycle"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("BowArrowCycle"))
        .Options(CheckboxOptions().Tooltip(
            T("Allows cycling between different arrow types (Normal, Fire, Ice, Light) while aiming the bow. "
            "Press the R button to cycle to the next available arrow type. "
            "Only works when aiming and only cycles to arrow types you own with sufficient magic.")));

    path.column = SECTION_COLUMN_3;
    AddWidget(path, T("Hookshot"), WIDGET_SEPARATOR_TEXT);
    AddWidget(path, T("Targetable Hookshot Reticle"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("HookshotableReticle"))
        .Options(CheckboxOptions().Tooltip(
            T("Makes the Hookshot Reticle use a different color when aiming at hookshotable collision.")));

    AddWidget(path, T("Boomerang"), WIDGET_SEPARATOR_TEXT);
    AddWidget(path, T("Instant Boomerang Recall"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("FastBoomerang"))
        .Options(CheckboxOptions().Tooltip(T("Instantly return the Boomerang to Link by pressing its item button while "
                                           "it's in the air.")));
    AddWidget(path, T("Aim Boomerang in First-Person Mode"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("BoomerangFirstPerson"))
        .Callback([](WidgetInfo& info) {
            if (!CVarGetInteger(CVAR_ENHANCEMENT("BoomerangFirstPerson"), 0)) {
                CVarSetInteger(CVAR_ENHANCEMENT("BoomerangReticle"), 0);
            }
        })
        .Options(CheckboxOptions().Tooltip(
            T("Change aiming for the Boomerang from Third-Person to First-Person to see past Link's head.")));
    AddWidget(path, T("Aiming Reticle for Boomerang"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("BoomerangReticle"))
        .PreFunc([](WidgetInfo& info) { info.isHidden = !CVarGetInteger(CVAR_ENHANCEMENT("BoomerangFirstPerson"), 0); })
        .Options(CheckboxOptions().Tooltip(T("Aiming with the Boomerang will display a reticle as with the Hookshot.")));

    AddWidget(path, T("Magic Spells"), WIDGET_SEPARATOR_TEXT);
    AddWidget(path, T("Better Farore's Wind"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("BetterFarore"))
        .Options(CheckboxOptions().Tooltip(
            T("Helps FW persist between ages, gives Child and Adult separate FW points, and can "
            "be used in more places.")));
    AddWidget(path, T("Faster Farore's Wind"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("FastFarores"))
        .Options(CheckboxOptions().Tooltip(T("Greatly decreases cast time of Farore's Wind magic spell.")));

    AddWidget(path, T("Bottles"), WIDGET_SEPARATOR_TEXT);
    AddWidget(path, T("Rebottle Blue Fire"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("RebottleBlueFire"))
        .Options(CheckboxOptions().Tooltip(T("Blue Fire dropped from a bottle can be bottled.")));

    // Fixes
    path.sidebarName = "Fixes";
    AddSidebarEntry("Enhancements", path.sidebarName, 3);
    path.column = SECTION_COLUMN_1;
    AddWidget(path, T("Gameplay Fixes"), WIDGET_SEPARATOR_TEXT);
    AddWidget(path, T("Fix the Gravedigging Tour Glitch"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("GravediggingTourFix"))
        .PreFunc([](WidgetInfo& info) {
            info.options->disabled = IS_RANDO && GameInteractor::IsSaveLoaded(true);
            info.options->disabledTooltip = T("This setting is always enabled in randomized save files.");
        })
        .Options(CheckboxOptions().Tooltip(
            T("Fixes a bug where the Gravedigging Tour Heart Piece disappears if the area reloads.")));
    AddWidget(path, T("Fix Dampé Going Backwards"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("FixDampeGoingBackwards"))
        .Options(CheckboxOptions().Tooltip(
            T("Fixes Dampé going backwards in certain circumstances when the player is going backwards.")));
    AddWidget(path, T("Fix Kokiri Forest Quest State"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("FixKokiriForestQuestState"))
        .Options(CheckboxOptions().Tooltip(T("Fixes Kokiri animation state to match their text state when getting "
                                           "Zelda's Letter before Kokiri Emerald.")));
    AddWidget(path, T("Fix Raised Floor Switches"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("FixFloorSwitches"))
        .Options(CheckboxOptions().Tooltip(T("Fixes the two raised floor switches, the one in the Forest Temple basement "
                                           "and the one at the top of the Fire "
                                           "Temple. This will lower them, making activating them easier.")));
    AddWidget(path, T("Fix Zora Hint Dialogue"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("FixZoraHintDialogue"))
        .Options(CheckboxOptions().Tooltip(
            T("Fixes one Zora's dialogue giving a hint about bringing Ruto's Letter to King Zora to properly occur "
            "before moving King Zora rather than after.")));
    AddWidget(path, T("Fix Falling from Vine Edges"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("FixVineFall"))
        .Options(
            CheckboxOptions().Tooltip(T("Prevents immediately falling off climbable surfaces if climbing on the edges.")));
    AddWidget(path, T("Fix Bush Item Drops"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("BushDropFix"))
        .Options(CheckboxOptions().Tooltip(
            T("Fixes the bushes to drop items correctly rather than spawning undefined items.")));
    AddWidget(path, T("Fix Flex Drops"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("FixFlexDrops"))
        .Options(CheckboxOptions().Tooltip(
            T("The flex drop gives whichever ammo you are lowest on, but doesn't check that you own the item to hold "
            "it, so the drop is lost. This makes it skip ammo you can't carry.")));
    AddWidget(path, T("Fix Enemies not Spawning Near Water"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("EnemySpawnsOverWaterboxes"))
        .Options(CheckboxOptions().Tooltip(
            T("Causes respawning enemies, like Stalchildren, to appear on land near bodies of water. "
            "Fixes an incorrect calculation that acted like water underneath ground was above it.")));
    AddWidget(path, T("Fix Poacher's Saw Softlock"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("FixSawSoftlock"))
        .PreFunc([](WidgetInfo& info) {
            info.options->disabled = CVarGetInteger(CVAR_ENHANCEMENT("SkipText"), 0) == 1;
            info.options->disabledTooltip = T("This option is forced on when Skip Text is enabled.");
        })
        .Options(CheckboxOptions().Tooltip(
            T("Prevents the Poacher's Saw softlock from mashing through the text, or with Skip Text enabled.")));
    AddWidget(path, T("Fix Anubis Fireballs"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("AnubisFix"))
        .Options(CheckboxOptions().Tooltip(
            T("Make Anubis Fireballs do Fire damage when reflected back at them with the Mirror Shield.")));
    AddWidget(path, T("Fix Goron City Doors After Fire Temple"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("GCDoorsAfterFireFix"))
        .Options(CheckboxOptions().Tooltip(
            T("Forces Goron City doors open if you somehow complete Fire Temple without talking to Goron Link"
            " and receiving the Goron Tunic.")));

    AddWidget(path, T("Fix MQ Water 1F Lock"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("MQWaterLockFix"))
        .PreFunc([](WidgetInfo& info) {
            info.options->disabled = IS_RANDO && GameInteractor::IsSaveLoaded(true);
            info.options->disabledTooltip = T("This setting is forcefully enabled when you are playing a Randomizer.");
        })
        .Options(CheckboxOptions().Tooltip(
            T("The second small key lock in MQ Water Temple is removed before the player can reach it by a shared flag "
            "with some Stalfos on the way to Dark Link.\n"
            "Enabling this will cause that lock to use a different flag, working as intended.")));

    AddWidget(path, T("Item-related Fixes"), WIDGET_SEPARATOR_TEXT);
    AddWidget(path, T("Fix Deku Nut Upgrade"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("DekuNutUpgradeFix"))
        .Options(CheckboxOptions().Tooltip(T("Prevents the Forest Stage Deku Nut upgrade from becoming unobtainable "
                                           "after receiving the Poacher's Saw.")));
    AddWidget(path, T("Fix Megaton Hammer Crouch Stab"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("CrouchStabHammerFix"))
        .Callback([](WidgetInfo& info) {
            if (!CVarGetInteger(CVAR_ENHANCEMENT("CrouchStabHammerFix"), 0)) {
                CVarClear(CVAR_ENHANCEMENT("CrouchStabFix"));
            }
        })
        .Options(CheckboxOptions().Tooltip(
            T("Make the Megaton Hammer's crouch stab able to destroy rocks without first swinging it normally.")));
    AddWidget(path, T("Remove Power Crouch Stab"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("CrouchStabFix"))
        .PreFunc(
            [](WidgetInfo& info) { info.isHidden = CVarGetInteger(CVAR_ENHANCEMENT("CrouchStabHammerFix"), 0) == 0; })
        .Options(CheckboxOptions().Tooltip(T("Make crouch stabbing always do the same damage as a regular slash.")));
    AddWidget(path, T("Fix Broken Giant's Knife Bug"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("FixBrokenGiantsKnife"))
        .PreFunc([](WidgetInfo& info) {
            info.options->disabled = IS_RANDO && GameInteractor::IsSaveLoaded(true);
            info.options->disabledTooltip = T("This setting is forcefully enabled when you are playing a Randomizer.");
        })
        .Callback([](WidgetInfo& info) {
            bool hasGiantsKnife = CHECK_OWNED_EQUIP(EQUIP_TYPE_SWORD, EQUIP_INV_SWORD_BIGGORON);
            bool hasBrokenKnife = CHECK_OWNED_EQUIP_ALT(EQUIP_TYPE_SWORD, EQUIP_INV_SWORD_BROKENGIANTKNIFE);
            bool knifeIsBroken = gSaveContext.swordHealth == 0.0f;

            if (hasGiantsKnife && (hasBrokenKnife != knifeIsBroken)) {
                func_800849EC(gPlayState);
            }
        })
        .Options(
            CheckboxOptions().Tooltip(T("Fixes the Broken Giant's Knife flag not being reset when Medigoron fixes it.")));

    AddWidget(path, T("Camera Fixes"), WIDGET_SEPARATOR_TEXT);
    AddWidget(path, T("Fix Camera Drift"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("FixCameraDrift"))
        .Options(CheckboxOptions().Tooltip(T("Fixes camera slightly drifting to the left when standing still due to a "
                                           "math error. May impact certain glitches.")));
    AddWidget(path, T("Fix Camera Swing"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("FixCameraSwing"))
        .Options(CheckboxOptions().Tooltip(
            T("Fixes camera getting stuck on collision when standing still. Also fixes slight shift "
            "back in camera when Link stops moving. May impact certain glitches.")));
    AddWidget(path, T("Fix Hanging Ledge Swing Rate"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("FixHangingLedgeSwingRate"))
        .Options(CheckboxOptions().Tooltip(T("Fixes camera swing rate when the player falls off a ledge and the camera "
                                           "swings around. May impact certain glitches.")));

    path.column = SECTION_COLUMN_2;
    AddWidget(path, T("Graphical Fixes"), WIDGET_SEPARATOR_TEXT);
    AddWidget(path, T("Fix L&R Pause Menu"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("FixMenuLR"))
        .RaceDisable(false)
        .Options(CheckboxOptions().Tooltip(T("Makes the L and R buttons in the pause menu the same color.")));
    AddWidget(path, T("Fix Dungeon Entrances"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("FixDungeonMinimapIcon"))
        .RaceDisable(false)
        .Options(CheckboxOptions().Tooltip(
            T("Removes the Dungeon Entrance icon on the top-left corner of the screen when no dungeon is present on the "
            "current map.")));
    AddWidget(path, T("Fix Two-Handed Idle Animations"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("TwoHandedIdle"))
        .RaceDisable(false)
        .Options(CheckboxOptions().Tooltip(
            T("Re-Enables the two-handed idle animation, a seemingly finished animation that was disabled on accident "
            "in the original game.")));
    AddWidget(path, T("Fix Navi Text HUD Position"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("NaviTextFix"))
        .RaceDisable(false)
        .Options(CheckboxOptions().Tooltip(T("Correctly centers the Navi text prompt on the HUD's C-Up button.")));
    AddWidget(path, T("Fix Gerudo Warrior's Clothing Colors"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("GerudoWarriorClothingFix"))
        .RaceDisable(false)
        .Options(CheckboxOptions().Tooltip(
            T("Prevent the Gerudo Warrior's clothes changing color when changing Link's tunic or "
            "using bombs in front of her.")));
    AddWidget(path, T("Fix Out of Bounds Textures"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("FixTexturesOOB"))
        .Callback([](WidgetInfo& info) { ApplyAuthenticGfxPatches(); })
        .Options(CheckboxOptions().Tooltip(
            T("Fixes authentic out of bounds texture reads, instead loading textures with the correct size.")));
    AddWidget(path, T("Fix Link's Eyes Open while Sleeping"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("FixEyesOpenWhileSleeping"))
        .RaceDisable(false)
        .Options(CheckboxOptions().Tooltip(
            T("Fixes Link's eyes being open in the opening cutscene when he is supposed to be sleeping.")));
    AddWidget(path, T("Fix Hand Holding Hammer"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("FixHammerHand"))
        .RaceDisable(false)
        .Options(CheckboxOptions().Tooltip(
            T("Fixes Adult Link having a backwards left hand when holding the Megaton Hammer.")));
    AddWidget(path, T("Fix Vanishing Paths"), WIDGET_CVAR_COMBOBOX)
        .CVar(CVAR_ENHANCEMENT("SceneSpecificDirtPathFix"))
        .RaceDisable(false)
        .Options(
            ComboboxOptions()
                .ComboMap(zFightingOptions)
                .DefaultIndex(ZFIGHT_FIX_DISABLED)
                .Tooltip(T("Disabled: Paths vanish more the higher the resolution (Z-Fighting is based on resolution).\n"
                         "Consistent: Certain paths vanish the same way in all resolutions.\n"
                         "No Vanish: Paths do not vanish, Link seems to sink in to some paths.\n"
                         "This might affect other decal effects.")));

    AddWidget(path, T("Audio Fixes"), WIDGET_SEPARATOR_TEXT);
    AddWidget(path, T("Fix Missing Jingle after 5 Silver Rupees"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("SilverRupeeJingleExtend"))
        .RaceDisable(false)
        .Options(CheckboxOptions().Tooltip(
            T("Adds 5 higher pitches for the Silver Rupee Jingle for the rooms with more than 5 Silver Rupees. "
            "Only relevant for playthroughs involving Master Quest Dungeons.")));

    AddWidget(path, T("Desync Fixes"), WIDGET_SEPARATOR_TEXT);
    AddWidget(path, T("Fix Darunia Dancing too Fast"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("FixDaruniaDanceSpeed"))
        .RaceDisable(false)
        .Options(CheckboxOptions().Tooltip(
            T("Fixes Darunia's dancing speed so he dances to the beat of Saria's Song, like in the Original Game.")));
    AddWidget(path, T("Fix Credits Timing (PAL)"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("CreditsFix"))
        .RaceDisable(false)
        .Options(CheckboxOptions().Tooltip(
            T("Extend certain credits scenes so the music lines up properly with the visuals. (PAL only)")));

    path.column = SECTION_COLUMN_3;
    AddWidget(path, T("Graphical Restorations"), WIDGET_SEPARATOR_TEXT);
    AddWidget(path, T("Red Ganon Blood"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("RedGanonBlood"))
        .RaceDisable(false)
        .Options(
            CheckboxOptions().Tooltip(T("Restore the original red blood from NTSC 1.0/1.1. Disable for Green blood.")));
    AddWidget(path, T("Restore Old Gold Skulltula Cutscene"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("GSCutscene"))
        .Options(CheckboxOptions().Tooltip(
            T("Restore pre-release behavior where defeating a Gold Skulltula will play a cutscene showing it die.")));
    AddWidget(path, T("Pulsate Boss Icon"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("PulsateBossIcon"))
        .RaceDisable(false)
        .Options(CheckboxOptions().Tooltip(
            T("Restores an unfinished feature to pulsate the boss room icon when you are in the boss room.")));
    AddWidget(path, T("Saria's Friends Forever Gesture"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("SariaGestureFriendsForever"))
        .RaceDisable(false)
        .Options(CheckboxOptions().Tooltip(
            T("Restores an unused animation of Saria when she says, \"Saria and Link will be friends forever.\"")));

    AddWidget(path, T("Glitch Restorations"), WIDGET_SEPARATOR_TEXT);
    AddWidget(path, T("Fish while Hovering"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("HoverFishing"))
        .Options(CheckboxOptions().Tooltip(
            T("Restore a bug from NTSC 1.0 that allows casting the Fishing Rod while using the Hover Boots.")));
    AddWidget(path, T("N64 Weird Frames"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("N64WeirdFrames"))
        .Options(CheckboxOptions().Tooltip(
            T("Restores N64 Weird Frames allowing weirdshots and weirdslides to behave the same as N64.")));
    AddWidget(path, T("Bombchus Out of Bounds"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("BombchusOOB"))
        .Options(
            CheckboxOptions().Tooltip(T("Allows Bombchus to explode out of bounds. Similar to GameCube and Wii VC.")));
    AddWidget(path, T("Quick Putaway"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("QuickPutaway"))
        .Options(CheckboxOptions().Tooltip(
            T("Restore a bug from NTSC 1.0 that allows putting away an item without an animation and performing "
            "Putaway Ocarina Items.")));
    AddWidget(path, T("Quick Bongo Kill"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("QuickBongoKill"))
        .Options(CheckboxOptions().Tooltip(
            T("Restore a bug from NTSC 1.0 that allows bypassing Bongo Bongo's intro cutscene to quickly kill him.")));
    AddWidget(path, T("Early Eyeball Frog"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("EarlyEyeballFrog"))
        .Options(CheckboxOptions().Tooltip(
            T("Restores a bug from NTSC 1.0/1.1 that allows you to obtain the eyeball frog from King Zora "
            "instead of the Zora Tunic by Holding Shield.")));
    AddWidget(path, T("Child Hookshot Softlock"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("ChildHookshotSoftlock"))
        .Options(CheckboxOptions().Tooltip(T("Using the Hookshot as child softlocks.")));
    AddWidget(path, T("Get Item Manipulation"), WIDGET_CVAR_COMBOBOX)
        .CVar(CVAR_ENHANCEMENT("GetItemManipulation"))
        .Options(ComboboxOptions()
                     .ComboMap(getItemManipulationOptions)
                     .DefaultIndex(GIM_DISABLED)
                     .Tooltip(T("Restores Get Item Manipulation.\n"
                              "NTSC and PAL have separate tables.")));

    AddWidget(path, T("Misc Restorations"), WIDGET_SEPARATOR_TEXT);
    AddWidget(path, T("Fix L&Z Page Switch in Pause Menu"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("NGCKaleidoSwitcher"))
        .Options(CheckboxOptions().Tooltip(
            T("Makes L and R switch pages like on the GameCube. Z opens the Debug Menu instead.")));
    AddWidget(path, T("Wide Door Ranges"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("WideShutterDoorRange"))
        .Options(CheckboxOptions().Tooltip(T("Restores the wider range of certain shutter doors from NTSC 1.0.\n"
                                           "Notably affects Jabu-Jabu and boss doors.")));
    AddWidget(path, T("Grave Hole Jumps"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("GraveHoles"))
        .Options(CheckboxOptions().Tooltip(
            T("Restores NTSC 1.0 behavior where Link jumps over grave holes and grabs the ledges.")));

    // Difficulty Options
    path.sidebarName = "Difficulty";
    AddSidebarEntry("Enhancements", path.sidebarName, 3);
    path.column = SECTION_COLUMN_1;

    AddWidget(path, T("Health"), WIDGET_SEPARATOR_TEXT);
    AddWidget(path, T("Permanent Heart Loss"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("PermanentHeartLoss"))
        .Options(CheckboxOptions().Tooltip(
            T("When you lose 4 quarters of a heart you will permanently lose that Heart Container.\n\n"
            "Disabling this after the fact will restore your Heart Containers.")));
    AddWidget(path, T("Damage Multiplier"), WIDGET_CVAR_COMBOBOX)
        .CVar(CVAR_ENHANCEMENT("DamageMult"))
        .Options(ComboboxOptions().ComboMap(allPowers).DefaultIndex(0).Tooltip(
            T("Modifies all sources of damage not affected by other sliders:\n"
            "2x: Can survive all common attacks from the start of the game.\n"
            "4x: Dies in 1 hit to any substantial attack from the start of the game.\n"
            "8x: Can only survive trivial damage from the start of the game.\n"
            "16x: Can survive all common attacks with max health without double defense.\n"
            "32x: Can survive all common attacks with max health and double defense.\n"
            "64x: Can survive trivial damage with max health without double defense.\n"
            "128x: Can survive trivial damage with max health and double defense.\n"
            "256x: Cannot survive damage.")));
    AddWidget(path, T("Fall Damage Multiplier"), WIDGET_CVAR_COMBOBOX)
        .CVar(CVAR_ENHANCEMENT("FallDamageMult"))
        .Options(ComboboxOptions().ComboMap(subPowers).Tooltip(
            T("Modifies all fall damage:\n"
            "2x: Can survive all fall damage from the start of the game.\n"
            "4x: Can only survive short fall damage from the start of the game.\n"
            "8x: Cannot survive any fall damage from the start of the game.\n"
            "16x: Can survive all fall damage with max health without double defense.\n"
            "32x: Can survive all fall damage with max health and double defense.\n"
            "64x: Can survive short fall damage with double defense.\n"
            "128x: Cannot survive fall damage.")));
    AddWidget(path, T("Void Damage Multiplier"), WIDGET_CVAR_COMBOBOX)
        .CVar(CVAR_ENHANCEMENT("VoidDamageMult"))
        .Options(ComboboxOptions()
                     .ComboMap(subSubPowers)
                     .DefaultIndex(0)
                     .Tooltip(T("Modifies damage taken after falling into a void:\n"
                              "2x: Can survive void damage from the start of the game.\n"
                              "4x: Cannot survive void damage from the start of the game.\n"
                              "8x: Can survive void damage twice with max health without double defense.\n"
                              "16x: Can survive void damage with max health without double defense.\n"
                              "32x: Can survive void damage with max health and double defense.\n"
                              "64x: Cannot survive void damage.")));
    AddWidget(path, T("Bonk Damage Multiplier"), WIDGET_CVAR_COMBOBOX)
        .CVar(CVAR_ENHANCEMENT("BonkDamageMult"))
        .Options(ComboboxOptions()
                     .ComboMap(bonkDamageValues)
                     .DefaultIndex(BONK_DAMAGE_NONE)
                     .Tooltip(T("Modifies Damage taken after Bonking.")));
    AddWidget(path, T("Spawn with Full Health"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("FullHealthSpawn"))
        .Options(CheckboxOptions().Tooltip(T("Respawn with Full Health instead of 3 hearts.")));
    AddWidget(path, T("No Heart Drops"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("NoHeartDrops"))
        .Options(CheckboxOptions().Tooltip(
            T("Disables Heart Drops, but not Heart Placements, like from a Deku Scrub running off.\n"
            "This simulates Hero Mode from other games in the series.")));

    path.column = SECTION_COLUMN_2;
    AddWidget(path, T("Drops"), WIDGET_SEPARATOR_TEXT);
    AddWidget(path, T("No Random Drops"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("NoRandomDrops"))
        .Options(CheckboxOptions().Tooltip(T("Disables Random Drops, except from the Goron Pot, Dampe, and Bosses.")));
    AddWidget(path, T("Enable Bombchu Drops"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("EnableBombchuDrops"))
        .PreFunc([](WidgetInfo& info) {
            info.options->disabled =
                IS_RANDO && GameInteractor::IsSaveLoaded(true) &&
                OTRGlobals::Instance->gRandoContext->GetOption(RSK_ENABLE_BOMBCHU_DROPS).Is(RO_GENERIC_ON);
            info.options->disabledTooltip = T("This setting is forcefully enabled because a randomized savefile with "
                                            "\"Enable Bombchu Drops\" is loaded.");
        })
        .Options(CheckboxOptions().Tooltip(T("Bombchus will sometimes drop in place of Bombs.")));
    AddWidget(path, T("Trees Drop Sticks"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("TreesDropSticks"))
        .Options(CheckboxOptions().Tooltip(
            T("Bonking into Trees will have a chance to drop up to 3 Sticks. Must have obtained sticks previously.")));
    AddWidget(path, T("Dampe Drop Rate"), WIDGET_CVAR_COMBOBOX)
        .CVar(CVAR_ENHANCEMENT("DampeDropRate"))
        .Options(ComboboxOptions()
                     .ComboMap(dampeDropRates)
                     .DefaultIndex(DAMPE_NORMAL)
                     .Tooltip(T("Adjusts rate Dampe drops flames during race.")));

    AddWidget(path, T("Miscellaneous"), WIDGET_SEPARATOR_TEXT);
    AddWidget(path, T("Delete File on Death"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("DeleteFileOnDeath"))
        .Options(CheckboxOptions().Tooltip(T("Dying will delete your file.\n\n\xef\x81\xb1 WARNING \xef\x81\xb1"
                                                    "\nTHIS IS IRREVERSIBLE!\nUSE AT YOUR OWN RISK!")));
    AddWidget(path, T("Switch Timer Multiplier"), WIDGET_CVAR_SLIDER_INT)
        .CVar(CVAR_ENHANCEMENT("SwitchTimerMultiplier"))
        .Options(IntSliderOptions().Min(-5).Max(5).DefaultValue(0).Format("%+d").Tooltip(
            T("-5 will be half as much time, +5 will be 6x as much time. Affects timed switches, torches, GTG statue "
            "eyes, & doors in race with Dampe.")));
    AddWidget(path, T("Always Win Goron Pot"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("GoronPot"))
        .Options(CheckboxOptions().Tooltip(T("Always get the Heart Piece/Purple Rupee from the Spinning Goron Pot.")));
    AddWidget(path, T("Always Win Dampe Digging Game"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("DampeWin"))
        .Options(CheckboxOptions().Tooltip(
            T("Always win the Heart Piece/Purple Rupee on the first dig in Dampe's Grave Digging game. "
            "In a Randomizer file, this defaults to on if this enhancement has never been changed.")));
    AddWidget(path, T("All Dogs are Richard"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("AllDogsRichard"))
        .Options(CheckboxOptions().Tooltip(T("All dogs can be traded in and will count as Richard.")));
    AddWidget(path, T("Cuccos Stay Put Multiplier: %dx"), WIDGET_CVAR_SLIDER_INT)
        .CVar(CVAR_ENHANCEMENT("CuccoStayDurationMult"))
        .Options(IntSliderOptions().Min(1).Max(5).DefaultValue(1).Format("%dx").Tooltip(
            T("Cuccos will stay in place longer after putting them down, by a multiple of the value of the slider.")));
    AddWidget(path, T("Cuccos Needed By Anju: %d"), WIDGET_CVAR_SLIDER_INT)
        .CVar(CVAR_ENHANCEMENT("CuccosToReturn"))
        .Options(IntSliderOptions().Min(0).Max(7).DefaultValue(7).Format("%d").Tooltip(
            T("The amount of cuccos needed to receive the bottle from Anju the Cucco Lady.")));

    path.column = SECTION_COLUMN_3;
    AddWidget(path, T("Enemies"), WIDGET_SEPARATOR_TEXT);
    AddWidget(path, T("Hyper Bosses"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("HyperBosses"))
        .Options(CheckboxOptions().Tooltip(T("All Major Bosses move and act twice as fast.")));
    AddWidget(path, T("Hyper Enemies"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("HyperEnemies"))
        .Options(CheckboxOptions().Tooltip(T("All Regular Enemies and Mini-Bosses move and act twice as fast.")));
    AddWidget(path, T("Enable Visual Guard Vision"), WIDGET_CVAR_CHECKBOX).CVar(CVAR_ENHANCEMENT("GuardVision"));
    AddWidget(path, T("Leever Spawn Rate: %d seconds"), WIDGET_CVAR_SLIDER_INT)
        .CVar(CVAR_ENHANCEMENT("LeeverSpawnRate"))
        .Options(IntSliderOptions()
                     .Min(0)
                     .Max(10)
                     .DefaultValue(0)
                     .Format(T("%d seconds"))
                     .Tooltip(T("The time between groups of Leevers spawning.")));

    // Minigames
    path.sidebarName = "Minigames";
    AddSidebarEntry("Enhancements", path.sidebarName, 3);
    path.column = SECTION_COLUMN_1;

    AddWidget(path, T("Shooting Gallery"), WIDGET_SEPARATOR_TEXT);
    auto shootingGalleryDisabledFunc = [](WidgetInfo& info) {
        info.options->disabled = !CVarGetInteger(CVAR_ENHANCEMENT("CustomizeShootingGallery"), 0);
        info.options->disabledTooltip = T("This option is disabled because \"Customize Behavior\" is turned off.");
    };
    AddWidget(path, T("Customize Behavior##Shooting"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("CustomizeShootingGallery"))
        .Options(CheckboxOptions().Tooltip(T("Turn on/off changes to the shooting gallery behavior.")));
    AddWidget(path, T("Instant Win"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("InstantShootingGalleryWin"))
        .PreFunc(shootingGalleryDisabledFunc)
        .Options(CheckboxOptions().Tooltip(T("Skips the Shooting Gallery minigame.")));
    AddWidget(path, T("No Rupee Randomization"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("ConstantAdultGallery"))
        .PreFunc(shootingGalleryDisabledFunc)
        .Options(CheckboxOptions().Tooltip(
            T("Forces the rupee order to not be randomized as adult, making it the same as child.")));
    AddWidget(path, T("Child Starting Ammunition: %d seeds"), WIDGET_CVAR_SLIDER_INT)
        .CVar(CVAR_ENHANCEMENT("ShootingGalleryAmmoChild"))
        .PreFunc(shootingGalleryDisabledFunc)
        .Options(IntSliderOptions()
                     .Min(10)
                     .Max(30)
                     .DefaultValue(15)
                     .Format(T("%d seeds"))
                     .Tooltip(T("The ammunition at the start of the Shooting Gallery minigame as Child.")));
    AddWidget(path, T("Adult Starting Ammunition: %d arrows"), WIDGET_CVAR_SLIDER_INT)
        .CVar(CVAR_ENHANCEMENT("ShootingGalleryAmmoAdult"))
        .PreFunc(shootingGalleryDisabledFunc)
        .Options(IntSliderOptions()
                     .Min(10)
                     .Max(30)
                     .DefaultValue(15)
                     .Format(T("%d arrows"))
                     .Tooltip(T("The ammunition at the start of the Shooting Gallery minigame as Adult.")));

    AddWidget(path, T("Bombchu Bowling"), WIDGET_SEPARATOR_TEXT);
    AddWidget(path, T("Customize Behavior##Bowling"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("CustomizeBombchuBowling"))
        .Options(CheckboxOptions().Tooltip(T("Turn on/off changes to the Bombchu Bowling behavior.")));
    auto bombchuBowlingDisabledFunc = [](WidgetInfo& info) {
        info.options->disabled = CVarGetInteger(CVAR_ENHANCEMENT("CustomizeBombchuBowling"), 0) == 0;
        info.options->disabledTooltip = T("This option is disabled because \"Customize Behavior\" is turned off.");
    };
    AddWidget(path, T("Remove Small Cucco"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("BombchuBowlingNoSmallCucco"))
        .PreFunc(bombchuBowlingDisabledFunc)
        .Options(CheckboxOptions().Tooltip(T("Prevents the small Cucco from appearing in the Bombchu Bowling minigame.")));
    AddWidget(path, T("Remove Big Cucco"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("BombchuBowlingNoBigCucco"))
        .PreFunc(bombchuBowlingDisabledFunc)
        .Options(CheckboxOptions().Tooltip(T("Prevents the big Cucco from appearing in the Bombchu Bowling minigame.")));
    AddWidget(path, T("Bombchu Count: %d bombchus"), WIDGET_CVAR_SLIDER_INT)
        .CVar(CVAR_ENHANCEMENT("BombchuBowlingAmmo"))
        .PreFunc(bombchuBowlingDisabledFunc)
        .Options(IntSliderOptions()
                     .Min(3)
                     .Max(20)
                     .DefaultValue(10)
                     .Format(T("%d bombchus"))
                     .Tooltip(T("The number of Bombchus available at the start of the Bombchu Bowling minigame.")));
    AddWidget(path, T("First Prize"), WIDGET_CVAR_COMBOBOX)
        .CVar(CVAR_ENHANCEMENT("BombchuBowlingFirstPrize"))
        .PreFunc(bombchuBowlingDisabledFunc)
        .Options(ComboboxOptions()
                     .ComboMap(bombchuBowlingFirstPrizeOptions)
                     .DefaultIndex(BOWLING_FIRST_PRIZE_RANDOM)
                     .Tooltip(T("The prize the cycle starts on.")));
    AddWidget(path, T("Horseback Archery"), WIDGET_SEPARATOR_TEXT);
    AddWidget(path, T("Customize Behavior##HBA"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("CustomizeHorsebackArchery"))
        .Options(CheckboxOptions().Tooltip(T("Turn on/off changes to the Horseback Archery minigame behavior.")));
    auto hbaDisabledFunc = [](WidgetInfo& info) {
        info.options->disabled = CVarGetInteger(CVAR_ENHANCEMENT("CustomizeHorsebackArchery"), 0) == 0;
        info.options->disabledTooltip = T("This option is disabled because \"Customize Behavior\" is turned off.");
    };
    AddWidget(path, T("Instant Win##HBA"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("InstantHorsebackArcheryWin"))
        .PreFunc(hbaDisabledFunc)
        .Options(CheckboxOptions().Tooltip(T("Skips the Horseback Archery minigame, automatically awarding the prize.")));
    AddWidget(path, T("Always Score 100##HBA"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("HorsebackArcheryAlwaysScore"))
        .PreFunc(hbaDisabledFunc)
        .Options(CheckboxOptions().Tooltip(
            T("Every arrow that hits a target scores 100 points (inner ring) regardless of where it lands.")));
    AddWidget(path, T("Arrow Count"), WIDGET_CVAR_SLIDER_INT)
        .CVar(CVAR_ENHANCEMENT("HorsebackArcheryAmmo"))
        .PreFunc(hbaDisabledFunc)
        .Options(IntSliderOptions()
                     .Min(15)
                     .Max(40)
                     .DefaultValue(20)
                     .Format(T("%d arrows"))
                     .Tooltip(T("The number of arrows available at the start of the Horseback Archery minigame.")));

    AddWidget(path, T("Frogs' Ocarina Game"), WIDGET_SEPARATOR_TEXT);
    AddWidget(path, T("Customize Behavior##Frogs"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("CustomizeFrogsOcarinaGame"))
        .Options(CheckboxOptions().Tooltip(T("Turn on/off changes to the Frogs' Ocarina Game behavior.")));
    auto frogsOcarinaGameDisabledFunc = [](WidgetInfo& info) {
        info.options->disabled = CVarGetInteger(CVAR_ENHANCEMENT("CustomizeFrogsOcarinaGame"), 0) == 0;
        info.options->disabledTooltip = T("This option is disabled because \"Customize Behavior\" is turned off.");
    };
    AddWidget(path, T("Instant Win##Frogs"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("InstantFrogsGameWin"))
        .PreFunc(frogsOcarinaGameDisabledFunc)
        .Options(CheckboxOptions().Tooltip(T("Skips the Frogs' Ocarina Game.")));
    AddWidget(path, T("Unlimited Playback Time##Frogs"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("FrogsUnlimitedFailTime"))
        .PreFunc(frogsOcarinaGameDisabledFunc)
        .Options(CheckboxOptions().Tooltip(T("Removes the timer to play back the song.")));
    AddWidget(path, T("Modify Note Timer: %dx"), WIDGET_CVAR_SLIDER_INT)
        .CVar(CVAR_ENHANCEMENT("FrogsModifyFailTime"))
        .PreFunc([](WidgetInfo& info) {
            info.options->disabled = !CVarGetInteger(CVAR_ENHANCEMENT("CustomizeFrogsOcarinaGame"), 0) ||
                                     CVarGetInteger(CVAR_ENHANCEMENT("FrogsUnlimitedFailTime"), 0);
            info.options->disabledTooltip = T("This option is disabled because \"Customize Behavior\" is turned off or "
                                            "\"Unlimited Playback Time\" is on.");
        })
        .Options(IntSliderOptions().Min(1).Max(5).DefaultValue(1).Format("%dx").Tooltip(
            T("Adjusts the time allowed for playback before failing.")));

    path.column = SECTION_COLUMN_2;
    AddWidget(path, T("Lost Woods Ocarina Game"), WIDGET_SEPARATOR_TEXT);
    AddWidget(path, T("Customize Behavior##LostWoods"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("CustomizeOcarinaGame"))
        .Options(CheckboxOptions().Tooltip(T("Turn on/off changes to the Lost Woods Ocarina Game behavior.")));
    auto ocarinaMemoryGameDisabledFunc = [](WidgetInfo& info) {
        info.options->disabled = CVarGetInteger(CVAR_ENHANCEMENT("CustomizeOcarinaGame"), 0) == 0;
        info.options->disabledTooltip = T("This option is disabled because \"Customize Behavior\" is turned off.");
    };
    AddWidget(path, T("Instant Win##LostWoods"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("InstantOcarinaGameWin"))
        .PreFunc(ocarinaMemoryGameDisabledFunc)
        .Options(CheckboxOptions().Tooltip(T("Skips the Lost Woods Ocarina Memory Game.")));
    AddWidget(path, T("Note Play Speed: %dx"), WIDGET_CVAR_SLIDER_INT)
        .CVar(CVAR_ENHANCEMENT("OcarinaGame.NoteSpeed"))
        .PreFunc(ocarinaMemoryGameDisabledFunc)
        .Options(IntSliderOptions().Min(1).Max(5).DefaultValue(1).Format("%dx").Tooltip(
            T("Adjust the speed that the Skull Kids play the notes.")));
    AddWidget(path, T("Unlimited Playback Time##LostWoods"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("OcarinaUnlimitedFailTime"))
        .PreFunc(ocarinaMemoryGameDisabledFunc)
        .Options(CheckboxOptions().Tooltip(T("Removes the timer to play back the song.")));
    AddWidget(path, T("Number of Starting Notes: %d notes"), WIDGET_CVAR_SLIDER_INT)
        .CVar(CVAR_ENHANCEMENT("OcarinaGame.StartingNotes"))
        .PreFunc(ocarinaMemoryGameDisabledFunc)
        .Options(IntSliderOptions()
                     .Min(1)
                     .Max(8)
                     .DefaultValue(3)
                     .Format(T("%d notes"))
                     .Tooltip(T("Adjust the number of notes the Skull Kids play to start the first round.")));
    int roundMin = CVarGetInteger(CVAR_ENHANCEMENT("OcarinaGame.StartingNotes"), 3);
    AddWidget(path, T("Round One Notes: %d notes"), WIDGET_CVAR_SLIDER_INT)
        .CVar(CVAR_ENHANCEMENT("OcarinaGame.RoundOneNotes"))
        .PreFunc(ocarinaMemoryGameDisabledFunc)
        .Options(IntSliderOptions()
                     .Min(roundMin)
                     .Max(8)
                     .DefaultValue(5)
                     .Format(T("%d notes"))
                     .Tooltip(T("Adjust the number of notes you need to play to end the first round.")));
    AddWidget(path, T("Round Two Notes: %d notes"), WIDGET_CVAR_SLIDER_INT)
        .CVar(CVAR_ENHANCEMENT("OcarinaGame.RoundTwoNotes"))
        .PreFunc(ocarinaMemoryGameDisabledFunc)
        .Options(IntSliderOptions()
                     .Min(roundMin)
                     .Max(8)
                     .DefaultValue(6)
                     .Format(T("%d notes"))
                     .Tooltip(T("Adjust the number of notes you need to play to end the second round.")));
    AddWidget(path, T("Round Three Notes: %d notes"), WIDGET_CVAR_SLIDER_INT)
        .CVar(CVAR_ENHANCEMENT("OcarinaGame.RoundThreeNotes"))
        .PreFunc(ocarinaMemoryGameDisabledFunc)
        .Options(IntSliderOptions()
                     .Min(roundMin)
                     .Max(8)
                     .DefaultValue(8)
                     .Format(T("%d notes"))
                     .Tooltip(T("Adjust the number of notes you need to play to end the third round.")));

    AddWidget(path, T("Forest Temple"), WIDGET_SEPARATOR_TEXT);
    AddWidget(path, T("Solve Amy's Puzzle"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("SkipAmyPuzzle"))
        .Options(CheckboxOptions().Tooltip(T("Amy's block-pushing puzzle is instantly solved.")));

    AddWidget(path, T("Ingo's Race"), WIDGET_SEPARATOR_TEXT);
    AddWidget(path, T("Number of Races"), WIDGET_CVAR_COMBOBOX)
        .CVar(CVAR_ENHANCEMENT("IngoRaceOnce"))
        .Options(ComboboxOptions()
                     .ComboMap(ingoRaceOptions)
                     .DefaultIndex(INGO_RACE_TWICE)
                     .Tooltip(T("Number of races Link must win against Ingo to earn Epona. "
                              "Only works if Link is riding Epona, not the other rideable horse.")));

    path.column = SECTION_COLUMN_3;
    AddWidget(path, T("Rupee Diving Game"), WIDGET_SEPARATOR_TEXT);
    AddWidget(path, T("Time Limit: %d seconds"), WIDGET_CVAR_SLIDER_INT)
        .CVar(CVAR_ENHANCEMENT("DivingGame.TimeLimit"))
        .Options(IntSliderOptions().Min(30).Max(120).DefaultValue(50).Format(T("%d seconds")));

    AddWidget(path, T("Fishing"), WIDGET_SEPARATOR_TEXT);
    AddWidget(path, T("Customize Behavior##Fishing"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("CustomizeFishing"))
        .Options(CheckboxOptions().Tooltip(T("Turn on/off changes to the Fishing behavior.")));
    auto fishingDisabledFunc = [](WidgetInfo& info) {
        info.options->disabled = CVarGetInteger(CVAR_ENHANCEMENT("CustomizeFishing"), 0) == 0;
        info.options->disabledTooltip = T("This option is disabled because \"Customize Behavior\" is turned off.");
    };
    AddWidget(path, T("Instant Fishing"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("InstantFishing"))
        .PreFunc(fishingDisabledFunc)
        .Options(CheckboxOptions().Tooltip(T("All fish will be caught instantly.")));
    AddWidget(path, T("Guarantee Bite"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("GuaranteeFishingBite"))
        .PreFunc(fishingDisabledFunc)
        .Options(CheckboxOptions().Tooltip(T("When a line is stable, guarantee a bite. Otherwise use Default logic.")));
    AddWidget(path, T("Fish never Escape"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("FishNeverEscape"))
        .PreFunc(fishingDisabledFunc)
        .Options(CheckboxOptions().Tooltip(T("Once a hook has been set, Fish will never let go while being reeled in.")));
    AddWidget(path, T("Loaches always Appear"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("LoachesAlwaysAppear"))
        .PreFunc(fishingDisabledFunc)
        .Options(
            CheckboxOptions().Tooltip(T("Loaches will always appear in the fishing pond instead of every four visits.")));
    AddWidget(path, T("Skip Keep Confirmation"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("SkipKeepConfirmation"))
        .PreFunc(fishingDisabledFunc)
        .Options(
            CheckboxOptions().Tooltip(T("The Pond Owner will not ask to confirm if you want to keep a smaller Fish.")));
    AddWidget(path, T("All Fish are Hyrule Loaches"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("AllHyruleLoaches"))
        .PreFunc(fishingDisabledFunc)
        .Options(CheckboxOptions().Tooltip(T("Every fish in the Fishing Pond will always be a Hyrule Loach.\n\n"
                                           "NOTE: This requires reloading the area.")));
    AddWidget(path, T("Child Minimum Weight: %d lbs."), WIDGET_CVAR_SLIDER_INT)
        .CVar(CVAR_ENHANCEMENT("MinimumFishWeightChild"))
        .PreFunc(fishingDisabledFunc)
        .Options(IntSliderOptions().Min(3).Max(10).DefaultValue(10).Format(T("%d lbs.")).Tooltip(
            T("The minimum weight for the unique Fishing Reward as a child.")));
    AddWidget(path, T("Adult Minimum Weight: %d lbs."), WIDGET_CVAR_SLIDER_INT)
        .CVar(CVAR_ENHANCEMENT("MinimumFishWeightAdult"))
        .PreFunc(fishingDisabledFunc)
        .Options(IntSliderOptions().Min(6).Max(13).DefaultValue(13).Format(T("%d lbs.")).Tooltip(
            T("The minimum weight for the unique fishing reward as an adult.")));
    AddWidget(path, T("Allow fishing with blank B"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("FishingBlankB"))
        .Options(CheckboxOptions().Tooltip(T("Allow fishing even when not having any item equipped on the B button, "
                                           "fixing a vanilla bug. Always enabled in randomizer.")));

    AddWidget(path, T("Multiple Prizes"), WIDGET_SEPARATOR_TEXT);
    AddWidget(path, T("Dampe's Race"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("DampeBothPrizes"))
        .PreFunc([](WidgetInfo& info) {
            info.options->disabled = IS_RANDO && GameInteractor::IsSaveLoaded(true);
            info.options->disabledTooltip = T("This setting is forcefully enabled when you are playing a Randomizer.");
        })
        .Options(CheckboxOptions().Tooltip(T("Dampe awards both prizes on the first race, not just the Hookshot.")));
    AddWidget(path, T("Horseback Archery"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("HorsebackArcheryBothPrizes"))
        .Options(CheckboxOptions().Tooltip(T("Link can win both Horseback Archery prizes in one attempt")));

    // Extra Modes
    path.sidebarName = "Extra Modes";
    AddSidebarEntry("Enhancements", path.sidebarName, 3);
    path.column = SECTION_COLUMN_1;

    AddWidget(path, T("Bounce off Walls"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("BounceOffWalls"))
        .Options(
            CheckboxOptions().Tooltip(T("Allows Link to bounce off walls when linear velocity is high enough. This is "
                                      "relevant when frequently being knocked back by traps, CC, or in Anchor.")));
    AddWidget(path, T("Mirrored World"), WIDGET_CVAR_COMBOBOX)
        .CVar(CVAR_ENHANCEMENT("MirroredWorldMode"))
        .Options(
            ComboboxOptions()
                .DefaultIndex(MIRRORED_WORLD_OFF)
                .ComboMap(mirroredWorldModes)
                .Tooltip(
                    T("Mirrors the world horizontally:\n\n"
                    " - Always: Always mirror the world.\n"
                    " - Random: Randomly decide to mirror the world on each scene change.\n"
                    " - Random (Seeded): Scenes are mirrored based on the current randomizer seed/file.\n"
                    " - Dungeons: Mirror the world in Dungeons.\n"
                    " - Dungeons (Vanilla): Mirror the world in Vanilla Dungeons.\n"
                    " - Dungeons (MQ): Mirror the world in MQ Dungeons.\n"
                    " - Dungeons Random: Randomly decide to mirror the world in Dungeons.\n"
                    " - Dungeons Random (Seeded): Dungeons are mirrored based on the current randomizer seed/file.")));
    AddWidget(path, T("Ivan the Fairy (Coop Mode)"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("IvanCoopModeEnabled"))
        .Options(CheckboxOptions().Tooltip(
            T("Enables Ivan the Fairy. Player 2 can control Ivan and press the C-Buttons to use items and mess with "
            "Player 1!")));
    AddWidget(path, T("Dogs Follow You Everywhere"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("DogFollowsEverywhere"))
        .Options(CheckboxOptions().Tooltip(T("Allows dogs to follow you anywhere you go, even if you leave the Market.")));
    AddWidget(path, T("Rupee Dash Mode"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("RupeeDash"))
        .Options(CheckboxOptions().Tooltip(T("Rupees reduce over time; Link suffers damage when the count hits 0.")));
    AddWidget(path, T("Rupee Dash Wallet Scaling"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("RupeeDashScaling"))
        .PreFunc([](WidgetInfo& info) { info.isHidden = CVarGetInteger(CVAR_ENHANCEMENT("RupeeDash"), 0) == 0; })
        .Options(CheckboxOptions().DefaultValue(true).Tooltip(T("The larger Link's wallet, the faster Rupees reduce.")));
    AddWidget(path, T("Rupee Dash Interval: %d seconds"), WIDGET_CVAR_SLIDER_INT)
        .CVar(CVAR_ENHANCEMENT("RupeeDashInterval"))
        .PreFunc([](WidgetInfo& info) { info.isHidden = CVarGetInteger(CVAR_ENHANCEMENT("RupeeDash"), 0) == 0; })
        .Options(IntSliderOptions()
                     .Min(1)
                     .Max(10)
                     .DefaultValue(5)
                     .Format(T("%d seconds"))
                     .Tooltip(T("Interval between Rupee reduction in Rupee Dash Mode.")));
    AddWidget(path, T("Shadow Tag Mode"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("ShadowTag"))
        .Options(CheckboxOptions().Tooltip(T("A Wallmaster follows Link everywhere, don't get caught!")));
    AddWidget(path, T("Hurt Container Mode"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("HurtContainer"))
        .Options(CheckboxOptions().Tooltip(T("Changes Heart Piece and Heart Container functionality.\n\n"
                                           " - Each Heart Container or full Heart Piece reduces Link's Hearts by 1.\n"
                                           " - Can be enabled retroactively after a File has already started.")));
    AddWidget(path, T("Additional Traps"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("ExtraTraps.Enabled"))
        .Options(CheckboxOptions().Tooltip(T("Enables additional Trap variants.")));
    AddWidget(path, T("Trap Options"), WIDGET_SEPARATOR_TEXT).PreFunc([](WidgetInfo& info) {
        info.isHidden = CVarGetInteger(CVAR_ENHANCEMENT("ExtraTraps.Enabled"), 0) == 0;
    });
    AddWidget(path, T("Tier 1 Traps:"), WIDGET_TEXT).PreFunc([](WidgetInfo& info) {
        info.isHidden = CVarGetInteger(CVAR_ENHANCEMENT("ExtraTraps.Enabled"), 0) == 0;
    });
    AddWidget(path, T("Freeze Traps"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("ExtraTraps.Ice"))
        .PreFunc(
            [](WidgetInfo& info) { info.isHidden = CVarGetInteger(CVAR_ENHANCEMENT("ExtraTraps.Enabled"), 0) == 0; });
    AddWidget(path, T("Burn Traps"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("ExtraTraps.Burn"))
        .PreFunc(
            [](WidgetInfo& info) { info.isHidden = CVarGetInteger(CVAR_ENHANCEMENT("ExtraTraps.Enabled"), 0) == 0; });
    AddWidget(path, T("Shock Traps"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("ExtraTraps.Shock"))
        .PreFunc(
            [](WidgetInfo& info) { info.isHidden = CVarGetInteger(CVAR_ENHANCEMENT("ExtraTraps.Enabled"), 0) == 0; });
    AddWidget(path, T("Tier 2 Traps:"), WIDGET_TEXT).PreFunc([](WidgetInfo& info) {
        info.isHidden = CVarGetInteger(CVAR_ENHANCEMENT("ExtraTraps.Enabled"), 0) == 0;
    });
    AddWidget(path, T("Knockback Traps"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("ExtraTraps.Knockback"))
        .PreFunc(
            [](WidgetInfo& info) { info.isHidden = CVarGetInteger(CVAR_ENHANCEMENT("ExtraTraps.Enabled"), 0) == 0; });
    AddWidget(path, T("Speed Traps"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("ExtraTraps.Speed"))
        .PreFunc(
            [](WidgetInfo& info) { info.isHidden = CVarGetInteger(CVAR_ENHANCEMENT("ExtraTraps.Enabled"), 0) == 0; });
    AddWidget(path, T("Bomb Traps"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("ExtraTraps.Bomb"))
        .PreFunc(
            [](WidgetInfo& info) { info.isHidden = CVarGetInteger(CVAR_ENHANCEMENT("ExtraTraps.Enabled"), 0) == 0; });
    AddWidget(path, T("Tier 3 Traps:"), WIDGET_TEXT).PreFunc([](WidgetInfo& info) {
        info.isHidden = CVarGetInteger(CVAR_ENHANCEMENT("ExtraTraps.Enabled"), 0) == 0;
    });
    AddWidget(path, T("Void Traps"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("ExtraTraps.Void"))
        .PreFunc(
            [](WidgetInfo& info) { info.isHidden = CVarGetInteger(CVAR_ENHANCEMENT("ExtraTraps.Enabled"), 0) == 0; });
    AddWidget(path, T("Ammo Traps"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("ExtraTraps.Ammo"))
        .PreFunc(
            [](WidgetInfo& info) { info.isHidden = CVarGetInteger(CVAR_ENHANCEMENT("ExtraTraps.Enabled"), 0) == 0; });
    AddWidget(path, T("Death Traps"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("ExtraTraps.Kill"))
        .PreFunc(
            [](WidgetInfo& info) { info.isHidden = CVarGetInteger(CVAR_ENHANCEMENT("ExtraTraps.Enabled"), 0) == 0; });
    AddWidget(path, T("Teleport Traps"), WIDGET_CVAR_COMBOBOX)
        .CVar(CVAR_ENHANCEMENT("ExtraTraps.Teleport"))
        .PreFunc(
            [](WidgetInfo& info) { info.isHidden = CVarGetInteger(CVAR_ENHANCEMENT("ExtraTraps.Enabled"), 0) == 0; })
        .Options(ComboboxOptions()
                     .ComboMap(teleportTrapModes)
                     .DefaultIndex(TELEPORT_TRAP_OFF)
                     .Tooltip(T("Where a Teleport Trap can send you. Off keeps them out of the trap pool.\n\n"
                              "Simple: Link's House and the six warp song pads.\n"
                              "Advanced: The same pool the randomizer uses when it shuffles spawns and warp songs.")));

    // Cheats
    path.sidebarName = "Cheats";
    AddSidebarEntry("Enhancements", path.sidebarName, 3);
    path.column = SECTION_COLUMN_1;

    AddWidget(path, T("Infinite..."), WIDGET_SEPARATOR_TEXT);
    AddWidget(path, T("Money"), WIDGET_CVAR_CHECKBOX).CVar(CVAR_CHEAT("InfiniteMoney"));
    AddWidget(path, T("Health"), WIDGET_CVAR_CHECKBOX).CVar(CVAR_CHEAT("InfiniteHealth"));
    AddWidget(path, T("Ammo"), WIDGET_CVAR_CHECKBOX).CVar(CVAR_CHEAT("InfiniteAmmo"));
    AddWidget(path, T("Magic"), WIDGET_CVAR_CHECKBOX).CVar(CVAR_CHEAT("InfiniteMagic"));
    AddWidget(path, T("Nayru's Love"), WIDGET_CVAR_CHECKBOX).CVar(CVAR_CHEAT("InfiniteNayru"));
    AddWidget(path, T("Epona Boost"), WIDGET_CVAR_CHECKBOX).CVar(CVAR_CHEAT("InfiniteEponaBoost"));

    AddWidget(path, T("Items"), WIDGET_SEPARATOR_TEXT);
    AddWidget(path, T("Timeless Equipment"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_CHEAT("TimelessEquipment"))
        .Options(CheckboxOptions().Tooltip(T("Allows any item to be equipped, regardless of age.\n"
                                           "Also allows child to use adult strength upgrades.")));
    AddWidget(path, T("Unrestricted Items"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_CHEAT("NoRestrictItems"))
        .Options(CheckboxOptions().Tooltip(T("Allows you to use any item at any location")));
    AddWidget(path, T("Super Tunic"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_CHEAT("SuperTunic"))
        .Options(CheckboxOptions().Tooltip(T("Makes every tunic have the effects of every other tunic.")));
    AddWidget(path, T("Fireproof Deku Shield"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_CHEAT("FireproofDekuShield"))
        .Options(CheckboxOptions().Tooltip(T("Prevents the Deku Shield from burning on contact with fire.")));
    AddWidget(path, T("Shield with Two-Handed Weapons"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_CHEAT("ShieldTwoHanded"))
        .Options(CheckboxOptions().Tooltip(
            T("This allows you to put up your shield with any two-handed weapon in hand except for Deku Sticks.")));
    AddWidget(path, T("Deku Sticks:"), WIDGET_CVAR_COMBOBOX)
        .CVar(CVAR_CHEAT("DekuStick"))
        .Options(ComboboxOptions().ComboMap(dekuStickCheat).DefaultIndex(DEKU_STICK_NORMAL));
    AddWidget(path, T("Bomb Timer Multiplier: %.2fx"), WIDGET_CVAR_SLIDER_FLOAT)
        .CVar(CVAR_CHEAT("BombTimerMultiplier"))
        .Options(FloatSliderOptions().Format("%.2f").Min(0.1f).Max(5.0f).DefaultValue(1.0f));
    AddWidget(path, T("Hookshot Everything"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_CHEAT("HookshotEverything"))
        .Options(CheckboxOptions().Tooltip(T("Makes every surface in the game hookshotable.")));
    AddWidget(path, T("Hookshot Reach Multiplier: %.2fx"), WIDGET_CVAR_SLIDER_FLOAT)
        .CVar(CVAR_CHEAT("HookshotReachMultiplier"))
        .Options(FloatSliderOptions().Format("%.2f").Min(1.0f).Max(5.0f));

    path.column = SECTION_COLUMN_2;
    AddWidget(path, T("Misc"), WIDGET_SEPARATOR_TEXT);
    AddWidget(path, T("No Clip"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_CHEAT("NoClip"))
        .Options(CheckboxOptions().Tooltip(T("Allows you to walk through walls.")));
    AddWidget(path, T("Climb Everything"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_CHEAT("ClimbEverything"))
        .Options(CheckboxOptions().Tooltip(T("Makes every surface in the game climbable.")));
    AddWidget(path, T("Moon Jump on L"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_CHEAT("MoonJumpOnL"))
        .Options(CheckboxOptions().Tooltip(T("Holding L makes you float into the air.")));
    AddWidget(path, T("No ReDead/Gibdo Freeze"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_CHEAT("NoRedeadFreeze"))
        .Options(
            CheckboxOptions().Tooltip(T("Prevents ReDeads and Gibdos from being able to freeze you with their scream.")));
    AddWidget(path, T("Keese/Guay don't Target You"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_CHEAT("NoKeeseGuayTarget"))
        .Options(CheckboxOptions().Tooltip(
            T("Keese and Guay no longer target you and simply ignore you as if you were wearing the "
            "Skull Mask.")));
    AddWidget(path, T("Disable Haunted Wasteland Sandstorm"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_CHEAT("DisableSandstorm"))
        .Options(CheckboxOptions().Tooltip(T("Disables sandstorm effect in Haunted Wasteland.")));
    AddWidget(path, T("Targetable Gold Skulltula"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_CHEAT("GSTargetable"))
        .Options(CheckboxOptions().Tooltip(T("Allows Z-Targeting Gold Skulltulas.")));

    AddWidget(path, T("Glitch Aids"), WIDGET_SEPARATOR_TEXT);
    AddWidget(path, T("Easy Frame Advancing with Pause"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_CHEAT("EasyFrameAdvance"))
        .Options(CheckboxOptions().Tooltip(
            T("Continue holding START button when unpausing to only advance a single frame and then re-pause.")));
    AddWidget(path, T("Easy ISG"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_CHEAT("EasyISG"))
        .Options(CheckboxOptions().Tooltip(T("Passive Infinite Sword Glitch\n"
                                           "It makes your sword's swing effect and hitbox stay active indefinitely.")));
    AddWidget(path, T("Easy QPA"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_CHEAT("EasyQPA"))
        .Options(CheckboxOptions().Tooltip(T("Gives you the glitched damage value of the quick put away glitch.")));
    AddWidget(path, T("Clear Cutscene Pointer"), WIDGET_BUTTON)
        .Callback([](WidgetInfo& info) { GameInteractor::RawAction::ClearCutscenePointer(); })
        .Options(ButtonOptions()
                     .Tooltip(T("Clears the cutscene pointer to a value safe for wrong warps."))
                     .Size(UIWidgets::Sizes::Inline));

    AddWidget(path, T("Despawn Timers"), WIDGET_SEPARATOR_TEXT);
    AddWidget(path, T("Drops Don't Despawn"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_CHEAT("DropsDontDie"))
        .Options(
            CheckboxOptions().Tooltip(T("Drops from enemies, grass, etc. don't disappear after a set amount of time.")));
    AddWidget(path, T("Fish don't Despawn"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_CHEAT("NoFishDespawn"))
        .Options(CheckboxOptions().Tooltip(T("Prevents fish from automatically despawning after a while when dropped.")));
    AddWidget(path, T("Bugs don't Despawn"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_CHEAT("NoBugsDespawn"))
        .Options(CheckboxOptions().Tooltip(T("Prevents bugs from automatically despawning after a while when dropped.")));

    AddWidget(path, T("Time of Day"), WIDGET_SEPARATOR_TEXT);
    AddWidget(path, T("Freeze Time"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_CHEAT("FreezeTime"))
        .Options(CheckboxOptions().Tooltip(T("Freezes the time of day.")));
    AddWidget(path, T("Time Sync"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_CHEAT("TimeSync"))
        .Options(CheckboxOptions().Tooltip(T("Syncs the in-game time with the real world time.")));

    AddWidget(path, T("Instant Age Change"), WIDGET_SEPARATOR_TEXT);
    AddWidget(path, T("Change Age"), WIDGET_BUTTON)
        .Options(ButtonOptions().Tooltip(T("Switches Link's age and reloads the area.")).Size(UIWidgets::Sizes::Inline))
        .Callback([](WidgetInfo& info) { SwitchAge(); });

    path.column = SECTION_COLUMN_3;
    AddWidget(path, T("Speed Modifier"), WIDGET_SEPARATOR_TEXT);
    AddWidget(path, T("Toggle modifier instead of holding"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_CHEAT("SpeedModifier.SpeedToggle"));
    AddWidget(path, T("Don't affect jump distance/velocity"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_CHEAT("SpeedModifier.DoesntChangeJump"));
    AddWidget(path, T("Multiplier:"), WIDGET_CVAR_SLIDER_FLOAT)
        .CVar(CVAR_CHEAT("SpeedModifier.Value"))
        .Options(FloatSliderOptions().IsPercentage().Min(0.01f).Max(5.0f).DefaultValue(1.0f).ShowButtons(true).Format(
            "%.0f%%"));
    AddWidget(path, T("Button Combination:"), WIDGET_CVAR_BTN_SELECTOR)
        .CVar(CVAR_CHEAT("SpeedModifier.Btn"))
        .Options(
            BtnSelectorOptions()
                .DefaultValue(BTN_CUSTOM_MODIFIER1)
                .Tooltip(T("Buttons that activate Speed Modifier 1.\n\n"
                         "If \"Toggle modifier instead of holding\" is off, hold this combo to apply the modifier.\n"
                         "If it is on, tap this combo to toggle the modifier on/off.")));

    AddWidget(path, T("Save States"), WIDGET_SEPARATOR_TEXT);
    AddWidget(path, T("\xef\x81\xb1 WARNING!!!! \xef\x81\xb1"), WIDGET_TEXT)
        .Options(TextOptions().Color(Colors::Orange));
    AddWidget(path,
              T("These are NOT like emulator states. They do not save your game progress "
              "and they WILL break across transitions and load zones (like doors). "
              "Support for related issues will not be provided."),
              WIDGET_TEXT);
    AddWidget(path, T("I promise I have read the warning"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_CHEAT("SaveStatePromise"))
        .Callback([](WidgetInfo& info) {
            CVarSetInteger(CVAR_CHEAT("SaveStatesEnabled"), 0);
            Ship::Context::GetRawInstance()->GetWindow()->GetGui()->SaveConsoleVariablesNextFrame();
        });
    AddWidget(path, T("I understand, enable save states"), WIDGET_CVAR_CHECKBOX)
        .PreFunc([](WidgetInfo& info) { info.isHidden = CVarGetInteger(CVAR_CHEAT("SaveStatePromise"), 0) == 0; })
        .CVar(CVAR_CHEAT("SaveStatesEnabled"))
        .Options(CheckboxOptions().Tooltip(T("F5 to save, F6 to change slots, F7 to load")));

    AddWidget(path, T("Beta Quest"), WIDGET_SEPARATOR_TEXT);
    AddWidget(path, T("Enable Beta Quest"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_CHEAT("EnableBetaQuest"))
        .PreFunc([](WidgetInfo& info) { info.options->disabled = !isBetaQuestEnabled; })
        .Callback([](WidgetInfo& info) {
            if (CVarGetInteger(CVAR_CHEAT("EnableBetaQuest"), 0) == 0) {
                CVarClear(CVAR_CHEAT("BetaQuestWorld"));
            } else {
                CVarSetInteger(CVAR_CHEAT("BetaQuestWorld"), 0);
            }
            std::reinterpret_pointer_cast<Ship::ConsoleWindow>(
                Ship::Context::GetRawInstance()->GetWindow()->GetGui()->GetGuiWindow("Console"))
                ->Dispatch("reset");
            Ship::Context::GetRawInstance()->GetWindow()->GetGui()->SaveConsoleVariablesNextFrame();
        })
        .Options(CheckboxOptions().Tooltip(T("Turns on OoT Beta Quest. *WARNING*: This will reset your game!")));
    AddWidget(path, T("Beta Quest World: %d"), WIDGET_CVAR_SLIDER_INT)
        .CVar(CVAR_CHEAT("BetaQuestWorld"))
        .PreFunc([](WidgetInfo& info) {
            info.options->disabled = info.isHidden = CVarGetInteger(CVAR_CHEAT("EnableBetaQuest"), 0) == 0;
        })
        .Callback([](WidgetInfo& info) {
            std::reinterpret_pointer_cast<Ship::ConsoleWindow>(
                Ship::Context::GetRawInstance()->GetWindow()->GetGui()->GetGuiWindow("Console"))
                ->Dispatch("reset");
            Ship::Context::GetRawInstance()->GetWindow()->GetGui()->SaveConsoleVariablesNextFrame();
        })
        .Options(IntSliderOptions().DefaultValue(0).Min(0).Max(8).Tooltip(
            T("Set the Beta Quest world to explore. *WARNING*: Changing this will reset your game!\n"
            "Ctrl+Click to type in a value.")));

    // Cosmetics Editor
    path.sidebarName = "Cosmetics Editor";
    AddSidebarEntry("Enhancements", path.sidebarName, 1);
    path.column = SECTION_COLUMN_1;
    AddWidget(path, T("Popout Cosmetics Editor Window"), WIDGET_WINDOW_BUTTON)
        .CVar(CVAR_WINDOW("CosmeticsEditor"))
        .RaceDisable(false)
        .WindowName("Cosmetics Editor")
        .HideInSearch(true)
        .Options(WindowButtonOptions().Tooltip(T("Enables the separate Cosmetics Editor Window.")));

    // Audio Editor
    path.sidebarName = "Audio Editor";
    AddSidebarEntry("Enhancements", path.sidebarName, 1);
    AddWidget(path, T("Popout Audio Editor Window"), WIDGET_WINDOW_BUTTON)
        .CVar(CVAR_WINDOW("AudioEditor"))
        .RaceDisable(false)
        .WindowName("Audio Editor")
        .HideInSearch(true)
        .Options(WindowButtonOptions().Tooltip(T("Enables the separate Audio Editor Window.")));

    // Gameplay Stats
    path.sidebarName = "Gameplay Stats";
    AddSidebarEntry("Enhancements", path.sidebarName, 2);
    AddWidget(path, T("Popout Gameplay Stats Window"), WIDGET_WINDOW_BUTTON)
        .CVar(CVAR_WINDOW("GameplayStats"))
        .RaceDisable(false)
        .WindowName("Gameplay Stats")
        .HideInSearch(true)
        .Options(WindowButtonOptions().Tooltip(T("Enables the separate Gameplay Stats Window.")));

    // Time Splits
    path.sidebarName = "Time Splits";
    AddSidebarEntry("Enhancements", path.sidebarName, 1);
    AddWidget(path, T("Popout Timesplits Settings"), WIDGET_WINDOW_BUTTON)
        .CVar("gWindows.Timesplits.Settings")
        .WindowName("Time Splits Settings Window");

    // Timers
    path.sidebarName = "Timers";
    AddSidebarEntry("Enhancements", path.sidebarName, 3);
    AddWidget(path, T("Toggle Timers Window"), WIDGET_WINDOW_BUTTON)
        .CVar(CVAR_WINDOW("TimeDisplayEnabled"))
        .RaceDisable(false)
        .WindowName("Additional Timers")
        .Options(WindowButtonOptions().Tooltip(T("Enables the separate Additional Timers Window.")));
    AddWidget(path, T("Font Scale: %.2fx"), WIDGET_CVAR_SLIDER_FLOAT)
        .CVar(CVAR_TIME_DISPLAY("FontScale"))
        .RaceDisable(false)
        .Callback([](WidgetInfo& info) { TimeDisplayInitSettings(); })
        .Options(FloatSliderOptions().Min(1.0f).Max(5.0f).DefaultValue(1.0f).Format("%.2fx"));
    AddWidget(path, T("Hide Background"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_TIME_DISPLAY("ShowWindowBG"))
        .RaceDisable(false)
        .Callback([](WidgetInfo& info) { TimeDisplayInitSettings(); });
    for (auto& timer : timeDisplayList) {
        AddWidget(path, timer.timeLabel, WIDGET_CVAR_CHECKBOX)
            .RaceDisable(false)
            .CVar(timer.timeEnable)
            .Callback([](WidgetInfo& info) { TimeDisplayUpdateDisplayOptions(); });
    }
}

} // namespace SohGui

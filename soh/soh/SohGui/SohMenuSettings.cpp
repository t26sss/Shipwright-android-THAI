#include "SohMenu.h"
#include "ThaiText.h"
#include "soh/Notification/Notification.h"
#include "soh/Enhancements/enhancementTypes.h"
#include "SohModals.h"
#include "soh/OTRGlobals.h"
#include <soh/GameVersions.h>
#include "soh/ResourceManagerHelpers.h"
#include "UIWidgets.hpp"
#include "soh/Enhancements/controls/SohInputEditorWindow.h"
#include <ship/controller/controldeck/ControlDeck.h>
#include <ship/Context.h>
#include <algorithm>
#ifdef __ANDROID__
#include <jni.h>
#include <SDL2/SDL.h>
#endif

extern "C" {
#include "include/z64audio.h"
#include "variables.h"
}

namespace SohGui {

extern std::shared_ptr<SohMenu> mSohMenu;
extern std::shared_ptr<SohModalWindow> mModalWindow;
using namespace UIWidgets;

static std::map<int32_t, const char*> imguiScaleOptions = {
    { 0, T("Small") },
    { 1, T("Normal") },
    { 2, T("Large") },
    { 3, T("X-Large") },
};

static const std::map<int32_t, const char*> menuThemeOptions = {
    { UIWidgets::Colors::Red, T("Red") },
    { UIWidgets::Colors::DarkRed, T("Dark Red") },
    { UIWidgets::Colors::Orange, T("Orange") },
    { UIWidgets::Colors::Green, T("Green") },
    { UIWidgets::Colors::DarkGreen, T("Dark Green") },
    { UIWidgets::Colors::LightBlue, T("Light Blue") },
    { UIWidgets::Colors::Blue, T("Blue") },
    { UIWidgets::Colors::DarkBlue, T("Dark Blue") },
    { UIWidgets::Colors::Indigo, T("Indigo") },
    { UIWidgets::Colors::Violet, T("Violet") },
    { UIWidgets::Colors::Purple, T("Purple") },
    { UIWidgets::Colors::Brown, T("Brown") },
    { UIWidgets::Colors::Gray, T("Gray") },
    { UIWidgets::Colors::DarkGray, T("Dark Gray") },
};

static const std::map<int32_t, const char*> textureFilteringMap = {
    { Fast::FILTER_THREE_POINT, T("Three-Point") },
    { Fast::FILTER_LINEAR, T("Linear") },
    { Fast::FILTER_NONE, T("None") },
};

static const std::map<int32_t, const char*> notificationPosition = {
    { 0, T("Top Left") }, { 1, T("Top Right") }, { 2, T("Bottom Left") }, { 3, T("Bottom Right") }, { 4, T("Hidden") },
};

static const std::map<int32_t, const char*> bootSequenceLabels = {
    { BOOTSEQUENCE_DEFAULT, T("Default") },        { BOOTSEQUENCE_AUTHENTIC, T("Authentic") },
    { BOOTSEQUENCE_FILESELECT, T("File Select") }, { BOOTSEQUENCE_DEBUGWARPSCREEN, T("Debug Warp Screen") },
    { BOOTSEQUENCE_WARPPOINT, T("Warp Point") },
};

#if defined(__ANDROID__)
static const std::map<int32_t, const char*> touchFaceButtonLayoutMap = {
    { 0, T("ABXY (Nintendo)") },
    { 1, T("BAYX (Xbox)") },
    { 2, T("GC Layout") },
};

static void SetAndroidTouchControlsDisabled(bool disabled) {
    JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
    jobject activity = (jobject)SDL_AndroidGetActivity();
    if (env == nullptr || activity == nullptr) {
        return;
    }

    jclass activityClass = env->GetObjectClass(activity);
    if (activityClass == nullptr) {
        env->DeleteLocalRef(activity);
        return;
    }

    jmethodID method = env->GetMethodID(activityClass, "setTouchControlsDisabledFromNative", "(Z)V");
    if (method != nullptr) {
        env->CallVoidMethod(activity, method, disabled ? JNI_TRUE : JNI_FALSE);
    }

    env->DeleteLocalRef(activityClass);
    env->DeleteLocalRef(activity);
}

static void SetAndroidTouchFaceButtonLayout(int32_t layout) {
    JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
    jobject activity = (jobject)SDL_AndroidGetActivity();
    if (env == nullptr || activity == nullptr) {
        return;
    }

    jclass activityClass = env->GetObjectClass(activity);
    if (activityClass != nullptr) {
        jmethodID method =
            env->GetMethodID(activityClass, "setTouchFaceButtonLayoutFromNative", "(I)V");
        if (method != nullptr) {
            env->CallVoidMethod(activity, method, static_cast<jint>(layout));
        }
        env->DeleteLocalRef(activityClass);
    }
    env->DeleteLocalRef(activity);
}

static void SetAndroidTouchLeftStickFloating(bool floating) {
    JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
    jobject activity = (jobject)SDL_AndroidGetActivity();
    if (env == nullptr || activity == nullptr) {
        return;
    }

    jclass activityClass = env->GetObjectClass(activity);
    if (activityClass != nullptr) {
        jmethodID method = env->GetMethodID(activityClass, "setTouchLeftStickFloatingFromNative", "(Z)V");
        if (method != nullptr) {
            env->CallVoidMethod(activity, method, floating ? JNI_TRUE : JNI_FALSE);
        }
        env->DeleteLocalRef(activityClass);
    }
    env->DeleteLocalRef(activity);
}

// Calls a no-argument void method on the Android activity (used by the touch layout editor).
static void CallAndroidActivityVoid(const char* methodName) {
    JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
    jobject activity = (jobject)SDL_AndroidGetActivity();
    if (env == nullptr || activity == nullptr) {
        return;
    }

    jclass activityClass = env->GetObjectClass(activity);
    if (activityClass != nullptr) {
        jmethodID method = env->GetMethodID(activityClass, methodName, "()V");
        if (method != nullptr) {
            env->CallVoidMethod(activity, method);
        }
        env->DeleteLocalRef(activityClass);
    }
    env->DeleteLocalRef(activity);
}

static void OpenAndroidDataFolderChooser() {
    JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
    jobject activity = (jobject)SDL_AndroidGetActivity();
    if (env == nullptr || activity == nullptr) {
        return;
    }

    jclass activityClass = env->GetObjectClass(activity);
    if (activityClass == nullptr) {
        env->DeleteLocalRef(activity);
        return;
    }

    jmethodID method = env->GetMethodID(activityClass, "changeDataFolderFromNative", "()V");
    if (method != nullptr) {
        env->CallVoidMethod(activity, method);
    }

    env->DeleteLocalRef(activityClass);
    env->DeleteLocalRef(activity);
}
#endif

const char* GetGameVersionString(uint32_t index) {
    uint32_t gameVersion = ResourceMgr_GetGameVersion(index);
    switch (gameVersion) {
        case OOT_NTSC_US_10:
            return "NTSC 1.0";
        case OOT_NTSC_US_11:
            return "NTSC 1.1";
        case OOT_NTSC_US_12:
            return "NTSC 1.2";
        case OOT_NTSC_US_GC:
            return "NTSC-U GC";
        case OOT_NTSC_JP_GC:
            return "NTSC-J GC";
        case OOT_NTSC_JP_GC_CE:
            return "NTSC-J GC (Collector's Edition)";
        case OOT_NTSC_US_MQ:
            return "NTSC-U MQ";
        case OOT_NTSC_JP_MQ:
            return "NTSC-J MQ";
        case OOT_PAL_10:
            return "PAL 1.0";
        case OOT_PAL_11:
            return "PAL 1.1";
        case OOT_PAL_GC:
            return "PAL GC";
        case OOT_PAL_MQ:
            return "PAL MQ";
        case OOT_PAL_GC_DBG1:
        case OOT_PAL_GC_DBG2:
            return "PAL GC-D";
        case OOT_PAL_GC_MQ_DBG:
            return "PAL MQ-D";
        case OOT_IQUE_CN:
            return "IQUE CN";
        case OOT_IQUE_TW:
            return "IQUE TW";
        default:
            return "UNKNOWN";
    }
}

#include "message_data_static.h"
extern "C" MessageTableEntry* sNesMessageEntryTablePtr;
extern "C" MessageTableEntry* sGerMessageEntryTablePtr;
extern "C" MessageTableEntry* sFraMessageEntryTablePtr;
extern "C" MessageTableEntry* sJpnMessageEntryTablePtr;

static const std::array<MessageTableEntry**, LANGUAGE_MAX> messageTables = {
    &sNesMessageEntryTablePtr, &sGerMessageEntryTablePtr, &sFraMessageEntryTablePtr, &sJpnMessageEntryTablePtr
};

void SohMenu::UpdateLanguageMap(std::map<int32_t, const char*>& languageMap) {
    for (int32_t i = LANGUAGE_ENG; i < LANGUAGE_MAX; i++) {
        if (*messageTables.at(i) != NULL) {
            if (!languageMap.contains(i)) {
                languageMap.insert(std::make_pair(i, languages.at(i)));
            }
        } else {
            languageMap.erase(i);
        }
    }
}

void SohMenu::AddMenuSettings() {
    // Add Settings Menu
    AddMenuEntry("Settings", CVAR_SETTING("Menu.SettingsSidebarSection"));
    AddSidebarEntry("Settings", "General", 2);
    WidgetPath path = { "Settings", "General", SECTION_COLUMN_1 };

    // General - Settings
    AddWidget(path, T("Menu Settings"), WIDGET_SEPARATOR_TEXT);
    AddWidget(path, T("Menu Theme"), WIDGET_CVAR_COMBOBOX)
        .CVar(CVAR_SETTING("Menu.Theme"))
        .RaceDisable(false)
        .Options(ComboboxOptions()
                     .Tooltip(T("Changes the Theme of the Menu Widgets."))
                     .ComboMap(menuThemeOptions)
                     .DefaultIndex(Colors::LightBlue));
#if not defined(__SWITCH__) and not defined(__WIIU__)
    AddWidget(path, T("Menu Controller Navigation"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_IMGUI_CONTROLLER_NAV)
        .RaceDisable(false)
        .Options(CheckboxOptions().Tooltip(
            T("Allows controller navigation of the port menu (Settings, Enhancements,...)\nCAUTION: "
            "This will disable game inputs while the menu is visible.\n\nD-pad to move between "
            "items, A to select, B to move up in scope.")));
    AddWidget(path, T("Allow background inputs"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ALLOW_BACKGROUND_INPUTS)
        .RaceDisable(false)
        .Callback([](WidgetInfo& info) {
            SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS,
                        CVarGetInteger(CVAR_ALLOW_BACKGROUND_INPUTS, 1) ? "1" : "0");
        })
        .Options(CheckboxOptions()
                     .Tooltip(T("Allows controller inputs to be picked up by the game even when the game window isn't "
                              "the focused window."))
                     .DefaultValue(1));
    AddWidget(path, T("Menu Background Opacity"), WIDGET_CVAR_SLIDER_FLOAT)
        .CVar(CVAR_SETTING("Menu.BackgroundOpacity"))
        .RaceDisable(false)
        .Options(FloatSliderOptions().DefaultValue(0.85f).IsPercentage().Tooltip(
            T("Sets the opacity of the background of the port menu.")));

    AddWidget(path, T("General Settings"), WIDGET_SEPARATOR_TEXT);
    AddWidget(path, T("Cursor Always Visible"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_SETTING("CursorVisibility"))
        .RaceDisable(false)
        .Callback([](WidgetInfo& info) {
            Ship::Context::GetRawInstance()->GetWindow()->SetForceCursorVisibility(
                CVarGetInteger(CVAR_SETTING("CursorVisibility"), 0));
        })
        .Options(CheckboxOptions().Tooltip(T("Makes the cursor always visible, even in full screen.")));
#endif
    AddWidget(path, T("Search In Sidebar"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_SETTING("Menu.SidebarSearch"))
        .RaceDisable(false)
        .Callback([](WidgetInfo& info) {
            if (CVarGetInteger(CVAR_SETTING("Menu.SidebarSearch"), 0)) {
                mSohMenu->InsertSidebarSearch();
            } else {
                mSohMenu->RemoveSidebarSearch();
            }
        })
        .Options(CheckboxOptions().Tooltip(
            T("Displays the Search menu as a sidebar entry in Settings instead of in the header.")));
    AddWidget(path, T("Search Input Autofocus"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_SETTING("Menu.SearchAutofocus"))
        .RaceDisable(false)
        .Options(CheckboxOptions().Tooltip(
            T("Search input box gets autofocus when visible. Does not affect using other widgets.")));
    AddWidget(path, T("Reset Button Combination:"), WIDGET_CVAR_BTN_SELECTOR)
        .CVar("gSettings.ResetBtn")
        .Options(BtnSelectorOptions().DefaultValue(BTN_CUSTOM_MODIFIER2));
#if defined(__ANDROID__)
    AddWidget(path, T("Current Data Folder"), WIDGET_CUSTOM).CustomFunction([](WidgetInfo& info) {
        std::string dataFolderPath = Ship::Context::GetAppDirectoryPath();
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 0.7f), "Current Data Folder");
        ImGui::PushTextWrapPos(ImGui::GetContentRegionAvail().x);
        ImGui::TextUnformatted(dataFolderPath.c_str());
        ImGui::PopTextWrapPos();
    });
    AddWidget(path, T("Change Data Folder"), WIDGET_BUTTON)
        .RaceDisable(false)
        .Callback([](WidgetInfo& info) { OpenAndroidDataFolderChooser(); })
        .Options(ButtonOptions().Tooltip(T("Choose where Android stores saves, mods, settings, and support files.")));
#else
    AddWidget(path, T("Open App Files Folder"), WIDGET_BUTTON)
        .RaceDisable(false)
        .Callback([](WidgetInfo& info) {
            std::string filesPath = Ship::Context::GetRawInstance()->GetAppDirectoryPath();
            SDL_OpenURL(std::string("file:///" + std::filesystem::absolute(filesPath).string()).c_str());
        })
        .Options(ButtonOptions().Tooltip(T("Opens the folder that contains the save and mods folders, etc.")));
#endif

    AddWidget(path, T("Boot"), WIDGET_SEPARATOR_TEXT);
    AddWidget(path, T("Boot Sequence"), WIDGET_CVAR_COMBOBOX)
        .CVar(CVAR_SETTING("BootSequence"))
        .RaceDisable(false)
        .Options(ComboboxOptions()
                     .DefaultIndex(BOOTSEQUENCE_DEFAULT)
                     .LabelPosition(LabelPositions::Far)
                     .ComponentAlignment(ComponentAlignments::Right)
                     .ComboMap(bootSequenceLabels)
                     .Tooltip(T("Configure what happens when starting or resetting the game.\n\n"
                              "Default: LUS logo -> N64 logo\n"
                              "Authentic: N64 logo only\n"
                              "File Select: Skip to file select menu\n"
                              "Debug Warp Screen: Skip to the debug warp screen\n"
                              "Warp Point: Skip to active warp point (if set), see Dev Tools -> General")));

    AddWidget(path, T("Languages"), WIDGET_SEPARATOR_TEXT);
    AddWidget(path, T("Translate Title Screen"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_SETTING("TitleScreenTranslation"))
        .RaceDisable(false);
    AddWidget(path, T("Language"), WIDGET_CVAR_COMBOBOX)
        .CVar(CVAR_SETTING("Languages"))
        .RaceDisable(false)
        .PreFunc([](WidgetInfo& info) {
            auto options = std::static_pointer_cast<UIWidgets::ComboboxOptions>(info.options);
            SohMenu::UpdateLanguageMap(options->comboMap);
        })
        .Options(ComboboxOptions()
                     .LabelPosition(LabelPositions::Far)
                     .ComponentAlignment(ComponentAlignments::Right)
                     .ComboMap(languages)
                     .DefaultIndex(LANGUAGE_ENG));
    AddWidget(path, T("Accessibility"), WIDGET_SEPARATOR_TEXT);
#if defined(_WIN32) || defined(__APPLE__) || defined(ESPEAK)
    AddWidget(path, T("Text to Speech"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_SETTING("A11yTTS"))
        .RaceDisable(false)
        .Options(CheckboxOptions().Tooltip(T("Enables text to speech for in-game dialogue")));
#endif
    AddWidget(path, T("Disable Idle Camera Re-Centering"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_SETTING("A11yDisableIdleCam"))
        .RaceDisable(false)
        .Options(CheckboxOptions().Tooltip(T("Disables the automatic re-centering of the camera when idle.")));
    AddWidget(path, T("Disable Screen Flash for Finishing Blow"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_SETTING("A11yNoScreenFlashForFinishingBlow"))
        .RaceDisable(false)
        .Options(CheckboxOptions().Tooltip(T("Disables the white screen flash on enemy kill.")));
    AddWidget(path, T("Disable Jabu Wobble"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_SETTING("A11yNoJabuWobble"))
        .RaceDisable(false)
        .Options(CheckboxOptions().Tooltip(T("Disable the geometry wobble and camera distortion inside Jabu.")));
    AddWidget(path, T("Disable Heat Haze"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_SETTING("A11yNoHeatHaze"))
        .RaceDisable(false)
        .Options(CheckboxOptions().Tooltip(T("Disable the heat haze distortion effect in Death Mountain / Fire Temple.")));
    AddWidget(path, T("EXPERIMENTAL"), WIDGET_SEPARATOR_TEXT).Options(TextOptions().Color(Colors::Orange));
    AddWidget(path, T("Menu Scale"), WIDGET_CUSTOM).CustomFunction([](WidgetInfo& info) {
        const auto theme = static_cast<Colors>(CVarGetInteger(CVAR_SETTING("Menu.Theme"), Colors::LightBlue));
        const float resetButtonWidth =
            ImGui::CalcTextSize(ICON_FA_UNDO).x + ImGui::GetStyle().FramePadding.x * 2.0f;
        const float sliderWidth =
            std::max(120.0f, ImGui::GetContentRegionAvail().x - resetButtonWidth - ImGui::GetStyle().ItemSpacing.x);

        CVarSliderFloat("Menu Scale: %.2fx", CVAR_SETTING("ImGuiScale.Value"),
                        FloatSliderOptions()
                            .Min(0.65f)
                            .Max(2.5f)
                            .Step(0.05f)
                            .DefaultValue(1.0f)
                            .Format("%.2fx")
                            .Tooltip(T("Changes the scaling of the menu elements."))
                            .ShowButtons(true)
                            .Size(ImVec2(sliderWidth, 0.0f))
                            .ComponentAlignment(ComponentAlignments::Left)
                            .LabelPosition(LabelPositions::Above)
                            .Color(theme));

        ImGui::SameLine();
        if (Button(ICON_FA_UNDO "##ResetMenuScale",
                   ButtonOptions()
                       .Tooltip(T("Reset menu scale to 100%."))
                       .Size(Sizes::Inline)
                       .Color(theme))) {
            CVarSetFloat(CVAR_SETTING("ImGuiScale.Value"), 1.0f);
            Ship::Context::GetRawInstance()->GetWindow()->GetGui()->SaveConsoleVariablesNextFrame();
        }
        OTRGlobals::Instance->ScaleImGui();
    });

    // General - About
    path.column = SECTION_COLUMN_2;

    AddWidget(path, T("About"), WIDGET_SEPARATOR_TEXT);
    AddWidget(path, T("Ship Of Harkinian"), WIDGET_TEXT);
#if defined(__ANDROID__) && defined(ANDROID_APP_VERSION_NAME)
    AddWidget(path, T("Android App Version"), WIDGET_TEXT);
    AddWidget(path, ANDROID_APP_VERSION_NAME, WIDGET_TEXT);
#endif
    if (gGitCommitTag[0] != 0) {
        AddWidget(path, gBuildVersion, WIDGET_TEXT);
    } else {
        AddWidget(path, ("Branch: " + std::string(gGitBranch)), WIDGET_TEXT);
        AddWidget(path, ("Commit: " + std::string(gGitCommitHash)), WIDGET_TEXT);
    }
    for (uint32_t i = 0; i < ResourceMgr_GetNumGameVersions(); i++) {
        AddWidget(path, GetGameVersionString(i), WIDGET_TEXT);
    }

    // Audio Settings
    path.sidebarName = "Audio";
    path.column = SECTION_COLUMN_1;
    AddSidebarEntry("Settings", "Audio", 3);

    AddWidget(path, T("Master Volume: %d %%"), WIDGET_CVAR_SLIDER_INT)
        .CVar(CVAR_SETTING("Volume.Master"))
        .RaceDisable(false)
        .Options(IntSliderOptions().Min(0).Max(100).DefaultValue(40).ShowButtons(true).Format(""));
    AddWidget(path, T("Main Music Volume: %d %%"), WIDGET_CVAR_SLIDER_INT)
        .CVar(CVAR_SETTING("Volume.MainMusic"))
        .RaceDisable(false)
        .Options(IntSliderOptions().Min(0).Max(100).DefaultValue(100).ShowButtons(true).Format(""))
        .Callback([](WidgetInfo& info) {
            Audio_SetGameVolume(SEQ_PLAYER_BGM_MAIN,
                                ((float)CVarGetInteger(CVAR_SETTING("Volume.MainMusic"), 100) / 100.0f));
        });
    AddWidget(path, T("Sub Music Volume: %d %%"), WIDGET_CVAR_SLIDER_INT)
        .CVar(CVAR_SETTING("Volume.SubMusic"))
        .RaceDisable(false)
        .Options(IntSliderOptions().Min(0).Max(100).DefaultValue(100).ShowButtons(true).Format(""))
        .Callback([](WidgetInfo& info) {
            Audio_SetGameVolume(SEQ_PLAYER_BGM_SUB,
                                ((float)CVarGetInteger(CVAR_SETTING("Volume.SubMusic"), 100) / 100.0f));
        });
    AddWidget(path, T("Fanfare Volume: %d %%"), WIDGET_CVAR_SLIDER_INT)
        .CVar(CVAR_SETTING("Volume.Fanfare"))
        .RaceDisable(false)
        .Options(IntSliderOptions().Min(0).Max(100).DefaultValue(100).ShowButtons(true).Format(""))
        .Callback([](WidgetInfo& info) {
            Audio_SetGameVolume(SEQ_PLAYER_FANFARE,
                                ((float)CVarGetInteger(CVAR_SETTING("Volume.Fanfare"), 100) / 100.0f));
        });
    AddWidget(path, T("Sound Effects Volume: %d %%"), WIDGET_CVAR_SLIDER_INT)
        .CVar(CVAR_SETTING("Volume.SFX"))
        .RaceDisable(false)
        .Options(IntSliderOptions().Min(0).Max(100).DefaultValue(100).ShowButtons(true).Format(""))
        .Callback([](WidgetInfo& info) {
            Audio_SetGameVolume(SEQ_PLAYER_SFX, ((float)CVarGetInteger(CVAR_SETTING("Volume.SFX"), 100) / 100.0f));
        });
    AddWidget(path, T("Audio API (Needs reload)"), WIDGET_AUDIO_BACKEND).RaceDisable(false);

    // Graphics Settings
    static int32_t maxFps = 360;
    const char* tooltip =
        T("Uses Matrix Interpolation to create extra frames, resulting in smoother graphics. This is "
        "purely visual and does not impact game logic, execution of glitches, etc.\n\nA higher target "
        "FPS than your monitor's refresh rate will waste resources, and might give a worse result.");
    path.sidebarName = "Graphics";
    AddSidebarEntry("Settings", "Graphics", 3);
    AddWidget(path, T("Graphics Options"), WIDGET_SEPARATOR_TEXT);
    AddWidget(path, T("Toggle Fullscreen"), WIDGET_BUTTON)
        .RaceDisable(false)
        .Callback([](WidgetInfo& info) { Ship::Context::GetRawInstance()->GetWindow()->ToggleFullscreen(); })
        .Options(ButtonOptions().Tooltip(T("Toggles Fullscreen On/Off.")));
    AddWidget(path, T("Internal Resolution"), WIDGET_CVAR_SLIDER_FLOAT)
        .CVar(CVAR_INTERNAL_RESOLUTION)
        .RaceDisable(false)
        .Callback([](WidgetInfo& info) {
            Ship::Context::GetRawInstance()->GetWindow()->SetResolutionMultiplier(
                CVarGetFloat(CVAR_INTERNAL_RESOLUTION, 1));
        })
        .PreFunc([](WidgetInfo& info) {
            if (mSohMenu->disabledMap.at(DISABLE_FOR_ADVANCED_RESOLUTION_ON).active &&
                mSohMenu->disabledMap.at(DISABLE_FOR_VERTICAL_RES_TOGGLE_ON).active) {
                info.activeDisables.push_back(DISABLE_FOR_ADVANCED_RESOLUTION_ON);
                info.activeDisables.push_back(DISABLE_FOR_VERTICAL_RES_TOGGLE_ON);
            } else if (mSohMenu->disabledMap.at(DISABLE_FOR_LOW_RES_MODE_ON).active) {
                info.activeDisables.push_back(DISABLE_FOR_LOW_RES_MODE_ON);
            }
        })
        .Options(
            FloatSliderOptions()
                .Tooltip(T("Multiplies your output resolution by the value inputted, as a more intensive but effective "
                         "form of anti-aliasing."))
                .ShowButtons(false)
                .IsPercentage()
                .Min(0.5f)
                .Max(2.0f));
#ifndef __WIIU__
    AddWidget(path, T("Anti-aliasing (MSAA)"), WIDGET_CVAR_SLIDER_INT)
        .CVar(CVAR_MSAA_VALUE)
        .RaceDisable(false)
        .Callback([](WidgetInfo& info) {
            Ship::Context::GetRawInstance()->GetWindow()->SetMsaaLevel(CVarGetInteger(CVAR_MSAA_VALUE, 1));
        })
        .Options(
            IntSliderOptions()
                .Tooltip(T("Activates MSAA (multi-sample anti-aliasing) from 2x up to 8x, to smooth the edges of "
                         "rendered geometry.\n"
                         "Higher sample count will result in smoother edges on models, but may reduce performance."))
                .Min(1)
                .Max(8)
                .DefaultValue(1));
#endif
    auto fps = CVarGetInteger(CVAR_SETTING("InterpolationFPS"), 20);
    const char* fpsFormat = fps == 20 ? T("Original (%d)") : "%d";
    AddWidget(path, T("Current FPS"), WIDGET_CVAR_SLIDER_INT)
        .CVar(CVAR_SETTING("InterpolationFPS"))
        .RaceDisable(false)
        .Callback([](WidgetInfo& info) {
            auto options = std::static_pointer_cast<IntSliderOptions>(info.options);
            int32_t defaultValue = options->defaultValue;
            if (CVarGetInteger(info.cVar, defaultValue) == defaultValue) {
                options->format = T("Original (%d)");
            } else {
                options->format = "%d";
            }
        })
        .PreFunc([](WidgetInfo& info) {
            if (mSohMenu->disabledMap.at(DISABLE_FOR_MATCH_REFRESH_RATE_ON).active)
                info.activeDisables.push_back(DISABLE_FOR_MATCH_REFRESH_RATE_ON);
        })
        .Options(IntSliderOptions().Tooltip(tooltip).Min(20).Max(maxFps).DefaultValue(20).Format(fpsFormat));
    AddWidget(path, T("Match Refresh Rate"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_SETTING("MatchRefreshRate"))
        .RaceDisable(false)
        .Options(CheckboxOptions().Tooltip(T("Matches interpolation value to the refresh rate of your display.")));
    AddWidget(path, T("Renderer API (Needs reload)"), WIDGET_VIDEO_BACKEND).RaceDisable(false);
    AddWidget(path, T("Enable Vsync"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_VSYNC_ENABLED)
        .RaceDisable(false)
        .PreFunc([](WidgetInfo& info) { info.isHidden = mSohMenu->disabledMap.at(DISABLE_FOR_NO_VSYNC).active; })
        .Options(CheckboxOptions()
                     .Tooltip(T("Removes tearing, but clamps your max FPS to your display's refresh rate."))
                     .DefaultValue(true));
    AddWidget(path, T("Windowed Fullscreen"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_SDL_WINDOWED_FULLSCREEN)
        .RaceDisable(false)
        .PreFunc([](WidgetInfo& info) {
            info.isHidden = mSohMenu->disabledMap.at(DISABLE_FOR_NO_WINDOWED_FULLSCREEN).active;
        })
        .Options(CheckboxOptions().Tooltip(T("Enables Windowed Fullscreen Mode.")));
    AddWidget(path, T("Allow multi-windows"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENABLE_MULTI_VIEWPORTS)
        .RaceDisable(false)
        .PreFunc(
            [](WidgetInfo& info) { info.isHidden = mSohMenu->disabledMap.at(DISABLE_FOR_NO_MULTI_VIEWPORT).active; })
        .Options(CheckboxOptions()
                     .Tooltip(T("Allows multiple windows to be opened at once. Requires a reload to take effect."))
                     .DefaultValue(true));
    AddWidget(path, T("Texture Filter (Needs reload)"), WIDGET_CVAR_COMBOBOX)
        .CVar(CVAR_TEXTURE_FILTER)
        .RaceDisable(false)
        .Options(ComboboxOptions().Tooltip(T("Sets the applied Texture Filtering.")).ComboMap(textureFilteringMap));

    path.column = SECTION_COLUMN_2;
    AddWidget(path, T("Advanced Graphics Options"), WIDGET_SEPARATOR_TEXT);

    // Controls
    path.sidebarName = "Controls";
    path.column = SECTION_COLUMN_1;
    AddSidebarEntry("Settings", "Controls", 2);
    AddWidget(path, T("Manual Jump (Experimental)"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("ManualJump"))
        .Options(CheckboxOptions().Tooltip(T("Experimental: enable grounded jumps with your Jump binding. Disabled by "
                                          "default and during race lockout. Configure the button in Manual Jump Bindings.")));
    AddWidget(path, T("Manual Jump Bindings"), WIDGET_BUTTON)
        .Callback([](WidgetInfo& info) {
            auto window = std::dynamic_pointer_cast<SohInputEditorWindow>(
                Ship::Context::GetRawInstance()->GetWindow()->GetGui()->GetGuiWindow("Configure Controller"));
            if (window != nullptr) {
                window->OpenManualJumpMenu();
            }
        })
        .Options(ButtonOptions().Size(Sizes::Inline));
    AddWidget(path, T("Clear Devices"), WIDGET_BUTTON)
        .Callback([](WidgetInfo& info) {
            SohGui::mModalWindow->RegisterPopup(
                T("Clear Config"),
                T("This will completely erase the controls config, including registered devices.\nContinue?"), "Clear",
                "Cancel",
                []() {
                    Ship::Context::GetRawInstance()->GetConsoleVariables()->ClearBlock(CVAR_PREFIX_SETTING
                                                                                       ".Controllers");
                    uint8_t bits = 0;
                    Ship::Context::GetRawInstance()->GetControlDeck()->Init(&bits);
                },
                nullptr);
        })
        .Options(ButtonOptions().Size(Sizes::Inline));
    AddWidget(path, T("Controller Bindings"), WIDGET_SEPARATOR_TEXT);
    AddWidget(path, T("Popout Bindings Window"), WIDGET_WINDOW_BUTTON)
        .CVar(CVAR_WINDOW("ControllerConfiguration"))
        .RaceDisable(false)
        .WindowName("Configure Controller")
        .HideInSearch(true)
        .Options(WindowButtonOptions().Tooltip(T("Enables the separate Bindings Window.")));

#if defined(__ANDROID__)
    // Touch Controls
    path.sidebarName = "Touch Controls";
    path.column = SECTION_COLUMN_1;
    AddSidebarEntry("Settings", path.sidebarName, 2);
    AddWidget(path, T("Touch Face Buttons"), WIDGET_CVAR_COMBOBOX)
        .CVar(CVAR_SETTING("TouchControls.FaceButtonLayout"))
        .RaceDisable(false)
        .Callback([](WidgetInfo& info) {
            SetAndroidTouchFaceButtonLayout(
                CVarGetInteger(CVAR_SETTING("TouchControls.FaceButtonLayout"), 0));
        })
        .Options(ComboboxOptions()
                     .ComboMap(touchFaceButtonLayoutMap)
                     .DefaultIndex(0)
                     .Tooltip(T("Choose ABXY (Nintendo), BAYX (Xbox), or GC Layout touch-button placement.")));
    AddWidget(path, T("Floating Left Stick"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_SETTING("TouchControls.FloatingLeftStick"))
        .RaceDisable(false)
        .Callback([](WidgetInfo& info) {
            SetAndroidTouchLeftStickFloating(
                CVarGetInteger(CVAR_SETTING("TouchControls.FloatingLeftStick"), 1) != 0);
        })
        .Options(CheckboxOptions().DefaultValue(true).Tooltip(
            T("When disabled, the left touch stick stays fixed in the lower-left corner.")));
    AddWidget(path, T("Edit Touch Button Positions"), WIDGET_BUTTON)
        .RaceDisable(false)
        .Callback([](WidgetInfo& info) {
            // Close the menu first so the on-screen buttons can be dragged.
            std::shared_ptr<Menu> menu =
                static_pointer_cast<Menu>(Ship::Context::GetRawInstance()->GetWindow()->GetGui()->GetMenu());
            if (menu != nullptr && !menu->IsMenuPopped()) {
                menu->ToggleVisibility();
            }
            CallAndroidActivityVoid("openTouchLayoutEditorFromNative");
        })
        .Options(ButtonOptions().Tooltip(
            T("Closes this menu and lets you drag the touch buttons and stick. Long-press the touch toggle "
            "button to save and exit.")));
    AddWidget(path, T("Adjust Touch Button Size"), WIDGET_BUTTON)
        .RaceDisable(false)
        .Callback([](WidgetInfo& info) { CallAndroidActivityVoid("openTouchSizeEditorFromNative"); })
        .Options(ButtonOptions().Tooltip(T("Choose a touch button or the stick and change its size (50%-200%).")));
    AddWidget(path, T("Reset Touch Layout"), WIDGET_BUTTON)
        .RaceDisable(false)
        .Callback([](WidgetInfo& info) { CallAndroidActivityVoid("resetTouchLayoutFromNative"); })
        .Options(ButtonOptions().Tooltip(T("Restores the default position and size of all touch buttons.")));
    AddWidget(path, T("Separate Roll Button"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("SeparateRollButton"))
        .Options(CheckboxOptions().Tooltip(
            T("A no longer rolls, hops or backflips. Only the on-screen Roll button does. Disabled during race "
              "lockout.")));
    AddWidget(path, T("Sprint Button (Stamina)"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_ENHANCEMENT("TouchSprint"))
        .Options(CheckboxOptions().Tooltip(
            T("Hold the on-screen Run button to run faster. Stamina drains while sprinting; at zero you get tired "
              "and move slower until it recovers. Disabled during race lockout.")));
    AddWidget(path, T("Disable Touch Controls"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_SETTING("TouchControls.Disabled"))
        .RaceDisable(false)
        .Callback([](WidgetInfo& info) {
            SetAndroidTouchControlsDisabled(CVarGetInteger(CVAR_SETTING("TouchControls.Disabled"), 0) != 0);
        })
        .Options(CheckboxOptions().Tooltip(T("Hides the Android touch controls and eye button.")));

    // Keep Android's persisted overlay state aligned with the native CVars after relaunches.
    SetAndroidTouchFaceButtonLayout(CVarGetInteger(CVAR_SETTING("TouchControls.FaceButtonLayout"), 0));
    SetAndroidTouchLeftStickFloating(
        CVarGetInteger(CVAR_SETTING("TouchControls.FloatingLeftStick"), 1) != 0);
    SetAndroidTouchControlsDisabled(CVarGetInteger(CVAR_SETTING("TouchControls.Disabled"), 0) != 0);
#endif

    // Input Viewer
    path.sidebarName = "Input Viewer";
    AddSidebarEntry("Settings", path.sidebarName, 3);
    AddWidget(path, T("Input Viewer"), WIDGET_SEPARATOR_TEXT);
    AddWidget(path, T("Toggle Input Viewer"), WIDGET_WINDOW_BUTTON)
        .CVar(CVAR_WINDOW("InputViewer"))
        .RaceDisable(false)
        .WindowName("Input Viewer")
        .HideInSearch(true)
        .Options(WindowButtonOptions().Tooltip(T("Toggles the Input Viewer.")).EmbedWindow(false));

    AddWidget(path, T("Input Viewer Settings"), WIDGET_SEPARATOR_TEXT);
    AddWidget(path, T("Popout Input Viewer Settings"), WIDGET_WINDOW_BUTTON)
        .CVar(CVAR_WINDOW("InputViewerSettings"))
        .RaceDisable(false)
        .WindowName("Input Viewer Settings")
        .HideInSearch(true)
        .Options(WindowButtonOptions().Tooltip(T("Enables the separate Input Viewer Settings Window.")));

    // Notifications
    path.sidebarName = "Notifications";
    path.column = SECTION_COLUMN_1;
    AddSidebarEntry("Settings", path.sidebarName, 3);
    AddWidget(path, T("Position"), WIDGET_CVAR_COMBOBOX)
        .CVar(CVAR_SETTING("Notifications.Position"))
        .RaceDisable(false)
        .Options(ComboboxOptions()
                     .Tooltip(T("Which corner of the screen notifications appear in."))
                     .ComboMap(notificationPosition)
                     .DefaultIndex(3));
    AddWidget(path, T("Duration (seconds):"), WIDGET_CVAR_SLIDER_FLOAT)
        .CVar(CVAR_SETTING("Notifications.Duration"))
        .RaceDisable(false)
        .Options(FloatSliderOptions()
                     .Tooltip(T("How long notifications are displayed for."))
                     .Format("%.1f")
                     .Step(0.1f)
                     .Min(3.0f)
                     .Max(30.0f)
                     .DefaultValue(10.0f));
    AddWidget(path, T("Background Opacity"), WIDGET_CVAR_SLIDER_FLOAT)
        .CVar(CVAR_SETTING("Notifications.BgOpacity"))
        .RaceDisable(false)
        .Options(FloatSliderOptions()
                     .Tooltip(T("How opaque the background of notifications is."))
                     .DefaultValue(0.5f)
                     .IsPercentage());
    AddWidget(path, T("Size:"), WIDGET_CVAR_SLIDER_FLOAT)
        .CVar(CVAR_SETTING("Notifications.Size"))
        .RaceDisable(false)
        .Options(FloatSliderOptions()
                     .Tooltip(T("How large notifications are."))
                     .Format("%.1f")
                     .Step(0.1f)
                     .Min(1.0f)
                     .Max(5.0f)
                     .DefaultValue(1.8f));
    AddWidget(path, T("Test Notification"), WIDGET_BUTTON)
        .RaceDisable(false)
        .Callback([](WidgetInfo& info) {
            Notification::Emit({
                .itemIcon = "__OTR__textures/icon_item_24_static/gQuestIconGoldSkulltulaTex",
                .prefix = "This",
                .message = "is a",
                .suffix = "test.",
            });
        })
        .Options(ButtonOptions().Tooltip(T("Displays a test notification.")));
    AddWidget(path, T("Mute Notification Sound"), WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_SETTING("Notifications.Mute"))
        .RaceDisable(false)
        .Options(CheckboxOptions().Tooltip(T("Prevent notifications from playing a sound.")));

    // Mod Menu
    path.sidebarName = "Mod Menu";
    AddSidebarEntry("Settings", path.sidebarName, 1);
    AddWidget(path, T("Popout Mod Menu Window"), WIDGET_WINDOW_BUTTON)
        .CVar(CVAR_WINDOW("ModMenu"))
        .WindowName("Mod Menu")
        .HideInSearch(true)
        .Options(WindowButtonOptions().Tooltip(T("Enables the separate Mod Menu Window.")));
}

} // namespace SohGui

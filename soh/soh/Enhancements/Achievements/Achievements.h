#pragma once

#ifdef __cplusplus

#include <ship/window/gui/GuiWindow.h>

namespace Achievements {

// Unlocks the achievement with the given id (no-op if already unlocked) and shows a notification.
void Unlock(int num);
bool IsUnlocked(int num);
// Clears every achievement (used by the "Reset" button in the window).
void ResetAll();

class Window final : public Ship::GuiWindow {
  public:
    using GuiWindow::GuiWindow;

    void InitElement() override{};
    void DrawElement() override;
    void UpdateElement() override{};
};

// Always-on overlay that shows the "Achievement Unlocked" card (GTA style, top center).
class ToastWindow final : public Ship::GuiWindow {
  public:
    using GuiWindow::GuiWindow;

    void InitElement() override{};
    void DrawElement() override{};
    void Draw() override;
    void UpdateElement() override{};
};

} // namespace Achievements

#endif // __cplusplus

#pragma once

#ifdef __cplusplus

#include <ship/window/gui/GuiWindow.h>

namespace Achievements {

// Unlocks the achievement with the given id (no-op if already unlocked) and shows a notification.
void Unlock(const char* id);
bool IsUnlocked(const char* id);
// Clears every achievement (used by the "Reset" button in the window).
void ResetAll();

class Window final : public Ship::GuiWindow {
  public:
    using GuiWindow::GuiWindow;

    void InitElement() override{};
    void DrawElement() override;
    void UpdateElement() override{};
};

} // namespace Achievements

#endif // __cplusplus

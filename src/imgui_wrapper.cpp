
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include "proj.h"
#include "imgui.h"
#include "main_cli.h"
#include "proj.h"
#include "version.h"
#include "viewport.h"
#include "main_menu.h"
#include "imgui_wrapper.h"

// Helper to display a little (?) mark which shows a tooltip when hovered.
// In your own code you may want to display an actual icon if you are using a merged icon fonts (see docs/FONTS.md)
void HelpMarker(const char *desc)
{
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort) && ImGui::BeginTooltip()) {
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}


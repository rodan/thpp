
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include "imgui.h"
#include "proj.h"
#include "version.h"
#include "help_about.h"

void help_about(bool *p_open)
{
    if (!ImGui::Begin("About ThPP", p_open, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::End();
        return;
    }

    ImGui::Text("Thermal processing panel v%d.%d build %d commit #%d", VER_MAJOR, VER_MINOR, BUILD,
                COMMIT);
    ImGui::Separator();
    ImGui::Text("copyright Petre Rodan, 2023-2025");
    ImGui::Text("ThPP is licensed under the GPLv3 License, see LICENSE for more information.");
    ImGui::End();
}


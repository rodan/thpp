
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

int imgui_wrapper(th_db_t * db)
{

    db->fe.return_state = RET_OK;

    main_menu(db);

    if (db->fe.return_state == RET_EXIT) {
        // file/exit was pressed
        return RET_EXIT;
    } else if (db->fe.return_state == RET_RST) {
        // file/open dialog has closed with a file selection
        // and most of db has been freed
        main_cli(db);
        viewport_refresh_vp(db);
    }

    //ImGui::ShowDemoWindow();
    //ImPlot::ShowDemoWindow();

    return db->fe.return_state;
}

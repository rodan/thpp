
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include "imgui.h"
#include "proj.h"
#include "tool_preferences.h"

extern struct global_preferences gp;

void tool_preferences(bool *p_open, th_db_t * db)
{
    uint8_t value_changed;

    if (!ImGui::Begin("preferences", p_open, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::End();
        return;
    }

    if (db->out_th == NULL) {
        ImGui::Text("file not opened");
        ImGui::End();
        return;
    }

    static int style_idx = DEF_STYLE;
    if (ImGui::Combo("theme", &style_idx, "Dark\0Light\0Classic\0")) {
        switch (style_idx) {
        case STYLE_DARK:
            ImGui::StyleColorsDark();
            break;
        case STYLE_LIGHT:
            ImGui::StyleColorsLight();
            break;
        case STYLE_CLASSIC:
            ImGui::StyleColorsClassic();
            break;
        }
    }

    static uint8_t s_pal = DEF_PALETTE;
    //if (reset_changes) {
    //    s_pal = db->p.pal;
    //}
    value_changed = ImGui::Combo("palette", (int *) &s_pal,
                 "256\0color\0grey\0hmetal0\0hmetal1\0hmetal2\0hotblue1\0hotblue2\0iron\0per_true\0pericolor\0rainbow\0rainbow0\0\0");
    if (value_changed) {
        gp.palette_default = s_pal;
        db->p.pal = s_pal;
        //show_apply_button = 1;
    }

    static int s_thumbnail_size = DEF_THUMBNAIL_SIZE;
    value_changed = ImGui::DragInt("thumbnail size", &s_thumbnail_size, 1, 64, 512, "%d", ImGuiSliderFlags_AlwaysClamp);
    if (value_changed) {
        gp.thumbnail_size = s_thumbnail_size;
    }

    ImGui::End();
}





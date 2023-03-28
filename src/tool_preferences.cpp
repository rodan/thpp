
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include "imgui.h"
#include "proj.h"
#include "graphics.h"
#include "viewport.h"
#include "tool_preferences.h"

void tool_preferences(bool *p_open, th_db_t * db)
{
    uint8_t value_changed;
    global_preferences_t *pref = gp_get_ptr();

    if (!ImGui::Begin("preferences", p_open, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::End();
        return;
    }

    if (db->out_th == NULL) {
        ImGui::Text("file not opened");
        ImGui::End();
        return;
    }

    ImGui::Separator();
    ImGui::Text("interface");

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


    ImGui::Separator();
    ImGui::Text("thumbnail image");

    static int s_thumbnail_size = DEF_THUMBNAIL_SIZE;
    value_changed = ImGui::DragInt("size", &s_thumbnail_size, 1, 64, 512, "%d", ImGuiSliderFlags_AlwaysClamp);
    if (value_changed) {
        pref->thumbnail_size = s_thumbnail_size;
    }

    ImGui::Separator();
    ImGui::Text("viewport image");

    static uint8_t s_pal = DEF_PALETTE;
    value_changed = ImGui::Combo("palette", (int *) &s_pal,
                 "256\0color\0grey\0hmetal0\0hmetal1\0hmetal2\0hotblue1\0hotblue2\0iron\0per_true\0pericolor\0rainbow\0rainbow0\0\0");
    if (value_changed) {
        pref->palette_default = s_pal;
        db->p.pal = s_pal;
    }

    // zoom interpolation technique
    int s_zoom_int = pref->zoom_interpolation;
    value_changed = ImGui::Combo("interpolation", (int *) &s_zoom_int, "nearest\0realsr\0\0");
    if (value_changed) {
        if (s_zoom_int == ZOOM_INTERP_REALSR) {
            if (pref->zoom_level > 1) {
                pref->zoom_level = 4;
                db->p.zoom_level = 4;
                db->fe.actual_zoom = 4;
            }
        }
        db->p.zoom_interpolation = s_zoom_int;
        pref->zoom_interpolation = s_zoom_int;
        image_zoom(&db->rgba[1], &db->rgba[0], pref->zoom_level, pref->zoom_interpolation);
        viewport_refresh_vp(db);
    }

    // target zoom level
    int s_zoom = pref->zoom_level;
    if (pref->zoom_interpolation == ZOOM_INTERP_REALSR) {
        value_changed = ImGui::SliderInt("zoom [1 or 4]", &s_zoom, 1, 4);
    } else {
        value_changed = ImGui::SliderInt("zoom [1..16]", &s_zoom, 1, 16);
    }
    if (value_changed) {
        if (s_zoom > pref->zoom_level) {
            set_zoom(db, ZOOM_INCREMENT);
        } else {
            set_zoom(db, ZOOM_DECREMENT);
        }
        viewport_refresh_vp(db);
    }


    ImGui::End();
}





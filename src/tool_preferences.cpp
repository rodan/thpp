
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include "imgui.h"
#include "proj.h"
#include "graphics.h"
#include "viewport.h"
#include "imgui_wrapper.h"
#include "file_properties.h"
#include "tool_preferences.h"

void tool_preferences(bool *p_open, th_db_t * db)
{
    uint8_t value_changed;
    global_preferences_t *pref = gp_get_ptr();
    fp_visibility_t *v = file_properties_get_ptr();

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
    ImGui::Indent();

    static int style_idx = DEF_STYLE;
    if (ImGui::Combo("theme", &style_idx, "Dark\0Light\0Classic\0")) {
        ImVec4* colors = ImGui::GetStyle().Colors;

        switch (style_idx) {
        case STYLE_DARK:
            ImGui::StyleColorsDark();
            style_set(STYLE_DARK, &pref->style);
            break;
        case STYLE_LIGHT:
            ImGui::StyleColorsLight();
            colors[ImGuiCol_WindowBg] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
            style_set(STYLE_LIGHT, &pref->style);
            break;
        case STYLE_CLASSIC:
            ImGui::StyleColorsClassic();
            style_set(STYLE_CLASSIC, &pref->style);
            break;
        }
    }

    static int font_scale = DEF_FONT_SCALE;
    value_changed = ImGui::DragInt("font scale", &font_scale, 1, 1, 2, "%d", ImGuiSliderFlags_AlwaysClamp);
    if (value_changed) {
        ImGuiIO & io = ImGui::GetIO();
        io.FontGlobalScale = font_scale;
    }

    ImGui::Unindent();
    ImGui::Separator();
    ImGui::Text("thumbnail image");
    ImGui::Indent();

    static int s_thumbnail_size = DEF_THUMBNAIL_SIZE;
    value_changed = ImGui::DragInt("size", &s_thumbnail_size, 1, 64, 512, "%d", ImGuiSliderFlags_AlwaysClamp);
    if (value_changed) {
        pref->thumbnail_size = s_thumbnail_size;
    }

    static int s_pal = DEF_PALETTE;
    value_changed = ImGui::Combo("palette", &s_pal,
                 "256\0color\0grey\0hmetal0\0hmetal1\0hmetal2\0hotblue1\0hotblue2\0iron\0per_true\0pericolor\0rainbow\0rainbow0\0\0");
    if (value_changed) {
        pref->palette_default = s_pal;
        db->p.pal = s_pal;
    }

    ImGui::Unindent();
    ImGui::Separator();
    ImGui::Text("viewport image");
    ImGui::Indent();

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
        image_zoom(&db->rgba[RGBA_ORIG_ZOOMED], &db->rgba[RGBA_ORIG], pref->zoom_level, pref->zoom_interpolation);
        if (db->fe.flags & HIGHLIGHT_LAYER_EN) {
            image_zoom(&db->rgba[RGBA_HIGHLIGHT_ZOOMED], &db->rgba[RGBA_HIGHLIGHT], pref->zoom_level, pref->zoom_interpolation);
        }
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
            if (set_zoom(db, ZOOM_INCREMENT)) {
                viewport_refresh_vp(db);
            }
        } else {
            if (set_zoom(db, ZOOM_DECREMENT)) {
                viewport_refresh_vp(db);
            }
        }
    }

    ImGui::Unindent();
    ImGui::Separator();
    if (ImGui::TreeNode("file properties")) {
        ImGui::SameLine(); HelpMarker("control the visibility of items in the file properties table");
        ImGui::Checkbox("camera make", &v->camera_make);
        ImGui::Checkbox("camera model", &v->camera_model);
        ImGui::Checkbox("exif byte order", &v->byte_order);
        ImGui::Checkbox("image resolution", &v->raw_th_img_res);
        ImGui::Checkbox("image timestamp", &v->ir_timestamp);
        ImGui::Checkbox("image file name", &v->ir_fname);
        ImGui::Checkbox("image comment", &v->ir_comment);
        ImGui::Checkbox("image min", &v->ir_min);
        ImGui::Checkbox("image max", &v->ir_max);
        ImGui::Checkbox("image avg", &v->ir_avg);
        ImGui::Checkbox("hl min", &v->hl_min);
        ImGui::Checkbox("hl max", &v->hl_max);
        ImGui::Checkbox("hl avg", &v->hl_avg);
        ImGui::Checkbox("emissivity", &v->emissivity);
        ImGui::Checkbox("object distance", &v->distance);
        ImGui::Checkbox("relative humidity", &v->rh);
        ImGui::Checkbox("atmospheric trans Alpha1", &v->alpha1);
        ImGui::Checkbox("atmospheric trans Alpha2", &v->alpha2);
        ImGui::Checkbox("atmospheric trans Beta1", &v->beta1);
        ImGui::Checkbox("atmospheric trans Beta2", &v->beta2);
        ImGui::Checkbox("planck r1", &v->planckR1);
        ImGui::Checkbox("planck r2", &v->planckR2);
        ImGui::Checkbox("planck b", &v->planckB);
        ImGui::Checkbox("planck f", &v->planckF);
        ImGui::Checkbox("planck o", &v->planckO);
        ImGui::Checkbox("atmospheric TransX", &v->atm_trans_X);
        ImGui::Checkbox("atmospheric temperature", &v->atm_temp);
        ImGui::Checkbox("reflected apparent temperature", &v->refl_temp);
    }


    ImGui::End();
}



#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <linux/limits.h>
#include "imgui.h"
#include "proj.h"
#include "lodepng.h"
#include "imgui_wrapper.h"
#include "viewport.h"
#include "tool_export.h"

#define    PREFIX_MAX  64

static char buf_dst_dir[PATH_MAX] = "/tmp";
static char buf_prefix[PREFIX_MAX] = {};
static char buf_output[PATH_MAX] = {};

style_t tool_export_style = {};

void tool_export(bool *p_open, th_db_t *db)
{
    unsigned err = 0;
    uint16_t buf_prefix_len;
    static bool table_en = false;
    uint8_t value_changed;
    uint8_t auto_refresh = 0;
    double t_min, t_max;
    double spot_temp = 0;
    global_preferences_t *pref = gp_get_ptr();
    static int prox_temp = 4;
    static int prox_pix = 24;
    static uint16_t prev_x = 0;
    static uint16_t prev_y = 0;

    if (!ImGui::Begin("export", p_open, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::End();
        return;
    }

    if (db->out_th == NULL) {
        ImGui::Text("file not opened");
        ImGui::End();
        return;
    }

    ImGui::Text("path elements");
    ImGui::Indent();
    ImGui::InputText("dest dir", buf_dst_dir, PATH_MAX);

    // populate filename prefix if new thermogram was loaded
    if ( !(db->fe.flags & TOOL_EXPORT_GOT_BASENAME) ) {
        snprintf(buf_prefix, PREFIX_MAX, basename(db->p.in_file));
        buf_prefix_len = strlen(buf_prefix);
        if (buf_prefix_len > 6) {
            if (buf_prefix[buf_prefix_len - 4] == '.') {
                buf_prefix[buf_prefix_len - 4] = 0;
            }
        }
        db->fe.flags |= TOOL_EXPORT_GOT_BASENAME;
    }

    ImGui::InputText("prefix", buf_prefix, PREFIX_MAX);

    if (ImGui::Button("export image and scale")) {
        style_set(STYLE_LIGHT, &tool_export_style);
        generate_scale(db, &tool_export_style);
        snprintf(buf_output, PATH_MAX, "%s/%s.png", buf_dst_dir, buf_prefix);
        printf("saving image to %s\n", buf_output);
        if (pref->zoom_level == 1) {
            err = lodepng_encode32_file(buf_output, db->rgba[RGBA_ORIG].data, db->rgba[RGBA_ORIG].width, db->rgba[RGBA_ORIG].height);
        } else {
            err = lodepng_encode32_file(buf_output, db->rgba[RGBA_ORIG_ZOOMED].data, db->rgba[RGBA_ORIG_ZOOMED].width, db->rgba[RGBA_ORIG_ZOOMED].height);
        }
        if (err) {
            fprintf(stderr, "encoder error %u: %s\n", err, lodepng_error_text(err));
        }

        snprintf(buf_output, PATH_MAX, "%s/%s_scale.png", buf_dst_dir, buf_prefix);
        printf("saving scale to %s\n", buf_output);
        err = lodepng_encode32_file(buf_output, db->scale.combo, db->scale.width, db->scale.height);
        if (err) {
            fprintf(stderr, "encoder error %u: %s\n", err, lodepng_error_text(err));
        }
    }

    ImGui::Unindent();

    ImGui::Separator();
    ImGui::Text("highlight");

    ImGui::Indent();
    if (ImGui::CheckboxFlags("enable highlight layer", &db->fe.flags, HIGHLIGHT_LAYER_EN)) {
        if (db->fe.flags & HIGHLIGHT_LAYER_EN) {
            generate_highlight(db);
        }
    }

    if ( !(db->fe.flags & HIGHLIGHT_LAYER_EN) ) {
        ImGui::BeginDisabled();
    }

    if (ImGui::CheckboxFlags("preview", &db->fe.flags, HIGHLIGHT_LAYER_PREVIEW_EN)) {
        viewport_refresh_vp(db);
    }

    static int h_type = 0;
    value_changed = ImGui::Combo("profile type", &h_type,
                 "none\0punctiform\0line profile\0level slice\0\0");
    //if (value_changed) {
    //    db->pr.type = h_type;
    //}

    switch (h_type) {
        case PROFILE_TYPE_POINT:

            if ((db->pr.flags & PROFILE_REQ_VIEWPORT_RDY) && (db->pr.type & PROFILE_TYPE_POINT) && (db->temp_arr != NULL)) {
                spot_temp = db->temp_arr[db->pr.y1 * db->rgba[RGBA_ORIG].width + db->pr.x1];
                t_min = spot_temp - prox_temp;
                t_max = spot_temp + prox_temp;
            } else {
                get_min_max(db->out_th, &t_min, &t_max);
            }

            if (db->pr.flags & PROFILE_REQ_VIEWPORT_RDY) {
                db->pr.t_min = t_min;
                db->pr.t_max = t_max;
                db->pr.prox_pix = prox_pix;

                ImGui::Text("position: (%d,%d)", db->pr.x1, db->pr.y1);

                value_changed = ImGui::DragInt("grow [pixels]", &prox_pix, 1, 1, 200, "%d", ImGuiSliderFlags_AlwaysClamp);
                if (value_changed) {
                    auto_refresh = 1;
                }

                value_changed = ImGui::DragInt("proximity [C]", &prox_temp, 1, 1, 200, "%d", ImGuiSliderFlags_AlwaysClamp);
                if (value_changed) {
                    auto_refresh = 1;
                }

                if ((prev_x != db->pr.x1) || (prev_y != db->pr.y1)) {
                    auto_refresh = 1;
                    prev_x = db->pr.x1;
                    prev_y = db->pr.y1;
                }

            } else {
                ImGui::Text("position: undefined ");
            }
            if (ImGui::Button("pick position")) {
                db->pr.type = PROFILE_TYPE_POINT;
                db->pr.flags |= PROFILE_REQ_VIEWPORT_INT;
            }
            break;
        case PROFILE_TYPE_LEVEL_SLICE:
            get_min_max(db->out_th, &t_min, &t_max);

            static float s_begin_temp = t_min;
            static float s_end_temp = t_max;

            value_changed = ImGui::DragFloatRange2("temp slice [C]", &s_begin_temp, &s_end_temp, 0.5f, -20.0f, 300.0f, "min: %.1fC",
                                   "max: %.1fC", ImGuiSliderFlags_AlwaysClamp);
            if (value_changed) {
                db->pr.type = PROFILE_TYPE_LEVEL_SLICE;
                db->pr.t_min = s_begin_temp;
                db->pr.t_max = s_end_temp;
                auto_refresh = 1;
            }

            break;
        case PROFILE_TYPE_LINE:
            if (ImGui::Button("pick line")) {
                db->pr.type = PROFILE_TYPE_LINE;
                db->pr.flags |= PROFILE_REQ_VIEWPORT_INT;
            }
            break;
    }

    //ImGui::SameLine();
    if (ImGui::Button("refresh") || auto_refresh) {
        memset(db->rgba[RGBA_HIGHLIGHT].overlay, 0, db->rgba[RGBA_HIGHLIGHT].width * db->rgba[RGBA_HIGHLIGHT].height * 4);
        refresh_highlight_overlay(db, 0, db->p.pal);
        combine_highlight(db);
        set_zoom(db, ZOOM_FORCE_REFRESH);
        viewport_refresh_vp(db);
    }

    if ( !(db->fe.flags & HIGHLIGHT_LAYER_EN) ) {
        ImGui::EndDisabled();
    }
    ImGui::Unindent();


    ImGui::Separator();
    ImGui::Text("report");

    ImGui::Indent();
    ImGui::Checkbox("add table", &table_en);
    //static char buf3[64];
    //ImGui::InputText("template", buf3, 64, ImGuiInputTextFlags_CallbackEdit, MyCallback);

    ImGui::Unindent();
    if (ImGui::Button("generate latex report")) {
    }

    ImGui::End();
}

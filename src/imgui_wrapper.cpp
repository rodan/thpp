
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <GLFW/glfw3.h>
#include "proj.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "implot.h"
#include "imfilebrowser.h"
#include "opengl_helper.h"
#include "main_cli.h"
#include "proj.h"
#include "version.h"
#include "main_menu.h"
#include "tool_scale.h"
#include "tool_profile.h"
#include "tool_histogram.h"
#include "tool_processing.h"
#include "help_about.h"
#include "file_properties.h"
#include "imgui_wrapper.h"

extern ImGui::FileBrowser fileDialog;
extern ImGuiContext *GImGui;

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

void imgui_render_viewport(th_db_t * db, linedef_t * line)
{
    ImGuiIO & io = ImGui::GetIO();

    ImGui::Begin("viewport");
    ImVec2 screen_pos = ImGui::GetCursorScreenPos();

    if ((db->fe.vp_texture == 0) || (db->fe.return_state == RET_OK_REFRESH_NEEDED) || (db->fe.return_state == RET_RST)) {
        //load_texture_from_file(db->p.out_file, &db->fe.vp_texture, &vp_width, &vp_height);
        db->fe.actual_zoom = db->p.zoom;
        db->fe.vp_width = db->rgba.width;
        db->fe.vp_height = db->rgba.height;
        load_texture_from_mem(db->rgba.data, &db->fe.vp_texture, db->fe.vp_width, db->fe.vp_height);
        //load_texture_from_mem(db->rgba.data, fb_get_texture_ptr(), vp_width, vp_height);
    } else {
        ImGui::Image((void *)(intptr_t) db->fe.vp_texture, ImVec2(db->fe.vp_width, db->fe.vp_height));
    }

    int16_t img_pos_x, img_pos_y;
    static int16_t prev_img_pos_x, prev_img_pos_y;
    img_pos_x = (io.MousePos.x - screen_pos.x) / db->fe.actual_zoom;
    img_pos_y = (io.MousePos.y - screen_pos.y) / db->fe.actual_zoom;
    ImGuiContext & g = *GImGui;
    static uint8_t pointer_inside_image = 0;
    uint8_t pointer_over_viewport = 0;

    if (g.HoveredWindow) {
        if (strstr(g.HoveredWindow->Name, "viewport") != NULL) {
            pointer_over_viewport = 1;
        }
    }

    switch (db->in_th->type) {
    case TH_FLIR_RJPG:
        if ((img_pos_x > 0) && (img_pos_x < db->in_th->head.rjpg->raw_th_img_width) &&
            (img_pos_y > 0) && (img_pos_y < db->in_th->head.rjpg->raw_th_img_height) &&
            pointer_over_viewport) {
            ImGui::Text("spot %.2f°C",
                        db->temp_arr[img_pos_y * db->in_th->head.rjpg->raw_th_img_width +
                                     img_pos_x]);
            pointer_inside_image = 1;
        } else {
            pointer_inside_image = 0;
        }
        break;
    case TH_IRTIS_DTV:
        if ((img_pos_x > 0) && (img_pos_x < db->in_th->head.dtv->nst) &&
            (img_pos_y > 0) && (img_pos_y < db->in_th->head.dtv->nstv) && pointer_over_viewport) {
            ImGui::Text("spot %.2f°C",
                        db->temp_arr[img_pos_y * db->in_th->head.dtv->nst + img_pos_x]);
            pointer_inside_image = 1;
        } else {
            pointer_inside_image = 0;
        }
        break;
    }
    ImGui::End();

    if (db->fe.return_state != RET_OK) {
        memset(&line, 0, sizeof(linedef_t));
        return;
    }

    if (pointer_inside_image && ImGui::IsMouseDown(0) && (line->do_refresh == 0)
        && pointer_over_viewport) {
        prev_img_pos_x = img_pos_x;
        prev_img_pos_y = img_pos_y;
        line->do_refresh = 1;
    }

    if (pointer_inside_image && ImGui::IsMouseDown(0) && pointer_over_viewport) {
        if ((prev_img_pos_x == img_pos_x) && (prev_img_pos_y == img_pos_y)) {
            // do not activate the line plot after a doubleclick on the viewport
            line->active = 0;
        } else {
            line->x1 = prev_img_pos_x;
            line->y1 = prev_img_pos_y;
            line->x2 = img_pos_x;
            line->y2 = img_pos_y;
            line->active = 1;
        }
    }

    if (ImGui::IsMouseDown(0) == 0) {
        line->do_refresh = 0;
    }
}

int imgui_wrapper(th_db_t * db)
{
    static linedef_t line;

    db->fe.return_state = RET_OK;

    main_menu(db);

    if (db->fe.return_state == RET_EXIT) {
        // file/exit was pressed
        return RET_EXIT;
    } else if (db->fe.return_state == RET_RST) {
        // file/open dialog has closed with a file selection
        // and most of db has been freed
        main_cli(db);
        memset(&line, 0, sizeof(linedef_t));
    }

    tool_processing(db);

    if (db->fe.return_state == RET_OK_REFRESH_NEEDED) {
        // the 'apply changes' button was pressed
        // temperatures inside the image are bound to change
        // so invalidate the line plot
        memset(&line, 0, sizeof(linedef_t));
    }

    imgui_render_viewport(db, &line);

    tool_scale(db);
    tool_profile(db, &line);
    if (db->fe.return_state == RET_OK) {
        tool_histogram(db);
    }

    //ImGui::ShowDemoWindow();
    //ImPlot::ShowDemoWindow();

    return db->fe.return_state;
}

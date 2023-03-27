
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include "imgui.h"
#include "imgui_internal.h"
#include "proj.h"
#include "opengl_helper.h"
#include "graphics.h"
#include "viewport.h"

uint32_t prev_texture = 0;

uint8_t viewport_refresh_vp(th_db_t * db)
{
    global_preferences_t *pref = gp_get_ptr();

    //load_texture_from_file(db->p.out_file, &db->fe.vp_texture, &vp_width, &vp_height);
    db->fe.actual_zoom = pref->zoom_level;
    db->fe.vp_width = db->rgba[1].width;
    db->fe.vp_height = db->rgba[1].height;

    if (db->fe.vp_texture) {
        prev_texture = db->fe.vp_texture;
    }

    load_texture_from_mem(db->rgba[1].data, &db->fe.vp_texture, db->fe.vp_width, db->fe.vp_height);

    // we should force at least one more frame to be rendered

    return EXIT_SUCCESS;
}

void viewport_render(th_db_t * db)
{
    int16_t img_pos_x, img_pos_y;
    static int16_t prev_img_pos_x, prev_img_pos_y;
    static uint8_t pointer_inside_image = 0;
    uint8_t pointer_over_viewport = 0;
    ImGuiWindowClass window_class;

    ImGuiIO & io = ImGui::GetIO();
    ImGuiContext & g = *GImGui;

    window_class.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_NoTabBar | ImGuiDockNodeFlags_CentralNode;
    ImGui::SetNextWindowClass(&window_class);

    ImGui::Begin("viewport");
    ImVec2 screen_pos = ImGui::GetCursorScreenPos();

    if (db->in_th == NULL) {
        ImGui::End();
        return;
    }

    ImGui::Image((void *)(intptr_t) db->fe.vp_texture, ImVec2(db->fe.vp_width, db->fe.vp_height));

    if (prev_texture) {
        free_textures(1, &prev_texture);
        prev_texture = 0;
    }

    img_pos_x = (io.MousePos.x - screen_pos.x) / db->fe.actual_zoom;
    img_pos_y = (io.MousePos.y - screen_pos.y) / db->fe.actual_zoom;

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
        memset(&db->pr, 0, sizeof(profile_t));
        return;
    }

    if (pointer_inside_image && ImGui::IsMouseDown(0) && (db->pr.do_refresh == 0)
        && pointer_over_viewport) {
        prev_img_pos_x = img_pos_x;
        prev_img_pos_y = img_pos_y;
        db->pr.do_refresh = 1;
    }

    if (pointer_inside_image && ImGui::IsMouseDown(0) && pointer_over_viewport) {
        if ((prev_img_pos_x == img_pos_x) && (prev_img_pos_y == img_pos_y)) {
            // do not activate the line plot after a doubleclick on the viewport
            db->pr.active = 0;
        } else {
            db->pr.x1 = prev_img_pos_x;
            db->pr.y1 = prev_img_pos_y;
            db->pr.x2 = img_pos_x;
            db->pr.y2 = img_pos_y;
            db->pr.active = 1;
        }
    }

    global_preferences_t *pref = gp_get_ptr();

    if (pointer_inside_image && (io.MouseWheel < 0)) {
        if (pref->zoom_level > 1) {
            pref->zoom_level--;
            image_zoom(&db->rgba[1], &db->rgba[0], pref->zoom_level, pref->zoom_interpolation);
            viewport_refresh_vp(db);
        }
    }

    if (pointer_inside_image && (io.MouseWheel > 0)) {
        if (pref->zoom_level < 16) {
            pref->zoom_level++;
            image_zoom(&db->rgba[1], &db->rgba[0], pref->zoom_level, pref->zoom_interpolation);
            viewport_refresh_vp(db);
        }
    }

    if (ImGui::IsMouseDown(0) == 0) {
        db->pr.do_refresh = 0;
    }
}


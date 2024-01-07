
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

    select_vp(db);

    if (db->rgba_vp == NULL) {
        return EXIT_FAILURE;
    }

    db->fe.vp_width = db->rgba_vp->width;
    db->fe.vp_height = db->rgba_vp->height;

    if (db->fe.vp_texture) {
        prev_texture = db->fe.vp_texture;
    }

    if (db->fe.vp_width && db->fe.vp_height) {
        load_texture_from_mem(db->rgba_vp->data, &db->fe.vp_texture, db->fe.vp_width, db->fe.vp_height);
        db->pr.flags |= PROFILE_REQ_VIEWPORT_REFRESHED;
        return EXIT_SUCCESS;
    }

    // we should force at least one more frame to be rendered

    return EXIT_SUCCESS;
}

void viewport_render(th_db_t * db)
{
    int16_t img_pos_x, img_pos_y;
    static int16_t prev_img_pos_x, prev_img_pos_y;
    uint8_t pointer_inside_image = 0;
    uint8_t pointer_over_viewport = 0;
    ImGuiWindowClass window_class;
    double spot_temp = 0;

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
            spot_temp = db->temp_arr[img_pos_y * db->in_th->head.rjpg->raw_th_img_width +
                                     img_pos_x];
            pointer_inside_image = 1;
        } else {
            pointer_inside_image = 0;
        }
        break;
    case TH_IRTIS_DTV:
        if ((img_pos_x > 0) && (img_pos_x < db->in_th->head.dtv->nst) &&
            (img_pos_y > 0) && (img_pos_y < db->in_th->head.dtv->nstv) && pointer_over_viewport) {
            spot_temp = db->temp_arr[img_pos_y * db->in_th->head.dtv->nst + img_pos_x];
            pointer_inside_image = 1;
        } else {
            pointer_inside_image = 0;
        }
        break;
    }

    if (db->fe.return_state != RET_OK) {
        cleanup_profile(db, PROFILE_KEEP_INIT);
        ImGui::End();
        return;
    }

    if (pointer_inside_image && ImGui::IsMouseDown(0) && (db->pr.flags & PROFILE_REQ_VIEWPORT_START) && pointer_over_viewport) {
        // initial click, used as first (start) coordinate
        prev_img_pos_x = img_pos_x;
        prev_img_pos_y = img_pos_y;
        db->pr.flags &= ~PROFILE_REQ_VIEWPORT_START;
    }

    if (pointer_inside_image && ImGui::IsMouseDown(0) && pointer_over_viewport) {
        switch (db->pr.type) {
            case PROFILE_TYPE_POINT:
                if (db->pr.flags & PROFILE_REQ_VIEWPORT_INT) {
                    db->pr.x1 = img_pos_x;
                    db->pr.y1 = img_pos_y;
                    db->pr.flags = PROFILE_REQ_VIEWPORT_RDY;
                    db->pr.flags &= ~PROFILE_REQ_VIEWPORT_INT;
                }
                break;
            case PROFILE_TYPE_LINE:
                if (db->pr.flags & PROFILE_REQ_VIEWPORT_INT) {
                    if ((prev_img_pos_x == img_pos_x) && (prev_img_pos_y == img_pos_y)) {
                        // do not activate the line plot after a doubleclick on the viewport
                        db->pr.flags &= ~PROFILE_REQ_VIEWPORT_RDY;
                    } else {
                        db->pr.x1 = prev_img_pos_x;
                        db->pr.y1 = prev_img_pos_y;
                        db->pr.x2 = img_pos_x;
                        db->pr.y2 = img_pos_y;
                        // render line during mouse drag
                        db->pr.flags |= PROFILE_REQ_VIEWPORT_RDY;
                        db->pr.flags &= ~PROFILE_REQ_VIEWPORT_REFRESHED;
                    }
                }
            break;
        }
    }

    if (pointer_inside_image && ImGui::IsMouseReleased(0) && pointer_over_viewport && (db->pr.flags & PROFILE_REQ_VIEWPORT_INT)) {
        if (db->pr.type == PROFILE_TYPE_LINE) {
            db->pr.flags |= PROFILE_REQ_VIEWPORT_RDY;
            db->pr.flags |= PROFILE_REQ_DATA_PREPARE;
            db->pr.flags &= ~PROFILE_REQ_VIEWPORT_INT;
        }
    }

    if (pointer_inside_image && (io.MouseWheel < 0) && io.KeyCtrl) {
        if (set_zoom(db, ZOOM_DECREMENT)) {
            viewport_refresh_vp(db);
        }
    }

    if (pointer_inside_image && (io.MouseWheel > 0) && io.KeyCtrl) {
        if (set_zoom(db, ZOOM_INCREMENT)) {
            viewport_refresh_vp(db);
        }
    }

    ImGui::Image((void *)(intptr_t) db->fe.vp_texture, ImVec2(db->fe.vp_width, db->fe.vp_height));

    if (prev_texture) {
        free_textures(1, &prev_texture);
        prev_texture = 0;
    }

    if (pointer_inside_image) {
        ImGui::Text("spot %.2f°C", spot_temp);
    } else {
        ImGui::Text(" ");
    }
    ImGui::End();

#if 0
    if (ImGui::IsMouseDown(0) == 0) {
        db->pr.flags &= ~PROFILE_REQ_DATA_PREPARE;
        //db->pr.do_refresh = 0;
    }
#endif
}


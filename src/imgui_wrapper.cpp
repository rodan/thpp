
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <GLFW/glfw3.h>         // Will drag system OpenGL headers
#include "proj.h"
#include "imgui.h"
#include "implot.h"
#include "opengl_helper.h"
#include "main_cli.h"
#include "implot_wrapper.h"
#include "imgui_wrapper.h"

//SDL_Texture *vp_texture = NULL;
unsigned int vp_width = 0;
unsigned int vp_height = 0;
GLuint vp_texture = 0;

int imgui_wrapper(th_db_t * db)
{
    int ret = RET_OK;
    static int show_apply_button = 0;
    static uint8_t reset_changes = 0;

    // add docking
    //ImGuiIO& io = ImGui::GetIO();

    //if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
    //    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    //}

    //DockSpaceOverViewport();
    //ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());

    static bool opt_fullscreen = true;
    static bool opt_padding = false;
    static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

    // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
    // because it would be confusing to have two docking targets within each others.
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    if (opt_fullscreen) {
        const ImGuiViewport *viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        window_flags |=
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove;
        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    } else {
        dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
    }

    // When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
    // and handle the pass-thru hole, so we ask Begin() to not render a background.
    if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
        window_flags |= ImGuiWindowFlags_NoBackground;

    // Important: note that we proceed even if Begin() returns false (aka window is collapsed).
    // This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
    // all active windows docked into it will lose their parent and become undocked.
    // We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
    // any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
    if (!opt_padding)
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("DockSpace Demo", nullptr, window_flags);
    if (!opt_padding)
        ImGui::PopStyleVar();

    if (opt_fullscreen)
        ImGui::PopStyleVar(2);

    // Submit the DockSpace
    ImGuiIO & io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
        ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
    }

    // end of docking copy-pasta


    ImGui::Begin("Processing");

    ImGui::Separator();
    ImGui::Text("input parameters");
    ImGui::Separator();

    // palette picker
    static int s_pal = db->p.pal;
    ImGui::Combo("palette", &s_pal,
                 "256\0color\0grey\0hmetal0\0hmetal1\0hmetal2\0hotblue1\0hotblue2\0iron\0per_true\0pericolor\0rainbow\0rainbow0\0\0");
    if (s_pal != db->p.pal) {
        db->p.pal = s_pal;
        show_apply_button = 1;
    }

    // zoom level
    static int s_zoom = db->p.zoom;
    static int actual_zoom = s_zoom;
    ImGui::SliderInt("zoom [1..10]", &s_zoom, 1, 10);
    if (s_zoom != db->p.zoom) {
        db->p.zoom = s_zoom;
        show_apply_button = 1;
    }

    if (db->in_th->type == TH_IRTIS_DTV) {

        dtv_header_t *hd;
        hd = db->in_th->head.dtv;

        static float s_d_begin = hd->tsc[1];
        static float s_d_end = hd->tsc[1] + 256.0 * hd->tsc[0];
        if (reset_changes) {
            s_d_begin = hd->tsc[1];
            s_d_end = hd->tsc[1] + 256.0 * hd->tsc[0];
        }
        ImGui::DragFloatRange2("rescale [C]", &s_d_begin, &s_d_end, 0.5f, -20.0f, 300.0f,
                               "min: %.1fC", "max: %.1fC", ImGuiSliderFlags_AlwaysClamp);
        if ((s_d_begin != hd->tsc[1]) || (s_d_end != hd->tsc[1] + 256.0 * hd->tsc[0])) {
            db->p.flags |= OPT_SET_NEW_MIN | OPT_SET_NEW_MAX;
            db->p.t_min = s_d_begin;
            db->p.t_max = s_d_end;
            show_apply_button = 1;
        }

        ImGui::Separator();

    } else if (db->in_th->type == TH_FLIR_RJPG) {
        rjpg_header_t *h;
        h = db->out_th->head.rjpg;

        static float s_begin = h->t_min;
        static float s_end = h->t_max;
        if (reset_changes) {
            s_begin = h->t_min;
            s_end = h->t_max;
        }
        ImGui::DragFloatRange2("rescale [C]", &s_begin, &s_end, 0.5f, -20.0f, 300.0f, "min: %.1fC",
                               "max: %.1fC", ImGuiSliderFlags_AlwaysClamp);
        if ((s_begin != h->t_min) || (s_end != h->t_max)) {
            db->p.flags |= OPT_SET_NEW_MIN | OPT_SET_NEW_MAX;
            db->p.t_min = s_begin;
            db->p.t_max = s_end;
            show_apply_button = 1;
        }

        ImGui::Separator();

        // temperature compensation
        ImGui::Text("temperature compensation");
        ImGui::Separator();

        static float s_distance = h->distance;
        if (reset_changes) {
            s_distance = h->distance;
        }
        ImGui::DragFloat("distance [m]", &s_distance, 0.2f, 0.2f, 100.0f, "%0.2f m");
        if (s_distance != h->distance) {
            db->p.flags |= OPT_SET_DISTANCE_COMP | OPT_SET_NEW_DISTANCE;
            db->p.distance = s_distance;
            show_apply_button = 1;
        }

        static float s_emissivity = h->emissivity;
        if (reset_changes) {
            s_emissivity = h->emissivity;
        }
        ImGui::DragFloat("emissivity", &s_emissivity, 0.01f, 0.1f, 1.0f, "%0.2f");
        if (s_emissivity != h->emissivity) {
            db->p.flags |= OPT_SET_NEW_EMISSIVITY;
            db->p.emissivity = s_emissivity;
            show_apply_button = 1;
        }

        static float s_atm_temp = h->air_temp - RJPG_K;
        if (reset_changes) {
            s_atm_temp = h->air_temp - RJPG_K;
        }
        ImGui::DragFloat("atm temp [C]", &s_atm_temp, 1.0f, -20.1f, 300.0f, "%0.2f C");
        if (s_atm_temp != h->air_temp - RJPG_K) {
            db->p.flags |= OPT_SET_NEW_AT | OPT_SET_DISTANCE_COMP;
            db->p.atm_temp = s_atm_temp;
            show_apply_button = 1;
        }

        static float s_rh = h->rh * 100.0;
        if (reset_changes) {
            s_rh = h->rh * 100.0;
        }
        ImGui::DragFloat("rel humidity [%]", &s_rh, 1.0f, 0.1f, 100.0f, "%.0f %rH");
        if (s_rh != h->rh) {
            db->p.flags |= OPT_SET_NEW_RH | OPT_SET_DISTANCE_COMP;
            db->p.rh = s_rh / 100.0;
            show_apply_button = 1;
        }

    }

    ImGui::Separator();

    if (reset_changes) {
        reset_changes = 0;
    }

    if (ImGui::Button("reset changes")) {
        db->p.flags = 0;
        main_cli(db);
        show_apply_button = 0;
        ret = RET_OK_REFRESH_NEEDED;
        reset_changes = 1;
    }

    ImGui::SameLine();

    if (show_apply_button) {
        if (ImGui::Button("apply changes")) {
            main_cli(db);
            show_apply_button = 0;
            actual_zoom = db->p.zoom;
            ret = RET_OK_REFRESH_NEEDED;
        }
    }
    ImGui::End();

#if 1
    ImGui::Begin("Viewport");
    ImVec2 screen_pos = ImGui::GetCursorScreenPos();

    if ((vp_texture == 0) || (ret == RET_OK_REFRESH_NEEDED)) {
        //load_texture_from_file(db->p.out_file, &vp_texture, &vp_width, &vp_height);
        vp_width = db->rgba.width;
        vp_height = db->rgba.height;
        load_texture_from_mem(db->rgba.data, &vp_texture, vp_width, vp_height);
        //load_texture_from_mem(db->rgba.data, fb_get_texture_ptr(), vp_width, vp_height);
    } else {
        ImGui::Image((void *)(intptr_t) vp_texture, ImVec2(vp_width, vp_height));
    }

    int16_t img_pos_x, img_pos_y;
    static int16_t prev_img_pos_x, prev_img_pos_y;
    img_pos_x = (io.MousePos.x - screen_pos.x) / actual_zoom;
    img_pos_y = (io.MousePos.y - screen_pos.y) / actual_zoom;
    static uint8_t pointer_inside_image = 0;
    static linedef_t line;

    switch (db->in_th->type) {
        case TH_FLIR_RJPG:
            if ((img_pos_x > 0) && (img_pos_x < db->in_th->head.rjpg->raw_th_img_width) && 
               (img_pos_y > 0) && (img_pos_y < db->in_th->head.rjpg->raw_th_img_height)) {
                ImGui::Text("spot %.2f°C", db->temp_arr[img_pos_y * db->in_th->head.rjpg->raw_th_img_width + img_pos_x]);
                pointer_inside_image = 1;
            } else {
                pointer_inside_image = 0;
            }
            break;
        case TH_IRTIS_DTV:
            if ((img_pos_x > 0) && (img_pos_x < db->in_th->head.dtv->nst) && 
               (img_pos_y > 0) && (img_pos_y < db->in_th->head.dtv->nstv)) {
                ImGui::Text("spot %.2f°C", db->temp_arr[img_pos_y * db->in_th->head.dtv->nst + img_pos_x]);
                pointer_inside_image = 1;
            } else {
                pointer_inside_image = 0;
            }
            break;
    }

    if (pointer_inside_image && ImGui::IsMouseDown(0) && (line.do_refresh == 0))  {
        prev_img_pos_x = img_pos_x;
        prev_img_pos_y = img_pos_y;
        line.do_refresh = 1;
    }

    if (pointer_inside_image) {
        line.x1 = prev_img_pos_x;
        line.y1 = prev_img_pos_y;
        line.x2 = img_pos_x;
        line.y2 = img_pos_y;
    }

    if (ImGui::IsMouseDown(0) == 0) {
        line.do_refresh = 0;
    }

    ImGui::End();
#endif

    implot_wrapper(db, &line);

    //ImPlot::ShowDemoWindow();
    //ImGui::ShowDemoWindow();

    ImGui::End();

    return ret;
}


#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <GLFW/glfw3.h>         // Will drag system OpenGL headers
#include "proj.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "implot.h"
#include "imfilebrowser.h"
#include "opengl_helper.h"
#include "main_cli.h"
#include "version.h"
#include "implot_wrapper.h"
#include "imgui_wrapper.h"

struct idb_t {
    uint8_t actual_zoom;
    uint8_t return_state;
    unsigned int vp_width = 0;
    unsigned int vp_height = 0;
    unsigned int vp_texture = 0;
};

struct idb_t idb;

extern ImGui::FileBrowser fileDialog;
extern ImGuiContext *GImGui;

// Helper to display a little (?) mark which shows a tooltip when hovered.
// In your own code you may want to display an actual icon if you are using a merged icon fonts (see docs/FONTS.md)
static void HelpMarker(const char *desc)
{
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort) && ImGui::BeginTooltip()) {
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

void imgui_show_about(bool *p_open)
{
    if (!ImGui::Begin("About ThPP", p_open, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::End();
        return;
    }

    ImGui::Text("Thermal processing panel v%d.%d build %d commit #%d", VER_MAJOR, VER_MINOR, BUILD,
                COMMIT);
    ImGui::Separator();
    ImGui::Text("copyright Petre Rodan, 2023");
    ImGui::Text("ThPP is licensed under the GPLv3 License, see LICENSE for more information.");
    ImGui::End();
}

void imgui_show_properties(bool *p_open, th_db_t *db)
{
    rjpg_header_t *h;

    if (!ImGui::Begin("file properties", p_open, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::End();
        return;
    }

    switch (db->in_th->type) {
        case TH_FLIR_RJPG:
            h = db->in_th->head.rjpg;
            ImGui::Text("emissivity: %f", h->emissivity);
            ImGui::Text("object distance: %f", h->distance);
            ImGui::Text("relative humidity: %f", h->rh);
            ImGui::Text("atmospheric trans Alpha1: %f", h->alpha1);
            ImGui::Text("atmospheric trans Alpha2: %f", h->alpha2);
            ImGui::Text("atmospheric trans Beta1: %f", h->beta1);
            ImGui::Text("atmospheric trans Beta2: %f", h->beta2);
            ImGui::Text("planck r1: %f", h->planckR1);
            ImGui::Text("planck r2: %f", h->planckR2);
            ImGui::Text("planck b: %f", h->planckB);
            ImGui::Text("planck f: %f", h->planckF);
            ImGui::Text("planck o: %f", h->planckO);
            ImGui::Text("atmospheric TransX: %f", h->atm_trans_X);
            ImGui::Text("atmospheric temperature: %f", h->air_temp);
            ImGui::Text("reflected apparent temperature: %f", h->refl_temp);
            ImGui::Text("raw thermal image width: %u", h->raw_th_img_width);
            ImGui::Text("raw thermal image height: %u", h->raw_th_img_height);
            break;
        case TH_IRTIS_DTV:
            ImGui::Text("to be implemented");
            break;
    }
    

    ImGui::End();
}

int imgui_init_docking(th_db_t * db)
{
    // add docking
    //ImGuiIO& io = ImGui::GetIO();

    //if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
    //    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    //}

    //DockSpaceOverViewport();
    //ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());

    static bool opt_fullscreen = true;
    static bool opt_padding = false;
    static bool opt_open = false;
    bool opt_exit = false;
    static bool show_about = false;
    static bool show_properties = false;
    //bool opt_ignore;
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

    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Open ..")) {
                fileDialog.Open();
                opt_open = 1;
            }
            ImGui::MenuItem("Properties", NULL, &show_properties, 1);
            ImGui::Separator();
            ImGui::MenuItem("Exit", NULL, &opt_exit, 1);
            ImGui::EndMenu();
        }
        HelpMarker
            ("When docking is enabled, you can ALWAYS dock MOST window into another! Try it now!"
             "\n" "- Drag from window title bar or their tab to dock/undock." "\n"
             "- Drag from window menu button (upper-left button) to undock an entire node (all windows)."
             "\n" "- Hold SHIFT to disable docking (if io.ConfigDockingWithShift == false, default)"
             "\n" "- Hold SHIFT to enable docking (if io.ConfigDockingWithShift == true)" "\n"
             "This demo app has nothing to do with enabling docking!" "\n\n"
             "This demo app only demonstrate the use of ImGui::DockSpace() which allows you to manually create a docking node _within_ another window."
             "\n\n" "Read comments in ShowExampleAppDockSpace() for more details.");

        ImGui::MenuItem("About", NULL, &show_about);
        ImGui::EndMenuBar();
    }

    if (opt_open) {
        fileDialog.Display();

        if (fileDialog.HasSelected()) {
            printf("Selected filename %s\n", fileDialog.GetSelected().string().c_str());
            fileDialog.ClearSelected();
            opt_open = 0;
        }
    }

    if (show_about) {
        imgui_show_about(&show_about);
    }

    if (show_properties) {
        imgui_show_properties(&show_properties, db);
    }

    if (opt_exit) {
        idb.return_state = RET_EXIT;
        return RET_EXIT;
    }
    return 0;
}

void imgui_init_preferences(void)
{
    // window decorations, theme
    ImGuiStyle & style = ImGui::GetStyle();
    ImGui::StyleColorsClassic();
    style.FrameBorderSize = 1.0f;
}

void imgui_render_viewport(th_db_t * db, linedef_t * line)
{
    ImGuiIO & io = ImGui::GetIO();

    ImGui::Begin("viewport");
    ImVec2 screen_pos = ImGui::GetCursorScreenPos();

    if ((idb.vp_texture == 0) || (idb.return_state == RET_OK_REFRESH_NEEDED)) {
        //load_texture_from_file(db->p.out_file, &idb.vp_texture, &vp_width, &vp_height);
        idb.actual_zoom = db->p.zoom;
        idb.vp_width = db->rgba.width;
        idb.vp_height = db->rgba.height;
        load_texture_from_mem(db->rgba.data, &idb.vp_texture, idb.vp_width, idb.vp_height);
        //load_texture_from_mem(db->rgba.data, fb_get_texture_ptr(), vp_width, vp_height);
    } else {
        ImGui::Image((void *)(intptr_t) idb.vp_texture, ImVec2(idb.vp_width, idb.vp_height));
    }

    int16_t img_pos_x, img_pos_y;
    static int16_t prev_img_pos_x, prev_img_pos_y;
    img_pos_x = (io.MousePos.x - screen_pos.x) / idb.actual_zoom;
    img_pos_y = (io.MousePos.y - screen_pos.y) / idb.actual_zoom;
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
            ImGui::Text("spot %.2f??C",
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
            ImGui::Text("spot %.2f??C",
                        db->temp_arr[img_pos_y * db->in_th->head.dtv->nst + img_pos_x]);
            pointer_inside_image = 1;
        } else {
            pointer_inside_image = 0;
        }
        break;
    }

    if (pointer_inside_image && ImGui::IsMouseDown(0) && (line->do_refresh == 0)
        && pointer_over_viewport) {
        prev_img_pos_x = img_pos_x;
        prev_img_pos_y = img_pos_y;
        line->do_refresh = 1;
    }

    if (pointer_inside_image && ImGui::IsMouseDown(0) && pointer_over_viewport) {
        line->x1 = prev_img_pos_x;
        line->y1 = prev_img_pos_y;
        line->x2 = img_pos_x;
        line->y2 = img_pos_y;
    }

    if (ImGui::IsMouseDown(0) == 0) {
        line->do_refresh = 0;
    }
    ImGui::End();

    if (line->do_refresh) {
        ImGui::SetNextItemOpen(1, 0);
    }
}

void imgui_rended_processing_panel(th_db_t * db)
{
    double t_min, t_max;
    static uint8_t reset_changes = 0;
    static int show_apply_button = 0;

    ImGui::Begin("processing");

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

    // target zoom level
    static int s_zoom = db->p.zoom;
    ImGui::SliderInt("zoom [1..10]", &s_zoom, 1, 10);
    if (s_zoom != db->p.zoom) {
        db->p.zoom = s_zoom;
        show_apply_button = 1;
    }

    if (db->in_th->type == TH_IRTIS_DTV) {

        dtv_header_t *hd;
        hd = db->in_th->head.dtv;

        t_min = hd->tsc[1];
        t_max = hd->tsc[1] + 256.0 * hd->tsc[0];
        static float s_d_begin = t_min;
        static float s_d_end = t_max;
        if (reset_changes) {
            s_d_begin = t_min;
            s_d_end = t_max;
        }
        ImGui::DragFloatRange2("rescale [C]", &s_d_begin, &s_d_end, 0.5f, -20.0f, 300.0f,
                               "min: %.1fC", "max: %.1fC", ImGuiSliderFlags_AlwaysClamp);
        if ((fabs(s_d_begin - t_min) > 0.001) || (fabs(s_d_end - t_max) > 0.001)) {
            db->p.flags |= OPT_SET_NEW_MIN | OPT_SET_NEW_MAX;
            db->p.t_min = s_d_begin;
            db->p.t_max = s_d_end;
            show_apply_button = 1;
        }

        ImGui::Separator();

    } else if (db->in_th->type == TH_FLIR_RJPG) {
        rjpg_header_t *h;
        h = db->out_th->head.rjpg;

        t_min = h->t_min;
        t_max = h->t_max;
        static float s_begin = t_min;
        static float s_end = t_max;
        if (reset_changes) {
            s_begin = t_min;
            s_end = t_max;
        }
        ImGui::DragFloatRange2("rescale [C]", &s_begin, &s_end, 0.5f, -20.0f, 300.0f, "min: %.1fC",
                               "max: %.1fC", ImGuiSliderFlags_AlwaysClamp);
        if ((fabs(s_begin - t_min) > 0.001) || (fabs(s_end - t_max) > 0.001)) {
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
        if (fabs(s_distance - h->distance) > 0.001) {
            db->p.flags |= OPT_SET_DISTANCE_COMP | OPT_SET_NEW_DISTANCE;
            db->p.distance = s_distance;
            show_apply_button = 1;
        }

        static float s_emissivity = h->emissivity;
        if (reset_changes) {
            s_emissivity = h->emissivity;
        }
        ImGui::DragFloat("emissivity", &s_emissivity, 0.01f, 0.1f, 1.0f, "%0.2f");
        if (fabs(s_emissivity - h->emissivity) > 0.001) {
            db->p.flags |= OPT_SET_NEW_EMISSIVITY;
            db->p.emissivity = s_emissivity;
            show_apply_button = 1;
        }

        static float s_atm_temp = h->air_temp - RJPG_K;
        if (reset_changes) {
            s_atm_temp = h->air_temp - RJPG_K;
        }
        ImGui::DragFloat("atm temp [C]", &s_atm_temp, 1.0f, -20.1f, 300.0f, "%0.2f C");
        if (fabs(s_atm_temp - h->air_temp + RJPG_K) > 0.001) {
            db->p.flags |= OPT_SET_NEW_AT | OPT_SET_DISTANCE_COMP;
            db->p.atm_temp = s_atm_temp;
            show_apply_button = 1;
        }

        static float s_rh = h->rh * 100.0;
        if (reset_changes) {
            s_rh = h->rh * 100.0;
        }
        ImGui::DragFloat("rel humidity [%]", &s_rh, 1.0f, 0.1f, 100.0f, "%.0f %rH");
        if (fabs(s_rh / 100.0 - h->rh) > 0.001) {
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
        idb.return_state = RET_OK_REFRESH_NEEDED;
        reset_changes = 1;
    }

    ImGui::SameLine();

    if (show_apply_button) {
        if (ImGui::Button("apply changes")) {
            main_cli(db);
            show_apply_button = 0;
            idb.actual_zoom = db->p.zoom;
            idb.return_state = RET_OK_REFRESH_NEEDED;
        }
    }
    ImGui::End();
}

int imgui_wrapper(th_db_t * db)
{
    static linedef_t line;

    if (imgui_init_docking(db) == RET_EXIT) {
        return RET_EXIT;
    }
    imgui_init_preferences();

    idb.return_state = 0;
    imgui_rended_processing_panel(db);
    imgui_render_viewport(db, &line);

    implot_wrapper(db, &line);

    //ImGui::ShowDemoWindow();
    //ImPlot::ShowDemoWindow();

    ImGui::End();

    return idb.return_state;
}


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
#include "proj.h"
#include "version.h"
#include "implot_wrapper.h"
#include "imgui_wrapper.h"

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

void imgui_show_properties(bool *p_open, th_db_t * db)
{
    rjpg_header_t *h;

    if (!ImGui::Begin("file properties", p_open, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::End();
        return;
    }

    switch (db->in_th->type) {
    case TH_FLIR_RJPG:
        h = db->in_th->head.rjpg;
        ImGui::Text("emissivity: %.02f", h->emissivity);
        ImGui::Text("object distance: %.02f m", h->distance);
        ImGui::Text("relative humidity: %.02f rH", h->rh);
        ImGui::Text("atmospheric trans Alpha1: %f", h->alpha1);
        ImGui::Text("atmospheric trans Alpha2: %f", h->alpha2);
        ImGui::Text("atmospheric trans Beta1: %f", h->beta1);
        ImGui::Text("atmospheric trans Beta2: %f", h->beta2);
        ImGui::Text("planck r1: %f", h->planckR1);
        ImGui::Text("planck r2: %f", h->planckR2);
        ImGui::Text("planck b: %f", h->planckB);
        ImGui::Text("planck f: %f", h->planckF);
        ImGui::Text("planck o: %f", h->planckO);
        ImGui::Text("atmospheric TransX: %.02f", h->atm_trans_X);
        ImGui::Text("atmospheric temperature: %.02f K", h->air_temp);
        ImGui::Text("reflected apparent temperature: %.02f K", h->refl_temp);
        ImGui::Text("raw thermal image width: %u px", h->raw_th_img_width);
        ImGui::Text("raw thermal image height: %u px", h->raw_th_img_height);
        break;
    case TH_IRTIS_DTV:
        ImGui::Text("to be implemented");
        break;
    }

    ImGui::End();
}

uint8_t imgui_init_docking(th_db_t * db)
{
    static bool opt_fullscreen = true;
    static bool opt_padding = false;
    static bool opt_open = false;
    bool opt_exit = false;
    static bool show_about = false;
    static bool show_properties = false;
    static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;
    uint8_t ret = RET_OK;
    uint16_t path_size = 0;

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
            path_size = strlen(fileDialog.GetSelected().string().c_str());
            //printf("Selected filename %s %d\n", fileDialog.GetSelected().string().c_str(), path_size);
            cleanup();
            if (db->p.in_file) {
                free(db->p.in_file);
            }
            db->p.in_file = (char *) calloc(path_size + 1, sizeof(char));
            memcpy(db->p.in_file, fileDialog.GetSelected().string().c_str(), path_size + 1);
            db->p.in_file[path_size] = 0;
            fileDialog.ClearSelected();
            opt_open = 0;
            idb.return_state = RET_RST;
            ret = RET_RST;
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

    return ret;
}

void render_prop_table(th_db_t * db)
{
    double t_min, t_max;
    struct tm t;
    static ImGuiTableFlags flags =
        ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders |
        ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable;

    localtime_r(&(db->sb.st_mtime), &t);

    if (ImGui::BeginTable("prop_table", 2, flags)) {
        ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableHeadersRow();

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("IR: Date Of Creation");
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%d/%02d/%02d", t.tm_year + 1900, t.tm_mon + 1, t.tm_mday);

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("IR: Time Of Creation");
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%02d:%02d:%02d", t.tm_hour, t.tm_min, t.tm_sec);

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("IR: File Name");
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%s", basename(db->p.in_file));

        get_min_max(db->out_th, &t_min, &t_max);

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("IR: Min");
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%.02f C", t_min);

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("IR: Max");
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%.02f C", t_max);

#if 0
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("IR: Spot");
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("33 dC");
#endif
        ImGui::EndTable();
    }
}

void imgui_render_scale(th_db_t * db)
{
    ImGui::Begin("scale");
    if ((idb.si_texture == 0) || (idb.return_state == RET_OK_REFRESH_NEEDED) || (idb.return_state == RET_RST)) {
        db->scale.width = SCALE_WIDTH;
        db->scale.height = SCALE_HEIGHT;
        db->scale.pal_id = db->p.pal;

        if (db->p.flags & (OPT_SET_NEW_MIN | OPT_SET_NEW_MAX)) {
            db->scale.t_min = db->p.t_min;
            db->scale.t_max = db->p.t_max;
        } else {
            get_min_max(db->out_th, &db->scale.t_min, &db->scale.t_max);
        }
        idb.si_width = SCALE_WIDTH;
        idb.si_height = SCALE_HEIGHT;
        generate_scale(&db->scale);
        //load_texture_from_mem(db->scale.overlay, &idb.si_texture, idb.si_width, idb.si_height);
        load_texture_from_mem(db->scale.combo, &idb.si_texture, idb.si_width, idb.si_height);
    } else {
        ImGui::Image((void *)(intptr_t) idb.si_texture, ImVec2(idb.si_width, idb.si_height));
    }

    ImGui::End();
}

void imgui_render_viewport(th_db_t * db, linedef_t * line)
{
    ImGuiIO & io = ImGui::GetIO();

    ImGui::Begin("viewport");
    ImVec2 screen_pos = ImGui::GetCursorScreenPos();

    if ((idb.vp_texture == 0) || (idb.return_state == RET_OK_REFRESH_NEEDED) || (idb.return_state == RET_RST)) {
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

    if (ImGui::TreeNode("properties table")) {
        render_prop_table(db);
        ImGui::TreePop();
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

    if (idb.return_state != RET_OK) {
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

void imgui_render_processing_panel(th_db_t * db)
{
    double t_min, t_max;
    static uint8_t reset_changes_but = 0; // becomes true after the 'reset changes' button is pressed
    uint8_t reset_changes = idb.return_state || reset_changes_but;
    static int show_apply_button = 0;
    uint8_t value_changed;

    ImGui::Begin("processing");

    ImGui::Separator();
    ImGui::Text("input parameters");
    ImGui::Separator();

    // palette picker
    static int s_pal = db->p.pal;
    if (reset_changes) {
        s_pal = db->p.pal;
    }
    value_changed = ImGui::Combo("palette", &s_pal,
                 "256\0color\0grey\0hmetal0\0hmetal1\0hmetal2\0hotblue1\0hotblue2\0iron\0per_true\0pericolor\0rainbow\0rainbow0\0\0");
    if (value_changed) {
        db->p.pal = s_pal;
        show_apply_button = 1;
    }

    // target zoom level
    static int s_zoom = db->p.zoom;
    if (reset_changes) {
        s_zoom = db->p.zoom;
    }
    value_changed = ImGui::SliderInt("zoom [1..10]", &s_zoom, 1, 10);
    if (value_changed) {
        db->p.zoom = s_zoom;
        show_apply_button = 1;
    }

    get_min_max(db->out_th, &t_min, &t_max);
    static float s_begin = t_min;
    static float s_end = t_max;
    if (reset_changes) {
        s_begin = t_min;
        s_end = t_max;
    }
    value_changed = ImGui::DragFloatRange2("rescale [C]", &s_begin, &s_end, 0.5f, -20.0f, 300.0f, "min: %.1fC",
                           "max: %.1fC", ImGuiSliderFlags_AlwaysClamp);

    if (value_changed) {
        db->p.flags |= OPT_SET_NEW_MIN | OPT_SET_NEW_MAX;
        db->p.t_min = s_begin;
        db->p.t_max = s_end;
        show_apply_button = 1;
    }

    ImGui::Separator();

    if (db->in_th->type == TH_FLIR_RJPG) {
        rjpg_header_t *h;
        h = db->out_th->head.rjpg;

        // temperature compensation
        ImGui::Text("temperature compensation");
        ImGui::Separator();

        static float s_distance = h->distance;
        if (reset_changes) {
            s_distance = h->distance;
        }
        value_changed = ImGui::DragFloat("distance [m]", &s_distance, 0.2f, 0.2f, 100.0f, "%0.2f m");
        if (value_changed) {
            db->p.flags |= OPT_SET_DISTANCE_COMP | OPT_SET_NEW_DISTANCE;
            db->p.distance = s_distance;
            show_apply_button = 1;
        }

        static float s_emissivity = h->emissivity;
        if (reset_changes) {
            s_emissivity = h->emissivity;
        }
        value_changed = ImGui::DragFloat("emissivity", &s_emissivity, 0.01f, 0.1f, 1.0f, "%0.2f");
        if (value_changed) {
            db->p.flags |= OPT_SET_NEW_EMISSIVITY;
            db->p.emissivity = s_emissivity;
            show_apply_button = 1;
        }

        static float s_atm_temp = h->air_temp - RJPG_K;
        if (reset_changes) {
            s_atm_temp = h->air_temp - RJPG_K;
        }
        value_changed = ImGui::DragFloat("atm temp [C]", &s_atm_temp, 1.0f, -20.1f, 300.0f, "%0.2f C");
        if (value_changed) {
            db->p.flags |= OPT_SET_NEW_AT | OPT_SET_DISTANCE_COMP;
            db->p.atm_temp = s_atm_temp;
            show_apply_button = 1;
        }

        static float s_rh = h->rh * 100.0;
        if (reset_changes) {
            s_rh = h->rh * 100.0;
        }
        value_changed = ImGui::DragFloat("rel humidity [%]", &s_rh, 1.0f, 0.1f, 100.0f, "%.0f %rH");
        if (value_changed) {
            db->p.flags |= OPT_SET_NEW_RH | OPT_SET_DISTANCE_COMP;
            db->p.rh = s_rh / 100.0;
            show_apply_button = 1;
        }
    }

    ImGui::Separator();

    if (reset_changes_but) {
        reset_changes_but = 0;
    }

    if (ImGui::Button("reset changes")) {
        db->p.flags = 0;
        main_cli(db);
        show_apply_button = 0;
        idb.return_state = RET_OK_REFRESH_NEEDED;
        reset_changes_but = 1;
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

    idb.return_state = RET_OK;

    imgui_init_docking(db);

    if (idb.return_state == RET_EXIT) {
        // file/exit was pressed
        return RET_EXIT;
    } else if (idb.return_state == RET_RST) {
        // file/open dialog has closed with a file selection
        // and most of db has been freed
        main_cli(db);
        memset(&line, 0, sizeof(linedef_t));
    }

    imgui_render_processing_panel(db);

    if (idb.return_state == RET_OK_REFRESH_NEEDED) {
        // the 'apply changes' button was pressed
        // temperatures inside the image are bound to change
        // so invalidate the line plot
        memset(&line, 0, sizeof(linedef_t));
    }

    imgui_render_viewport(db, &line);
    imgui_render_scale(db);
    implot_wrapper(db, &line, &idb);

    ImGui::ShowDemoWindow();
    //ImPlot::ShowDemoWindow();

    return idb.return_state;
}

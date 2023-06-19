
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include "imgui.h"
#include "imgui_internal.h"
#include "proj.h"
#include "imgui_wrapper.h"
#include "main_cli.h"
#include "viewport.h"
#include "file_library.h"
#include "file_properties.h"
#include "tool_scale.h"
#include "tool_profile.h"
#include "tool_histogram.h"
#include "tool_processing.h"
#include "tool_export.h"
#include "tool_preferences.h"
#include "help_about.h"

#include "main_menu.h"

struct window_open {
    bool bhelp_about;
    bool bfile_library;
    bool bfile_properties;
    bool bfile_open;
    bool btool_processing;
    bool btool_histogram;
    bool btool_profile;
    bool btool_scale;
    bool btool_export;
    bool btool_preferences;
};

struct window_open wo;

uint8_t main_menu(th_db_t * db)
{
    static bool opt_fullscreen = true;
    static bool opt_padding = false;
    bool opt_exit = false;
    static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;
    uint8_t ret = RET_OK;

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
        if (ImGui::BeginMenu("file")) {
            ImGui::MenuItem("library", NULL, &wo.bfile_library, 1);
            ImGui::MenuItem("properties", NULL, &wo.bfile_properties, 1);
            ImGui::Separator();
            ImGui::MenuItem("exit", NULL, &opt_exit, 1);
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("tools")) {
            ImGui::MenuItem("processing", NULL, &wo.btool_processing, 1);
            ImGui::MenuItem("histogram", NULL, &wo.btool_histogram, 1);
            ImGui::MenuItem("profile", NULL, &wo.btool_profile, 1);
            ImGui::MenuItem("scale", NULL, &wo.btool_scale, 1);
            ImGui::MenuItem("export", NULL, &wo.btool_export, 1);
            ImGui::Separator();
            ImGui::MenuItem("preferences", NULL, &wo.btool_preferences, 1);
            ImGui::EndMenu();
        }
        ImGui::MenuItem("about", NULL, &wo.bhelp_about);
        ImGui::EndMenuBar();
    }

    if (wo.bhelp_about) {
        help_about(&wo.bhelp_about);
    }

    if (wo.bfile_library) {
        file_library(&wo.bfile_library, db);
    }

    if (wo.bfile_properties) {
        file_properties(&wo.bfile_properties, db);
    }

    if (wo.btool_processing) {
        tool_processing(&wo.btool_processing, db);
    }

    if (db->fe.return_state == RET_OK_REFRESH_NEEDED) {
        // the 'apply changes' button was pressed
        // temperatures inside the image are bound to change
        // so invalidate the line plot
        memset(&db->pr, 0, sizeof(profile_t));
    }

    if (wo.btool_histogram && db->fe.return_state == RET_OK) {
        tool_histogram(&wo.btool_histogram, db);
    }

    if (wo.btool_profile && db->fe.return_state == RET_OK) {
        tool_profile(&wo.btool_profile, db);
    }

    if (wo.btool_scale) {
        tool_scale(&wo.btool_scale, db);
    }

    if (wo.btool_export) {
        tool_export(&wo.btool_export, db);
    }

    if (wo.btool_preferences) {
        tool_preferences(&wo.btool_preferences, db);
    }

    viewport_render(db);

    if (opt_exit) {
        db->fe.return_state = RET_EXIT;
        return RET_EXIT;
    }

    return ret;
}


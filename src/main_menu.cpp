
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include "imgui.h"
#include "imfilebrowser.h"
#include "proj.h"
#include "imgui_wrapper.h"
#include "main_cli.h"
#include "help_about.h"
#include "file_properties.h"
#include "main_menu.h"

extern ImGui::FileBrowser fileDialog;

uint8_t main_menu(th_db_t * db)
{
    static bool opt_fullscreen = true;
    static bool opt_padding = false;
    static bool opt_open = false;
    bool opt_exit = false;
    static bool show_about = false;
    static bool show_properties = false;
    static bool show_properties_extra = false;
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
            ImGui::MenuItem("Properties", NULL, &show_properties_extra, 1);
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
            db->fe.return_state = RET_RST;
            ret = RET_RST;
        }
    }

    if (show_about) {
        help_about(&show_about);
    }

    if (show_properties_extra) {
        file_properties(&show_properties, db);
    }

    if (show_properties_extra) {
        file_properties_extra(&show_properties_extra, db);
    }

    if (opt_exit) {
        db->fe.return_state = RET_EXIT;
        return RET_EXIT;
    }

    return ret;
}



#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <filesystem>
#include "imgui.h"
#include "proj.h"
#include "opengl_helper.h"
#include "version.h"
#include "file_library.h"

uint32_t file_tx;
uint32_t dir_tx;

void file_library_init(void)
{
    unsigned int w, h;
    file_tx = 0;
    dir_tx = 0;

    if (load_texture_from_file("res/file_icon.png", &file_tx, &w, &h) != EXIT_SUCCESS) {
        fprintf(stderr, "error loading file icon\n");
    }
    if (load_texture_from_file("res/dir_icon.png", &dir_tx, &w, &h) != EXIT_SUCCESS) {
        fprintf(stderr, "error loading directory icon\n");
    }
}

void file_library(bool *p_open, th_db_t * db)
{
    if (!ImGui::Begin("image library", p_open, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::End();
        return;
    }

    uint32_t texture;
    static float padding = 16.0f;
    static float thumbnail_size = 128.0f;
    float cell_size = thumbnail_size + padding;

    float panel_width = ImGui::GetContentRegionAvail().x;
    int column_count = (int)(panel_width / cell_size);
    if (column_count < 1)
        column_count = 1;

    ImGui::Columns(column_count, 0, false);
    static std::filesystem::path m_current_directory = std::filesystem::current_path();

    ImVec4 bg_col = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);             // Black background
    ImVec4 tint_col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);           // No tint

    ImGui::PushID("../");
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
    ImGui::ImageButton("", (void *)(intptr_t) dir_tx, {thumbnail_size, thumbnail_size}, {1, 0}, {0, 1}, bg_col, tint_col);
    if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
	    m_current_directory = m_current_directory.parent_path();
	}
    ImGui::Text("../");
    ImGui::NextColumn();
    ImGui::PopID();

    for (auto & directory_entry:std::filesystem::directory_iterator(m_current_directory)) {
        const auto & path = directory_entry.path();

        std::string filename_string = path.filename().string();

        ImGui::PushID(filename_string.c_str());
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));

        if (directory_entry.is_directory()) {
            texture = dir_tx;
        } else {
            texture = file_tx;
        }

        ImGui::ImageButton("",(void *)(intptr_t) texture, {thumbnail_size, thumbnail_size}, {1, 0}, {0, 1}, bg_col, tint_col);

        ImGui::PopStyleColor();
        if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
            if (directory_entry.is_directory()) {
                m_current_directory /= path.filename();
            } else {
                printf("file %s has been clicked in %s\n", filename_string.c_str(), m_current_directory.c_str() );
            }
        }

        ImGui::TextWrapped(filename_string.c_str());
        ImGui::NextColumn();
        ImGui::PopID();
    }

    ImGui::End();
}

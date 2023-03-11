
#include <stdlib.h>
#include <stdint.h>
#include <GLFW/glfw3.h> // Will drag system OpenGL headers
#include "proj.h"
#include "imgui.h"
#include "lodepng.h"
#include "main_cli.h"

//SDL_Texture *vp_texture = NULL;
unsigned int vp_width = 0;
unsigned int vp_height = 0;
GLuint vp_texture = 0;

// Simple helper function to load an image into a OpenGL texture with common settings
bool load_texture_from_file(const char *filename, GLuint * out_texture, unsigned int *out_width,
                         unsigned int *out_height)
{
    // Load from file
    unsigned int image_width = 0;
    unsigned int image_height = 0;
    unsigned char *image_data = NULL;

    //unsigned char *image_data = stbi_load(filename, &image_width, &image_height, NULL, 4);
    lodepng_decode32_file(&image_data, &image_width, &image_height, filename);
    if (image_data == NULL)
        return false;

    // Create a OpenGL texture identifier
    GLuint image_texture;
    glGenTextures(1, &image_texture);
    glBindTexture(GL_TEXTURE_2D, image_texture);

    // Setup filtering parameters for display
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);        // This is required on WebGL for non power-of-two textures
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);        // Same

    // Upload pixels into texture
#if defined(GL_UNPACK_ROW_LENGTH) && !defined(__EMSCRIPTEN__)
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#endif
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_width, image_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
    //stbi_image_free(image_data);
    free(image_data);

    *out_texture = image_texture;
    *out_width = image_width;
    *out_height = image_height;

    return true;
}

int imgui_wrapper(th_db_t *db)
{
    int ret = RET_OK;
    static int show_apply_button = 0;
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
    ImGui::Combo("palette", &s_pal, "256\0color\0grey\0hmetal0\0hmetal1\0hmetal2\0hotblue1\0hotblue2\0iron\0per_true\0pericolor\0rainbow\0rainbow0\0\0");
    if (s_pal != db->p.pal) {
        db->p.pal = s_pal;
        show_apply_button = 1;
    }

    // zoom level
    static int s_zoom = db->p.zoom;
    ImGui::SliderInt("zoom [1..10]", &s_zoom, 1, 10);
    if (s_zoom != db->p.zoom) {
        db->p.zoom = s_zoom;
        show_apply_button = 1;
    }


    if (db->in_th->type == TH_FLIR_RJPG) {
        rjpg_header_t *h;
        h = db->out_th->head.rjpg;

        static float s_begin = h->t_min;
        static float s_end = h->t_max;
        ImGui::DragFloatRange2("rescale [C]", &s_begin, &s_end, 0.5f, -20.0f, 300.0f, "min: %.1fC", "max: %.1fC", ImGuiSliderFlags_AlwaysClamp);
        if ((s_begin != h->t_min) || (s_end != h->t_max)) {
            db->p.flags |= OPT_SET_NEW_MIN | OPT_SET_NEW_MAX;
            db->p.t_min = s_begin;
            db->p.t_max = s_end;
            show_apply_button = 1;
        }

        ImGui::Separator();

        // temperature compensation
        if (db->in_th->type == TH_FLIR_RJPG) {
            ImGui::Text("temperature compensation");
            ImGui::Separator();

            static float s_distance = h->distance;
            ImGui::DragFloat("distance [m]", &s_distance, 0.2f, 0.2f, 100.0f, "%0.2f m");
            if (s_distance != h->distance) {
                db->p.flags |= OPT_SET_DISTANCE_COMP;
                db->p.distance = s_distance;
                show_apply_button = 1;
            }

            static float s_emissivity = h->emissivity;
            ImGui::DragFloat("emissivity", &s_emissivity, 0.01f, 0.1f, 1.0f, "%0.2f");
            if (s_emissivity != h->emissivity) {
                db->p.flags |= OPT_SET_NEW_EMISSIVITY;
                db->p.emissivity = s_emissivity;
                show_apply_button = 1;
            }
        }
    }

    if (show_apply_button) {
        if (ImGui::Button("apply changes")) {
            main_cli(db);
            show_apply_button = 0;
            ret = RET_OK_REFRESH_NEEDED;
        }
    }

    ImGui::End();

    ImGui::Begin("Viewport");

    if ((vp_texture == 0) || (ret == RET_OK_REFRESH_NEEDED)) {
        load_texture_from_file(db->p.out_file, &vp_texture, &vp_width, &vp_height);
    } else {
        //ImGui::Text("pointer = %p", vp_texture);
        //ImGui::Text("size = %d x %d", vp_width, vp_height);
        ImGui::Image((void*)(intptr_t)vp_texture, ImVec2(vp_width, vp_height));
    }
    ImGui::End();


    ImGui::ShowDemoWindow();

    ImGui::End();

    return ret;
}


#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <linux/limits.h>
#include "imgui.h"
#include "proj.h"
#include "lodepng.h"
#include "imgui_wrapper.h"
#include "tool_export.h"

#define    PREFIX_MAX  64

static char buf_dst_dir[PATH_MAX] = "/tmp";
static char buf_prefix[PREFIX_MAX] = {};
static char buf_output[PATH_MAX] = {};

style_t tool_export_style = {};

void tool_export(bool *p_open, th_db_t *db)
{
    unsigned err = 0;
    uint16_t buf_prefix_len;

    if (!ImGui::Begin("export", p_open, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::End();
        return;
    }

    if (db->out_th == NULL) {
        ImGui::Text("file not opened");
        ImGui::End();
        return;
    }

    ImGui::Text("path elements");
    ImGui::InputText("dest dir", buf_dst_dir, PATH_MAX);

    // populate filename prefix if new thermogram was loaded
    if ( !(db->fe.flags & TOOL_EXPORT_GOT_BASENAME) ) {
        snprintf(buf_prefix, PREFIX_MAX, basename(db->p.in_file));
        buf_prefix_len = strlen(buf_prefix);
        if (buf_prefix_len > 6) {
            if (buf_prefix[buf_prefix_len - 4] == '.') {
                buf_prefix[buf_prefix_len - 4] = 0;
            }
        }
        db->fe.flags |= TOOL_EXPORT_GOT_BASENAME;
    }

    ImGui::InputText("prefix", buf_prefix, PREFIX_MAX); //, ImGuiInputTextFlags_CallbackHistory, MyCallback);

    if (ImGui::Button("export image and scale")) {
        style_set(STYLE_LIGHT, &tool_export_style);
        generate_scale(db, &tool_export_style);
        snprintf(buf_output, PATH_MAX, "%s/%s.png", buf_dst_dir, buf_prefix);
        printf("saving image to %s\n", buf_output);
        err = lodepng_encode32_file(buf_output, db->rgba_vp->data, db->rgba_vp->width, db->rgba_vp->height);
        if (err) {
            fprintf(stderr, "encoder error %u: %s\n", err, lodepng_error_text(err));
        }

        snprintf(buf_output, PATH_MAX, "%s/%s_scale.png", buf_dst_dir, buf_prefix);
        printf("saving scale to %s\n", buf_output);
        err = lodepng_encode32_file(buf_output, db->scale.combo, db->scale.width, db->scale.height);
        if (err) {
            fprintf(stderr, "encoder error %u: %s\n", err, lodepng_error_text(err));
        }
    }

    ImGui::Separator();
    ImGui::Text("template");

    //static char buf3[64];
    //ImGui::InputText("template", buf3, 64, ImGuiInputTextFlags_CallbackEdit, MyCallback);

    if (ImGui::Button("generate latex report")) {
    }

    ImGui::End();
}

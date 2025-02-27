
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include "proj.h"
#include "implot.h"
#include "opengl_helper.h"
#include "tool_scale.h"

void tool_scale(bool *p_open, th_db_t * db)
{
    global_preferences_t *pref;

    if (!ImGui::Begin("scale", p_open, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::End();
        return;
    }

    if (db->out_th == NULL) {
        ImGui::Text("file not opened");
        ImGui::End();
        return;
    }

    if ((db->fe.si_texture == 0) || (db->fe.return_state == RET_OK_REFRESH_NEEDED) || (db->fe.return_state == RET_RST)) {
        pref = gp_get_ptr();
        generate_scale(db, &pref->style);
        load_texture_from_mem(db->scale.combo, &db->fe.si_texture, db->fe.si_width, db->fe.si_height);
    } else {
        ImGui::Image((ImTextureID)(intptr_t) db->fe.si_texture, ImVec2(db->fe.si_width, db->fe.si_height));
    }

    ImGui::End();
}



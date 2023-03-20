
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
        db->scale.width = SCALE_WIDTH;
        db->scale.height = SCALE_HEIGHT;
        db->scale.pal_id = db->p.pal;

        if (db->p.flags & (OPT_SET_NEW_MIN | OPT_SET_NEW_MAX)) {
            db->scale.t_min = db->p.t_min;
            db->scale.t_max = db->p.t_max;
        } else {
            get_min_max(db->out_th, &db->scale.t_min, &db->scale.t_max);
        }
        db->fe.si_width = SCALE_WIDTH;
        db->fe.si_height = SCALE_HEIGHT;
        generate_scale(&db->scale);
        //load_texture_from_mem(db->scale.overlay, &db->fe.si_texture, db->fe.si_width, db->fe.si_height);
        load_texture_from_mem(db->scale.combo, &db->fe.si_texture, db->fe.si_width, db->fe.si_height);
    } else {
        ImGui::Image((void *)(intptr_t) db->fe.si_texture, ImVec2(db->fe.si_width, db->fe.si_height));
    }

    ImGui::End();
}



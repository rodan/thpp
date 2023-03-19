
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include "proj.h"
#include "implot.h"
#include "opengl_helper.h"
#include "tool_scale.h"


void tool_scale(th_db_t * db, idb_t *idb)
{
    ImGui::Begin("scale");
    if ((idb->si_texture == 0) || (idb->return_state == RET_OK_REFRESH_NEEDED) || (idb->return_state == RET_RST)) {
        db->scale.width = SCALE_WIDTH;
        db->scale.height = SCALE_HEIGHT;
        db->scale.pal_id = db->p.pal;

        if (db->p.flags & (OPT_SET_NEW_MIN | OPT_SET_NEW_MAX)) {
            db->scale.t_min = db->p.t_min;
            db->scale.t_max = db->p.t_max;
        } else {
            get_min_max(db->out_th, &db->scale.t_min, &db->scale.t_max);
        }
        idb->si_width = SCALE_WIDTH;
        idb->si_height = SCALE_HEIGHT;
        generate_scale(&db->scale);
        //load_texture_from_mem(db->scale.overlay, &idb.si_texture, idb.si_width, idb.si_height);
        load_texture_from_mem(db->scale.combo, &idb->si_texture, idb->si_width, idb->si_height);
    } else {
        ImGui::Image((void *)(intptr_t) idb->si_texture, ImVec2(idb->si_width, idb->si_height));
    }

    ImGui::End();
}



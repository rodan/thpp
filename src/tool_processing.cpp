
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "imgui.h"
#include "proj.h"
#include "thermogram.h"
#include "main_cli.h"
#include "viewport.h"
#include "tool_processing.h"

uint8_t check_th(tgram_t *th)
{
    if (th == NULL) {
        return EXIT_FAILURE;
    }

    if (th->frame == NULL) {
        return EXIT_FAILURE;
    }

    switch (th->type) {
        case TH_IRTIS_DTV:
            if (th->head.dtv == NULL) {
                return EXIT_FAILURE;
            }
            break;
        case TH_FLIR_RJPG:
            if (th->head.rjpg == NULL) {
                return EXIT_FAILURE;
            }
            break;
        default:
            return EXIT_FAILURE;
            break;
    }

    return EXIT_SUCCESS;
}


uint8_t proc_get_limits(tgram_t *th, proc_limits_t *data)
{
    uint32_t frame_sz = 0;
    uint32_t i;
    uint8_t c;
    uint32_t total = 0;

    if (check_th(th) == EXIT_FAILURE) {
        return EXIT_FAILURE;
    }

    switch (th->type) {
        case TH_IRTIS_DTV:
            frame_sz = th->head.dtv->nst * th->head.dtv->nstv;
            break;
        case TH_FLIR_RJPG:
            frame_sz = th->head.rjpg->raw_th_img_width * th->head.rjpg->raw_th_img_height;
            break;
        default:
            return EXIT_FAILURE;
            break;
    }

    if (frame_sz == 0) {
        return EXIT_FAILURE;
    }

    memset(data, 0, sizeof(proc_limits_t));

    data->umin = 255;
    data->umax = 0;
    data->fmin = 65000.0;
    data->fmax = -65000.0;
    
    for (i=0; i<frame_sz; i++) {
        c = th->frame[i];
        if (data->umin > c) {
            data->umin = c;
        }
        if (data->umax < c) {
            data->umax = c;
        }
        total += c;
    }

    data->uavg = total / frame_sz;

    return EXIT_SUCCESS;
}

void tool_processing(bool *p_open, th_db_t * db)
{
    double t_min, t_max;
    static uint8_t reset_changes_but = 0; // becomes true after the 'reset changes' button is pressed
    uint8_t reset_changes = db->fe.return_state || reset_changes_but;
    static int show_apply_button = 0;
    uint8_t value_changed;

    if (!ImGui::Begin("processing", p_open, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::End();
        return;
    }

    if ((db->in_th == NULL) || (db->out_th == NULL))  {
        ImGui::Text("file not opened");
        ImGui::End();
        return;
    }

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
        viewport_refresh_vp(db);
        show_apply_button = 0;
        db->fe.return_state = RET_OK_REFRESH_NEEDED;
        reset_changes_but = 1;
    }

    ImGui::SameLine();

    if (show_apply_button) {
        if (ImGui::Button("apply changes")) {
            main_cli(db);
            viewport_refresh_vp(db);
            show_apply_button = 0;
            db->fe.actual_zoom = db->p.zoom;
            db->fe.return_state = RET_OK_REFRESH_NEEDED;
        }
    }
    ImGui::End();
}




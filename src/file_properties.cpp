
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include "imgui.h"
#include "proj.h"
#include "file_properties.h"

fp_visibility_t v;

void file_properties(bool *p_open, th_db_t * db)
{
    double t_min, t_max, t_avg;
    struct tm t;
    rjpg_header_t *hf;
    dtv_header_t *hi;
    static ImGuiTableFlags flags =
        ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders |
        ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable;


    if (!ImGui::Begin("file properties", p_open, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::End();
        return;
    }

    if ((db->out_th == NULL) || (db->in_th == NULL)) {
        ImGui::Text("file not opened");
        ImGui::End();
        return;
    }

    localtime_r(&(db->sb.st_mtime), &t);

    if (ImGui::BeginTable("prop_table", 2, flags)) {
        ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableHeadersRow();

        if (v.ir_date) {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("IR: Date Of Creation");
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%d/%02d/%02d", t.tm_year + 1900, t.tm_mon + 1, t.tm_mday);
        }

        if (v.ir_time) {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("IR: Time Of Creation");
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%02d:%02d:%02d", t.tm_hour, t.tm_min, t.tm_sec);
        }

        if (v.ir_fname) {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("IR: File Name");
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%s", basename(db->p.in_file));
        }

        get_min_max(db->out_th, &t_min, &t_max);
        get_avg(db->out_th, &t_avg);

        if (v.ir_min) {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("IR: Min");
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%.02f C", t_min);
        }

        if (v.ir_max) {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("IR: Max");
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%.02f C", t_max);
        }

        if (v.ir_avg) {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("IR: Avg");
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%.02f C", t_avg);
        }

        switch (db->in_th->type) {
        case TH_FLIR_RJPG:
            hf = db->in_th->head.rjpg;

            if (v.camera_make) {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("camera make");
                ImGui::TableSetColumnIndex(1);
                ImGui::Text("%s", hf->camera_make);
            }

            if (v.camera_model) {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("camera model");
                ImGui::TableSetColumnIndex(1);
                ImGui::Text("%s", hf->camera_model);
            }

            if (v.emissivity) {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("emissivity");
                ImGui::TableSetColumnIndex(1);
                ImGui::Text("%.02lf", hf->emissivity);
            }

            if (v.distance) {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("distance");
                ImGui::TableSetColumnIndex(1);
                ImGui::Text("%.02lf", hf->distance);
            }

            if (v.rh) {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("relative humidity");
                ImGui::TableSetColumnIndex(1);
                ImGui::Text("%.02lf", hf->rh);
            }

            if (v.alpha1) {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("atmospheric trans Alpha1");
                ImGui::TableSetColumnIndex(1);
                ImGui::Text("%lf", hf->alpha1);
            }

            if (v.alpha2) {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("atmospheric trans Alpha2");
                ImGui::TableSetColumnIndex(1);
                ImGui::Text("%lf", hf->alpha2);
            }

            if (v.beta1) {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("atmospheric trans Beta1");
                ImGui::TableSetColumnIndex(1);
                ImGui::Text("%lf", hf->beta1);
            }

            if (v.beta2) {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("atmospheric trans Beta2");
                ImGui::TableSetColumnIndex(1);
                ImGui::Text("%lf", hf->beta2);
            }

            if (v.atm_trans_X) {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("atmospheric TransX");
                ImGui::TableSetColumnIndex(1);
                ImGui::Text("%lf", hf->atm_trans_X);
            }

            if (v.planckR1) {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("planck r1");
                ImGui::TableSetColumnIndex(1);
                ImGui::Text("%lf", hf->planckR1);
            }

            if (v.planckR2) {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("planck r2");
                ImGui::TableSetColumnIndex(1);
                ImGui::Text("%lf", hf->planckR2);
            }

            if (v.planckB) {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("planck b");
                ImGui::TableSetColumnIndex(1);
                ImGui::Text("%lf", hf->planckB);
            }

            if (v.planckF) {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("planck f");
                ImGui::TableSetColumnIndex(1);
                ImGui::Text("%lf", hf->planckF);
            }

            if (v.planckO) {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("planck o");
                ImGui::TableSetColumnIndex(1);
                ImGui::Text("%lf", hf->planckO);
            }

            if (v.air_temp) {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("atmospheric temperature");
                ImGui::TableSetColumnIndex(1);
                ImGui::Text("%.02lf", hf->air_temp);
            }

            if (v.refl_temp) {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("reflected apparent temperature");
                ImGui::TableSetColumnIndex(1);
                ImGui::Text("%.02lf", hf->refl_temp);
            }

            if (v.raw_th_img_width) {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("raw thermal image width");
                ImGui::TableSetColumnIndex(1);
                ImGui::Text("%u", hf->raw_th_img_width);
            }

            if (v.raw_th_img_height) {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("raw thermal image height");
                ImGui::TableSetColumnIndex(1);
                ImGui::Text("%u", hf->raw_th_img_height);
            }


            break;
        case TH_IRTIS_DTV:
            hi = db->in_th->head.dtv;

            if (v.raw_th_img_width) {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("raw thermal image width");
                ImGui::TableSetColumnIndex(1);
                ImGui::Text("%u", hi->nst);
            }

            if (v.raw_th_img_height) {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("raw thermal image height");
                ImGui::TableSetColumnIndex(1);
                ImGui::Text("%u", hi->nstv);
            }
            if (v.ir_comment) {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("IR: comment");
                ImGui::TableSetColumnIndex(1);
                ImGui::Text("%s", hi->inform);
            }

            break;
        }

#if 0
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("IR: Spot");
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("33 dC");
#endif
        ImGui::EndTable();
    }
    ImGui::End();
}


fp_visibility_t *file_properties_get_ptr(void)
{
    return &v;
}


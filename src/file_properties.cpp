
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include "imgui.h"
#include "proj.h"
#include "file_properties.h"

void file_properties(bool *p_open, th_db_t * db)
{
    double t_min, t_max;
    struct tm t;
    static ImGuiTableFlags flags =
        ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders |
        ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable;


    if (!ImGui::Begin("file properties", p_open, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::End();
        return;
    }

    if (db->out_th == NULL) {
        ImGui::Text("file not opened");
        ImGui::End();
        return;
    }

    localtime_r(&(db->sb.st_mtime), &t);

    if (ImGui::BeginTable("prop_table", 2, flags)) {
        ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableHeadersRow();

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("IR: Date Of Creation");
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%d/%02d/%02d", t.tm_year + 1900, t.tm_mon + 1, t.tm_mday);

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("IR: Time Of Creation");
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%02d:%02d:%02d", t.tm_hour, t.tm_min, t.tm_sec);

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("IR: File Name");
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%s", basename(db->p.in_file));

        get_min_max(db->out_th, &t_min, &t_max);

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("IR: Min");
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%.02f C", t_min);

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("IR: Max");
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%.02f C", t_max);

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


void file_properties_extra(bool *p_open, th_db_t * db)
{
    rjpg_header_t *h;

    if (!ImGui::Begin("file details", p_open, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::End();
        return;
    }

    if (db->in_th == NULL) {
        ImGui::Text("file not opened");
        ImGui::End();
        return;
    }

    switch (db->in_th->type) {
    case TH_FLIR_RJPG:
        h = db->in_th->head.rjpg;
        ImGui::Text("emissivity: %.02f", h->emissivity);
        ImGui::Text("object distance: %.02f m", h->distance);
        ImGui::Text("relative humidity: %.02f rH", h->rh);
        ImGui::Text("atmospheric trans Alpha1: %f", h->alpha1);
        ImGui::Text("atmospheric trans Alpha2: %f", h->alpha2);
        ImGui::Text("atmospheric trans Beta1: %f", h->beta1);
        ImGui::Text("atmospheric trans Beta2: %f", h->beta2);
        ImGui::Text("planck r1: %f", h->planckR1);
        ImGui::Text("planck r2: %f", h->planckR2);
        ImGui::Text("planck b: %f", h->planckB);
        ImGui::Text("planck f: %f", h->planckF);
        ImGui::Text("planck o: %f", h->planckO);
        ImGui::Text("atmospheric TransX: %.02f", h->atm_trans_X);
        ImGui::Text("atmospheric temperature: %.02f K", h->air_temp);
        ImGui::Text("reflected apparent temperature: %.02f K", h->refl_temp);
        ImGui::Text("raw thermal image width: %u px", h->raw_th_img_width);
        ImGui::Text("raw thermal image height: %u px", h->raw_th_img_height);
        break;
    case TH_IRTIS_DTV:
        ImGui::Text("to be implemented");
        break;
    }

    ImGui::End();
}


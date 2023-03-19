
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include "imgui.h"
#include "proj.h"
#include "file_properties.h"

void file_properties(bool *p_open, th_db_t * db)
{
    rjpg_header_t *h;

    if (!ImGui::Begin("file properties", p_open, ImGuiWindowFlags_AlwaysAutoResize)) {
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



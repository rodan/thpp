
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include "imgui.h"
#include "proj.h"
#include "tool_export.h"
#include "file_properties.h"

fp_visibility_t v;

#define  TEXT_LEN  64

char label[TEXT_LEN] = {};
char value[TEXT_LEN] = {};

void file_properties_add_row(char *label, char *value, FILE *report_table_file, const uint16_t flags)
{
    if (flags & FILE_PROPERTIES_OUT_GUI) {
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text(label);
        ImGui::TableSetColumnIndex(1);
        ImGui::Text(value);
    }

    if (flags & FILE_PROPERTIES_OUT_FILE) {
        fprintf(report_table_file, "  %s & %s \\\\ \\hline\n", label, value);
    }
}

void file_properties(th_db_t * db, FILE *report_table_file, const uint16_t flags)
{
    double t_min, t_max, t_avg;
    struct tm t;
    rjpg_header_t *hf;
    dtv_header_t *hi;

    if (flags == 0) {
        return;
    }

    if ((flags & FILE_PROPERTIES_OUT_FILE) && (report_table_file == NULL)) {
        return;
    }

    localtime_r(&(db->sb.st_mtime), &t);

    if (v.ir_fname) {
        strncpy(label, "IR: File Name", TEXT_LEN);
        snprintf(value, TEXT_LEN, "%s", basename(db->p.in_file));
        file_properties_add_row(label, value, report_table_file, flags);
    }

    get_min_max(db->out_th, &t_min, &t_max);
    get_avg(db->out_th, &t_avg);

    if (v.ir_min) {
        strncpy(label, "IR: Min", TEXT_LEN);
        snprintf(value, TEXT_LEN, "%.02f C", t_min);
        file_properties_add_row(label, value, report_table_file, flags);
    }

    if (v.ir_max) {
        strncpy(label, "IR: Max", TEXT_LEN);
        snprintf(value, TEXT_LEN, "%.02f C", t_max);
        file_properties_add_row(label, value, report_table_file, flags);
    }

    if (v.ir_avg) {
        strncpy(label, "IR: Avg", TEXT_LEN);
        snprintf(value, TEXT_LEN, "%.02f C", t_avg);
        file_properties_add_row(label, value, report_table_file, flags);
    }

    if (v.hl_min) {
        //strncpy(label, "highlight: Min", TEXT_LEN);
        snprintf(label, TEXT_LEN, "%s: Min", tool_export_get_buf_highlight());
        snprintf(value, TEXT_LEN, "%.02f C", db->pr.hl_min);
        file_properties_add_row(label, value, report_table_file, flags);
    }

    if (v.hl_max) {
        //strncpy(label, "highlight: Max", TEXT_LEN);
        snprintf(label, TEXT_LEN, "%s: Max", tool_export_get_buf_highlight());
        snprintf(value, TEXT_LEN, "%.02f C", db->pr.hl_max);
        file_properties_add_row(label, value, report_table_file, flags);
    }

    if (v.hl_avg) {
        //strncpy(label, "highlight: Avg", TEXT_LEN);
        snprintf(label, TEXT_LEN, "%s: Avg", tool_export_get_buf_highlight());
        snprintf(value, TEXT_LEN, "%.02f C", db->pr.hl_avg);
        file_properties_add_row(label, value, report_table_file, flags);
    }

    switch (db->in_th->type) {
    case TH_FLIR_RJPG:
        hf = db->in_th->head.rjpg;

        if (v.camera_make) {
            strncpy(label, "camera make", TEXT_LEN);
            strncpy(value, hf->camera_make, TEXT_LEN);
            file_properties_add_row(label, value, report_table_file, flags);
        }

        if (v.camera_model) {
            strncpy(label, "camera model", TEXT_LEN);
            strncpy(value, hf->camera_model, TEXT_LEN);
            file_properties_add_row(label, value, report_table_file, flags);
        }

        if (v.byte_order) {
            strncpy(label, "byte order", TEXT_LEN);
            if (hf->byte_order == ID_FLIR_LITTLE_ENDIAN) {
                strncpy(value, "little endian", TEXT_LEN);
            } else if (hf->byte_order == ID_FLIR_BIG_ENDIAN) {
                strncpy(value, "big endian", TEXT_LEN);
            } else {
                strncpy(value, "unknown", TEXT_LEN);
            }
            file_properties_add_row(label, value, report_table_file, flags);
        }


        if (v.emissivity) {
            if (db->p.flags & OPT_SET_NEW_EMISSIVITY) {
                strncpy(label, "emissivity (*)", TEXT_LEN);
                snprintf(value, TEXT_LEN, "%.02lf", db->p.emissivity);
            } else {
                strncpy(label, "emissivity", TEXT_LEN);
                snprintf(value, TEXT_LEN, "%.02lf", hf->emissivity);
            }
            file_properties_add_row(label, value, report_table_file, flags);
        }

        if (v.distance) {
            if (db->p.flags & OPT_SET_NEW_DISTANCE) {
                strncpy(label, "distance (*)", TEXT_LEN);
                snprintf(value, TEXT_LEN, "%.02lf", db->p.distance);
            } else {
                strncpy(label, "distance", TEXT_LEN);
                snprintf(value, TEXT_LEN, "%.02lf", hf->distance);
            }
            file_properties_add_row(label, value, report_table_file, flags);
        }

        if (v.rh) {
            if (db->p.flags & OPT_SET_NEW_RH) {
                strncpy(label, "relative humidity (*)", TEXT_LEN);
                snprintf(value, TEXT_LEN, "%.02lf", db->p.rh);
            } else {
                strncpy(label, "relative humidity", TEXT_LEN);
                snprintf(value, TEXT_LEN, "%.02lf", hf->rh);
            }
            file_properties_add_row(label, value, report_table_file, flags);
        }

        if (v.alpha1) {
            strncpy(label, "atmospheric trans Alpha1", TEXT_LEN);
            snprintf(value, TEXT_LEN, "%lf", hf->alpha1);
            file_properties_add_row(label, value, report_table_file, flags);
        }

        if (v.alpha2) {
            strncpy(label, "atmospheric trans Alpha2", TEXT_LEN);
            snprintf(value, TEXT_LEN, "%lf", hf->alpha2);
            file_properties_add_row(label, value, report_table_file, flags);
        }

        if (v.beta1) {
            strncpy(label, "atmospheric trans Beta1", TEXT_LEN);
            snprintf(value, TEXT_LEN, "%lf", hf->beta1);
            file_properties_add_row(label, value, report_table_file, flags);
        }

        if (v.beta2) {
            strncpy(label, "atmospheric trans Beta2", TEXT_LEN);
            snprintf(value, TEXT_LEN, "%lf", hf->beta2);
            file_properties_add_row(label, value, report_table_file, flags);
        }

        if (v.atm_trans_X) {
            strncpy(label, "atmospheric TransX", TEXT_LEN);
            snprintf(value, TEXT_LEN, "%lf", hf->atm_trans_X);
            file_properties_add_row(label, value, report_table_file, flags);
        }

        if (v.planckR1) {
            strncpy(label, "planck r1", TEXT_LEN);
            snprintf(value, TEXT_LEN, "%lf", hf->planckR1);
            file_properties_add_row(label, value, report_table_file, flags);
        }

        if (v.planckR2) {
            strncpy(label, "planck r2", TEXT_LEN);
            snprintf(value, TEXT_LEN, "%.010lf", hf->planckR2);
            file_properties_add_row(label, value, report_table_file, flags);
        }

        if (v.planckB) {
            strncpy(label, "planck b", TEXT_LEN);
            snprintf(value, TEXT_LEN, "%lf", hf->planckB);
            file_properties_add_row(label, value, report_table_file, flags);
        }

        if (v.planckF) {
            strncpy(label, "planck f", TEXT_LEN);
            snprintf(value, TEXT_LEN, "%lf", hf->planckF);
            file_properties_add_row(label, value, report_table_file, flags);
        }

        if (v.planckO) {
            strncpy(label, "planck o", TEXT_LEN);
            snprintf(value, TEXT_LEN, "%lf", hf->planckO);
            file_properties_add_row(label, value, report_table_file, flags);
        }

        if (v.atm_temp) {
            if (db->p.flags & OPT_SET_NEW_AT) {
                strncpy(label, "atmospheric temperature (*)", TEXT_LEN);
                snprintf(value, TEXT_LEN, "%.02lf", db->p.atm_temp);
            } else {
                strncpy(label, "atmospheric temperature", TEXT_LEN);
                snprintf(value, TEXT_LEN, "%.02lf", hf->atm_temp);
            }
            file_properties_add_row(label, value, report_table_file, flags);
        }

        if (v.refl_temp) {
            if (db->p.flags & OPT_SET_NEW_RT) {
                strncpy(label, "reflected apparent temperature (*)", TEXT_LEN);
                snprintf(value, TEXT_LEN, "%.02lf", db->p.refl_temp);
            } else {
                strncpy(label, "reflected apparent temperature", TEXT_LEN);
                snprintf(value, TEXT_LEN, "%.02lf", hf->refl_temp);
            }
            file_properties_add_row(label, value, report_table_file, flags);
        }

        if (v.ir_timestamp) {
            strncpy(label, "img create date", TEXT_LEN);
            strncpy(value, hf->create_ts, TEXT_LEN);
            file_properties_add_row(label, value, report_table_file, flags);
        }

        if (v.raw_th_img_res) {
            strncpy(label, "IR: resolution", TEXT_LEN);
            snprintf(value, TEXT_LEN, "%ux%u", hf->raw_th_img_width, hf->raw_th_img_height);
            file_properties_add_row(label, value, report_table_file, flags);
        }

        break;
    case TH_IRTIS_DTV:
        hi = db->in_th->head.dtv;

        if (v.raw_th_img_res) {
            strncpy(label, "IR: resolution", TEXT_LEN);
            snprintf(value, TEXT_LEN, "%ux%u", hi->nst, hi->nstv);
            file_properties_add_row(label, value, report_table_file, flags);
        }

        if (v.ir_comment) {
            strncpy(label, "IR: comment", TEXT_LEN);
            strncpy(value, hi->inform, TEXT_LEN);
            file_properties_add_row(label, value, report_table_file, flags);
        }

        if (v.ir_timestamp) {
            strncpy(label, "IR: Date Of Creation", TEXT_LEN);
            snprintf(value, TEXT_LEN, "%d/%02d/%02d %02d:%02d:%02d", t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, 
                    t.tm_hour, t.tm_min, t.tm_sec);
            file_properties_add_row(label, value, report_table_file, flags);
        }

        break;
    }    



}

void file_properties_panel(bool *p_open, th_db_t * db)
{
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

    if (ImGui::BeginTable("prop_table", 2, flags)) {
        ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableHeadersRow();


        file_properties(db, NULL, FILE_PROPERTIES_OUT_GUI);

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


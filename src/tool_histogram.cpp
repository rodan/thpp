
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include "proj.h"
#include "implot.h"
#include "tool_histogram.h"

#define MAX_HIST_BINS  100

void tool_histogram(bool *p_open, th_db_t * db)
{
    static ImPlotHistogramFlags hist_flags = ImPlotHistogramFlags_Density;
    static int bins = 50;
    static double mu = 5;
    static double sigma = 2;

    //if (!ImGui::Begin("histogram", p_open, ImGuiWindowFlags_AlwaysAutoResize)) {
    if (!ImGui::Begin("histogram", p_open, 0)) {
        ImGui::End();
        return;
    }

    if (db->in_th == NULL) {
        ImGui::Text("file not opened");
        ImGui::End();
        return;
    }

    //ImGui::SetNextItemWidth(200);
    if (ImGui::RadioButton("N Bins", bins >= 0)) {
        bins = 50;
    }
    if (bins >= 0) {
        ImGui::SameLine();
        ImGui::SetNextItemWidth(200);
        ImGui::SliderInt("##Bins", &bins, 1, MAX_HIST_BINS);
    }
    ImGui::CheckboxFlags("Density", (unsigned int *)&hist_flags, ImPlotHistogramFlags_Density);

    static double x[MAX_HIST_BINS];
    static double y[MAX_HIST_BINS];
    if (hist_flags & ImPlotHistogramFlags_Density) {
        for (int i = 0; i < MAX_HIST_BINS; ++i) {
            x[i] = -3 + 16 * (double)i / (MAX_HIST_BINS - 1);
            y[i] =
                exp(-(x[i] - mu) * (x[i] - mu) / (2 * sigma * sigma)) / (sigma *
                                                                         sqrt(2 *
                                                                              3.141592653589793238));
        }
        if (hist_flags & ImPlotHistogramFlags_Cumulative) {
            for (int i = 1; i < MAX_HIST_BINS; ++i)
                y[i] += y[i - 1];
            for (int i = 0; i < MAX_HIST_BINS; ++i)
                y[i] /= y[MAX_HIST_BINS - 1];
        }
    }

    if (ImPlot::BeginPlot("##Histograms")) {
        ImPlot::SetupAxes(NULL, NULL, ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit);
        ImPlot::SetNextFillStyle(IMPLOT_AUTO_COL, 0.5f);
        switch (db->in_th->type) {
        case TH_FLIR_RJPG:
            ImPlot::PlotHistogram("", db->temp_arr,
                                  db->in_th->head.rjpg->raw_th_img_width *
                                  db->in_th->head.rjpg->raw_th_img_height, bins, 1.0, ImPlotRange(),
                                  hist_flags);
            break;
        case TH_IRTIS_DTV:
            ImPlot::PlotHistogram("", db->temp_arr,
                                  db->in_th->head.dtv->nst * db->in_th->head.dtv->nstv, bins, 1.0,
                                  ImPlotRange(), hist_flags);
            break;
        }
        ImPlot::EndPlot();
    }
    ImGui::End();
}


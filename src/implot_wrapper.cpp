
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include "proj.h"
#include "implot.h"
#include "implot_wrapper.h"

#define MAX_HIST_BINS  100

double *xdata = NULL;
double *ydata = NULL;

void line_plot_calc(th_db_t * db, linedef_t * line, double *data, const uint16_t data_len)
{
    double x1, y1, x2, y2;
    double slope, offset;
    double qx;
    double tmp;
    uint16_t xtemp, ytemp;
    double i;
    uint16_t points_remaining = data_len;
    uint16_t points_c = 0;

    x1 = line->x1;
    y1 = line->y1;
    x2 = line->x2;
    y2 = line->y2;

    qx = (x2 - x1) / data_len;
    slope = (y2 - y1)/(x2 - x1);
    offset = y2 - (x2 * slope);
 
    i = x1;

    while (points_remaining) {
        tmp = slope * i + offset;
        ytemp = tmp;
        xtemp = i;
        switch (db->in_th->type) {
            case TH_FLIR_RJPG:
                data[points_c] = db->temp_arr[db->in_th->head.rjpg->raw_th_img_width * ytemp + xtemp];
                break;
            case TH_IRTIS_DTV:
                data[points_c] = db->temp_arr[db->in_th->head.dtv->nst * ytemp + xtemp];
                break;
        }
        i+=qx;
        points_remaining--;
        points_c++;
    }
}

void line_plot(th_db_t * db, linedef_t *line)
{
    uint16_t i;
    static uint16_t data_len = 0;
    double len;
    double x1 = line->x1;
    double y1 = line->y1;
    double x2 = line->x2;
    double y2 = line->y2;
    static ImVec4 color = ImVec4(1,1,0,1);
    double ymin = 0.0;
    double ymax = 0.0;

    if (line->do_refresh) {

        if (xdata != NULL) {
            free(xdata);
        }

        if (ydata != NULL) {
            free(ydata);
        }

        if (x1 == x2) {
            if (x1 == 0) {
                x1 = 1;
            } else {
                x1--;
            }
        }
        if (y1 == y2) {
            if (y1 == 0) {
                y1 = 1;
            } else {
                y1--;
            }
        }

        // distance in pixels between the two mouse pointers
        len = sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
        if (len == 0) {
            return;
        }
        data_len = len;

        xdata = (double *) calloc(data_len, sizeof(double));
        ydata = (double *) calloc(data_len, sizeof(double));

        for (i = 0; i < data_len; i++) {
            xdata[i] = i;
        }

        line_plot_calc(db, line, ydata, data_len);
    }

    if (ImPlot::BeginPlot("current line transient")) {
        ImPlot::SetupAxes("x", "y");
        ImPlot::SetNextLineStyle(color, 2);
        switch (db->in_th->type) {
        case TH_FLIR_RJPG:
            ymin = db->out_th->head.rjpg->t_min - 5.0;
            ymax = db->out_th->head.rjpg->t_max + 5.0;
            //ImPlot::SetupAxesLimits(0, 10, ymin, ymax, ImPlotCond_Always);
            ImPlot::SetupAxesLimits(0, data_len, ymin, ymax, ImPlotCond_Always);
            ImPlot::SetupAxes("pixel","temp [C]",ImPlotAxisFlags_AutoFit | ImPlotAxisFlags_NoDecorations, ImPlotAxisFlags_AutoFit);
            ImPlot::PlotLine("", xdata, ydata, data_len);
            break;
        case TH_IRTIS_DTV:
            ymin = db->in_th->head.dtv->tsc[1];
            ymax = db->in_th->head.dtv->tsc[1] + db->in_th->head.dtv->tsc[0] * 256.0;
            ImPlot::SetupAxesLimits(0, data_len, ymin, ymax, ImPlotCond_Always);
            ImPlot::SetupAxes("pixel","temp [C]",ImPlotAxisFlags_AutoFit | ImPlotAxisFlags_NoDecorations, ImPlotAxisFlags_AutoFit);
            ImPlot::PlotLine("", xdata, ydata, data_len);
            break;
        }
        ImPlot::EndPlot();
    }
}

void line_plot_free(void)
{
    if (xdata != NULL) {
        free(xdata);
        xdata = NULL;
    }

    if (ydata != NULL) {
        free(ydata);
        ydata = NULL;
    }
}

void histogram(th_db_t * db)
{
    static ImPlotHistogramFlags hist_flags = ImPlotHistogramFlags_Density;
    static int bins = 50;
    static double mu = 5;
    static double sigma = 2;
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
}

void implot_wrapper(th_db_t * db, linedef_t * ld)
{

    ImGui::Begin("plots window");
    ImGui::BeginTabItem("plots");

    if (ld->active) {
        if (ImGui::TreeNodeEx("line plot")) {
            line_plot(db, ld);
            ImGui::Text("%d %d -> %d %d", ld->x1, ld->y1, ld->x2, ld->y2);
            ImGui::TreePop();
        }
    }

    if (ImGui::TreeNodeEx("histogram")) {
        histogram(db);
        ImGui::TreePop();
    }

    ImGui::EndTabItem();
    ImGui::End();
}


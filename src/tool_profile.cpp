
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include "tlpi_hdr.h"
#include "proj.h"
#include "implot.h"
#include "tool_profile.h"

static double *xdata = NULL;
static double *ydata = NULL;

void line_plot_calc(th_db_t * db, double *data, const uint16_t data_len)
{
    double x1, y1, x2, y2;
    double slope, offset;
    double qx;
    double tmp;
    uint16_t xtemp, ytemp;
    double i;
    uint16_t points_remaining = data_len;
    uint16_t points_c = 0;

    x1 = db->pr.x1;
    y1 = db->pr.y1;
    x2 = db->pr.x2;
    y2 = db->pr.y2;

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

void line_plot(th_db_t * db)
{
    uint16_t i;
    static uint16_t data_len = 0;
    double len;
    double x1 = db->pr.x1;
    double y1 = db->pr.y1;
    double x2 = db->pr.x2;
    double y2 = db->pr.y2;
    static ImVec4 color = ImVec4(1,1,0,1);
    double ymin = 0.0;
    double ymax = 0.0;

    if (db->pr.do_refresh || !xdata || !ydata) {

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
        if (xdata == NULL) {
            errMsg("allocating buffer");
            return;
        }
        ydata = (double *) calloc(data_len, sizeof(double));
        if (ydata == NULL) {
            errMsg("allocating buffer");
            return;
        }

        for (i = 0; i < data_len; i++) {
            xdata[i] = i;
        }

        line_plot_calc(db, ydata, data_len);
    }

    if ((xdata == NULL) || (ydata == NULL)) {
        printf("saved %s, %p %p %d %d\n", db->p.in_file, (void *) xdata, (void *) ydata, data_len, db->pr.active);
        return;
    }

    if (ImPlot::BeginPlot("profile")) {
        ImPlot::SetupAxes("x", "y");
        ImPlot::SetNextLineStyle(color, 2);

        get_min_max(db->out_th, &ymin, &ymax);
        ymin -= 5.0;
        ymax += 5.0;
        ImPlot::SetupAxesLimits(0, data_len, ymin, ymax, ImPlotCond_Always);
        ImPlot::SetupAxes("pixel","temp [C]",ImPlotAxisFlags_AutoFit | ImPlotAxisFlags_NoDecorations, ImPlotAxisFlags_AutoFit);
        ImPlot::PlotLine("", xdata, ydata, data_len);

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

void tool_profile(bool *p_open, th_db_t * db)
{

    if (!ImGui::Begin("profile", p_open, 0)) {
        ImGui::End();
        return;
    }

    if ((db->in_th == NULL) || (db->out_th == NULL) || (!db->pr.active))  {
        ImGui::Text("profile not active or file not opened");
        ImGui::End();
        return;
    }

    if (db->pr.active && (db->fe.return_state == RET_OK)) {
        ImGui::SetNextItemOpen(1, 0);
        if (ImGui::TreeNodeEx("line plot")) {
            line_plot(db);
            ImGui::Text("%d %d -> %d %d", db->pr.x1, db->pr.y1, db->pr.x2, db->pr.y2);
            ImGui::TreePop();
        }
    }

    ImGui::End();
}


//----------------------------------------------------------------------------
// filter header for ThPP ver 1.x
// author: Petre Rodan <rodan@subdimension.com>
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
#ifndef analysisH
#define analysisH

//----------------------------------------------------------------------------

struct csom_segmented
{
public:
 char show_layer[256];

};

void get_hist(); //histogram
void get_histcum();

int csom_segmentation(char *fn, char *nfn, int window_size, int output_neurons, int max_epoch, int neigh_neurons, float min_evolution, float max_learn_rate, int options, bool *show_layer);

#endif

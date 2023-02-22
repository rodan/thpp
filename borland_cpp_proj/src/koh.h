//---------------------------------------------------------------------------
// Kohonen Circular Self Organizing Map Neural Network header for ThPP ver 1.x
// author: Petre Rodan <rodan@subdimension.com>
//---------------------------------------------------------------------------

#ifndef kohH
#define kohH

#include <math.h>

struct csom
{
public:
 char   id;            //id of this map (for threading)
 int    flags;          //csom is used
 int    input_neurons;  //number of input level neurons
 int    output_neurons; //number of output level neurons
 int    element;       //number of elements applied
 int    element_current; //current vector
 int    max_neigh;      //how many neurons share the winning (on each side of the winner)
 int    neigh_type;     //0- step, 1- triangular, 2- gaussian // not used, default is triangular
 int    epoch;          //current epoch
 int    max_epoch;      //how many itterations to perform
 bool   stop_on_max_epoch; //use "max_epoch" to stop?
 float  evolution;      //the changes of coefficients between 2 epochs
 float  min_evolution;  //minimal evolution when to stop
 bool   stop_on_evolution; //use "evolution" to stop?
 bool   random_init;   //use random init or not
 float *x0;          //first element for input vector
 float *x1;          //second element for input vector
 float *w0;          //coefficients for first element of input vector
 float *w1;          //coefficients for second element of input vector
 bool   use_0;         //use x0 and w0
 bool   use_1;         //use x1 and w1
 bool   wta;           //winner takes all
 float  max_learn_rate;  //initial value of learning rate
 float *learn_rate;  //function of learning rate
 float *neigh_rate;  //function of neighborhood
 unsigned char  *colormap;    //gray level for output pixels
 int   *csom_out;    //output vector of csom
 int   *activations; //how many times was a neuron activated
};

int csom_main(struct csom &csom_x);
int csom_init(struct csom &csom_x,int output_neurons, int elements, int max_epoch, int neigh_neurons, float evolution, float max_learn_rate, int options, bool *show_layer);
int csom_destruct(struct csom &csom_x);

#endif

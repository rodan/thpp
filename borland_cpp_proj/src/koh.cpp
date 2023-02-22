//---------------------------------------------------------------------------
// Kohonen Circular Self Organizing Map Neural Network header for ThPP ver 1.x
// author: Petre Rodan <rodan@subdimension.com>
//---------------------------------------------------------------------------

#include <system.hpp>

#include "koh.h"
#include "general.h"
#include "processing.h"

//---------------------------------------------------------------------------

int csom_init(struct csom &csom_x, int output_neurons, int elements, int max_epoch, int neigh_neurons, float min_evolution, float max_learn_rate, int options, bool *show_layer)
{
int i;
bool use_evolution=0,use_max_epoch=0,use_0=0,use_1=0,use_random=0,wta=0;
//int input_neurons;
//input_neurons=2;

if (options>31) {wta=1;options-=32;}
if (options>15) {use_random=1;options-=16;}
if (options>7) {use_1=1;options-=8;}
if (options>3) {use_0=1;options-=4;}
if (options>1) {use_evolution=1;options-=2;}
if (options>0) {use_max_epoch=1;}

csom_x.flags=1;
csom_x.id=1;
csom_x.input_neurons=(int)use_0+(int)use_1;
csom_x.output_neurons=output_neurons;
csom_x.element=elements;
csom_x.element_current=0;
csom_x.epoch=0;
csom_x.max_epoch=max_epoch;
csom_x.evolution=0;
csom_x.min_evolution=min_evolution;
csom_x.stop_on_max_epoch=use_max_epoch;
csom_x.stop_on_evolution=use_evolution;
csom_x.max_neigh=neigh_neurons;
//csom_x.neigh_type=neigh_type;
csom_x.max_learn_rate=max_learn_rate,
csom_x.use_0=use_0;
csom_x.use_1=use_1;
csom_x.random_init=use_random;
csom_x.wta=wta;

if (csom_x.use_0)
       {csom_x.x0=(float *)malloc((csom_x.element)*sizeof(float));
        csom_x.w0=(float *)malloc((csom_x.output_neurons)*sizeof(float));
       }

if (csom_x.use_1)
       {csom_x.x1=(float *)malloc((csom_x.element)*sizeof(float));
        csom_x.w1=(float *)malloc((csom_x.output_neurons)*sizeof(float));
       }

csom_x.colormap=(unsigned char *)malloc((csom_x.output_neurons)*sizeof(unsigned char));
csom_x.activations=(int *)malloc((csom_x.output_neurons)*sizeof(int));
csom_x.neigh_rate=(float *)malloc((csom_x.max_epoch)*sizeof(float));
csom_x.learn_rate=(float *)malloc((csom_x.max_epoch)*sizeof(float));
csom_x.csom_out=(int *)malloc((csom_x.element)*sizeof(int));

for (i=0;i<output_neurons;i++) {csom_x.colormap[i]=(int)show_layer[i]*floor(i*254/(output_neurons-1));}
for (i=0;i<csom_x.max_epoch;i++) { csom_x.neigh_rate[i]=ceil( (float)-(csom_x.max_neigh*1.00000001/csom_x.max_epoch)*i*1.00000001+ csom_x.max_neigh);}
for (i=0;i<csom_x.max_epoch;i++) { csom_x.learn_rate[i]=csom_x.max_learn_rate*(pow((0.01/csom_x.max_learn_rate),((float)i/csom_x.max_epoch)));}
for (i=0;i<output_neurons;i++) {csom_x.activations[i]=0;}

csom_x.neigh_rate[csom_x.max_epoch-1]=1;
randomize();
for (i=0;i<output_neurons;i++)
      { //if (csom_x.use_0) {csom_x.w0[i]=i*255.00/csom_x.output_neurons;}
        if (csom_x.random_init)
           {if (csom_x.use_0) {csom_x.w0[i]=random(255);}
            if (csom_x.use_1) {csom_x.w1[i]=random(20);}
           }
        if (! csom_x.random_init)
           {if (csom_x.use_0) {csom_x.w0[i]=(float)i*254.00/csom_x.output_neurons;}
            if (csom_x.use_1) {csom_x.w1[i]=(float)(i*40.00/csom_x.output_neurons)-20;}
           }
      }
return 0;
}

//---------------------------------------------------------------------------

int csom_main(struct csom &csom_x)
{
int j;
float *d,d_min;       //distances
int winner,winner_df; //distance from winner
float *w0_new, *w1_new; //new coefficients
bool keep_training=1;
int vector_no;
int st,dr;

d=(float *)malloc((csom_x.output_neurons)*sizeof(float));
w0_new=(float *)malloc((csom_x.output_neurons)*sizeof(float));
w1_new=(float *)malloc((csom_x.output_neurons)*sizeof(float));

csom_x.epoch=0;

if (csom_x.wta) {csom_x.max_epoch=1;csom_x.stop_on_max_epoch=1;}

while (keep_training)
   {
    //pas 1. initializations
    //pas 2. se aplica x(t)
    //pas 3. se calculeaza distanta dj de la intrare la vectorii pondere
    //pas 4. se determina neuronul cistigator
    //pas 5. se rafineaza ponderile pt neuronul cistigator si ale vecinilor
    //pas 6. se trece la urmatorul vector din lot.
    csom_x.evolution=0;
    for (j=0;j<csom_x.output_neurons;j++) {csom_x.activations[j]=0;}

    for (vector_no=0; vector_no<csom_x.element; vector_no++)
        {d_min=99999999.99;
         csom_x.element_current=vector_no;
         //euclidian distances from input vector to the coefficients
         for (j=0;j<csom_x.output_neurons;j++)
             {d[j]=0;
              if (csom_x.use_0)
                 {d[j]+=(csom_x.x0[vector_no] - csom_x.w0[j])*(csom_x.x0[vector_no]-csom_x.w0[j]);}
              if (csom_x.use_1)
                 {d[j]+=(csom_x.x1[vector_no] - csom_x.w1[j])*(csom_x.x1[vector_no]-csom_x.w1[j]);}
              //who is the winner for this vector?
              if (d[j]<d_min) {d_min=d[j]; winner=j;}
             }
         //share the winning
         csom_x.csom_out[csom_x.element_current]=winner;
         csom_x.activations[winner]++;

         winner_df=0;
         st=winner;
         dr=winner;

         while (winner_df<csom_x.neigh_rate[csom_x.epoch])
             {
                if (csom_x.use_0)
                  {w0_new[dr]=csom_x.w0[dr]+csom_x.learn_rate[csom_x.epoch]*(1-(winner_df*1.000000001/csom_x.neigh_rate[csom_x.epoch]))*(csom_x.x0[vector_no]-csom_x.w0[dr]);
                   csom_x.evolution+=pow(w0_new[dr]-csom_x.w0[dr],2);
                   csom_x.w0[dr]=w0_new[dr];

                   w0_new[st]=csom_x.w0[st]+csom_x.learn_rate[csom_x.epoch]*(1-(winner_df*1.000000001/csom_x.neigh_rate[csom_x.epoch]))*(csom_x.x0[vector_no]-csom_x.w0[st]);
                   csom_x.evolution+=pow(w0_new[st]-csom_x.w0[st],2);
                   csom_x.w0[st]=w0_new[st];
                  }
                if (csom_x.use_1)
                  {w1_new[dr]=csom_x.w1[dr]+csom_x.learn_rate[csom_x.epoch]*(1-(winner_df*1.000000001/csom_x.neigh_rate[csom_x.epoch]))*(csom_x.x1[vector_no]-csom_x.w1[dr]);
                   csom_x.evolution+=pow(w1_new[dr]-csom_x.w1[dr],2);
                   csom_x.w1[dr]=w1_new[dr];

                   w1_new[st]=csom_x.w1[st]+csom_x.learn_rate[csom_x.epoch]*(1-(winner_df*1.000000001/csom_x.neigh_rate[csom_x.epoch]))*(csom_x.x1[vector_no]-csom_x.w1[st]);
                   csom_x.evolution+=pow(w1_new[st]-csom_x.w1[st],2);
                   csom_x.w1[st]=w1_new[st];
                  }

                if (st==0) {st=csom_x.output_neurons;}
                if (dr==csom_x.output_neurons-1) {dr=-1;}

                st--; dr++; winner_df++;
             }
        } //all vectors done

    csom_x.epoch++;
    if (csom_x.epoch>=csom_x.max_epoch) {keep_training=0;}
   } //epoch

free(d);
free(w0_new);
free(w1_new);
return 0;
}

//---------------------------------------------------------------------------

int csom_destruct(struct csom &csom_x)
{

if (csom_x.use_0)
  {free(csom_x.x0);
   free(csom_x.w0);
   csom_x.x0=NULL;
   csom_x.w0=NULL;
  }
if (csom_x.use_1)
  {free(csom_x.x1);
   free(csom_x.w1);
   csom_x.x1=NULL;
   csom_x.w1=NULL;
  }
free(csom_x.colormap);
free(csom_x.learn_rate);
free(csom_x.neigh_rate);
free(csom_x.csom_out);
free(csom_x.activations);

csom_x.colormap=NULL;
csom_x.learn_rate=NULL;
csom_x.neigh_rate=NULL;
csom_x.csom_out=NULL;
csom_x.activations=NULL;

csom_x.flags=0;
csom_x.id=0;
csom_x.input_neurons=0;
csom_x.output_neurons=0;
csom_x.element=0;
csom_x.element_current=0;
csom_x.epoch=0;
csom_x.max_epoch=0;
csom_x.evolution=0;
csom_x.min_evolution=0;
csom_x.stop_on_max_epoch=0;
csom_x.stop_on_evolution=0;
csom_x.max_neigh=0;
csom_x.use_0=0;
csom_x.use_1=0;
csom_x.neigh_type=0;
csom_x.wta=0;

return 0;
}

//---------------------------------------------------------------------------


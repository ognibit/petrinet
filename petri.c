/*
 * Petri Net implementation.
 *
 * Author: Omar Rampado <omar@ognibit.it>
 *
 * TODO: index on transitions (list of in, out)
 * Version: 1.0
 * Simple implementation, array based, no optimization, ok for small nets.
 */

#include "petri.h"
#include <stdbool.h>
#include <stdlib.h>

typedef enum {
    ARC_IN,
    ARC_OUT
} pn_arcdir;

struct PetriArc {
    pn_arcdir dir; /* direction */
    pn_place place;
    pn_trans trans;
    pn_weight weight;
};

typedef struct PetriArc PetriArc;

struct PetriNet {
    size_t nplaces;
    size_t ntrans;
    size_t arcsLen;
    size_t arcsSize;
    pn_weight *marking; /* size = nplaces */
    PetriArc *arcs;
};

/******************** PRIVATE ***********************************/

/* Get the arc that match (D,T,P).
 * If it does not exists, create it when alloc is true.
 * return NULL if reallocation of memory fails or not found.
 */
static
PetriArc * petri_arc(PetriNet *net, pn_arcdir d, pn_trans t, pn_place p, bool alloc)
{
    PetriArc *arc = NULL;
    for (size_t i=0; i < net->arcsLen; i++){
        arc = &net->arcs[i];
        if ((arc->dir == d) && (arc->place == p) && (arc->trans == t)){
            return arc;
        }
    }

    /* not found */
    if (!alloc){
        return NULL;
    }

    if (net->arcsLen >= net->arcsSize){

        size_t nsize = net->arcsSize * 2;
        PetriArc *arcs = (PetriArc*)reallocarray(net->arcs, nsize, sizeof(PetriArc));
        if (arcs == NULL){
            return NULL;
        }

        net->arcsSize = nsize;
        net->arcs = arcs;
    }

    /* return a new arc */
    arc = &net->arcs[net->arcsLen];
    arc->dir = d;
    arc->place = p;
    arc->trans = t;
    arc->weight = 0;

    net->arcsLen += 1;

    return arc;
}/* petri_arc */

/******************** PUBLIC ***********************************/

PetriNet * petri_new(size_t nplaces, size_t ntrans)
{
    const size_t n = nplaces + ntrans;

    PetriArc *arcs = (PetriArc*)calloc(n, sizeof(PetriArc));
    if (arcs == NULL){
        return NULL;
    }

    pn_weight *marking = (pn_weight*)calloc(nplaces, sizeof(pn_weight));
    if (marking == NULL){
        free(arcs);
        return NULL;
    }

    PetriNet *net = (PetriNet*)malloc(sizeof(PetriNet));
    if (net == NULL){
        free(marking);
        free(arcs);
        return NULL;
    }

    net->nplaces = nplaces;
    net->ntrans = ntrans;
    net->arcsLen = 0;
    net->arcsSize = n;
    net->marking = marking;
    net->arcs = arcs;

    return net;
}/* petri_new */

void petri_free(PetriNet *net)
{
    if (net == NULL){
        return;
    }

    free(net->arcs);
    free(net);
}/* petri_free */

size_t petri_nplaces(PetriNet *net)
{
    if (net == NULL){
        return 0;
    }
    return net->nplaces;
}/* petri_nplaces */

size_t petri_ntrans(PetriNet *net)
{
    if (net == NULL){
        return 0;
    }
    return net->ntrans;
}/* petri_ntrans */

bool petri_conf_input(PetriNet *net, pn_place p, pn_trans t, pn_weight w)
{
    if (net == NULL){
        return false;
    }

    if (p >= net->nplaces || t >= net->ntrans){
        return false;
    }

    //TODO when w = 0 it waste space
    PetriArc *arc = petri_arc(net, ARC_IN, t, p, true);
    if (arc == NULL){
        return false;
    }
    arc->weight = w;

    return true;
}/* petri_conf_input */

bool petri_conf_output(PetriNet *net, pn_trans t, pn_place p, pn_weight w)
{
    //TODO merge with petri_cont_input
    if (net == NULL){
        return false;
    }

    if (p >= net->nplaces || t >= net->ntrans){
        return false;
    }

    //TODO when w = 0 it waste space
    PetriArc *arc = petri_arc(net, ARC_OUT, t, p, true);
    if (arc == NULL){
        return false;
    }
    arc->weight = w;

    return true;
}/* petri_conf_output */

void petri_marking_get(PetriNet *net, pn_weight *outmark)
{
    if (net == NULL || outmark == NULL){
        return;
    }

    //TODO memmove
    for (pn_place i=0; i < net->nplaces; i++){
        outmark[i] = net->marking[i];
    }
}/* petri_marking_get */

void petri_marking_set(PetriNet *net, const pn_weight *inmark)
{
    if (net == NULL || inmark == NULL){
        return;
    }

    //TODO memmove
    for (pn_place i=0; i < net->nplaces; i++){
        net->marking[i] = inmark[i];
    }
}/* petri_marking_set */

/* M(p) >= I(t,p) for all p */
bool petri_trans_enabled(PetriNet *net, pn_trans t)
{
    if (net == NULL || t >= net->ntrans){
        return false;
    }

    //TODO optimize with index on T

    for (pn_place p=0; p < net->nplaces; p++){
        pn_weight m = net->marking[p];
        pn_weight inputs = petri_weight_in(net, p, t);

        if (m < inputs){
            return false;
        }
    }

    return true;
}/* petri_trans_enabled */

/* t is enabled
 * M'(p) = M(p) - I(t,p) + O(t,p) for all p
 */
bool petri_fire(PetriNet *net, pn_trans t)
{
    /* control ALL the places before start modifing them */
    if (!petri_trans_enabled(net, t)){
        return false;
    }

    //TODO optimize with index on T

    for (pn_place p=0; p < net->nplaces; p++){
        pn_weight m = net->marking[p];
        pn_weight inputs = petri_weight_in(net, p, t);
        pn_weight outputs = petri_weight_out(net, t, p);

        net->marking[p] = m - inputs + outputs;
    }
    return true;
}/* petri_fire */

/* Get the marking of place P.
 * p: the place
 *
 * return the number of tokens in P. Zero is p out of range.
 */
pn_weight petri_weight_of(PetriNet *net, pn_place p)
{
    if (net == NULL || p >= net->nplaces){
        return 0;
    }

    return net->marking[p];
}/* petri_weight_of */

pn_weight petri_weight_in(PetriNet *net, pn_place p, pn_trans t)
{
    PetriArc *arc = petri_arc(net, ARC_IN, t, p, false);
    if (arc == NULL){
        return 0;
    }

    return arc->weight;
}/* petri_weight_in */

pn_weight petri_weight_out(PetriNet *net, pn_trans t, pn_place p)
{
    PetriArc *arc = petri_arc(net, ARC_OUT, t, p, false);
    if (arc == NULL){
        return 0;
    }

    return arc->weight;
}/* petri_weight_out */


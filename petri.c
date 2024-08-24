/*
 * Petri Net implementation.
 *
 * Author: Omar Rampado <omar@ognibit.it>
 *
 * Version: 1.0
 * Implementation with transition index and arcs as linked list.
 */

#include "petri.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
    ARC_IN,
    ARC_OUT
} pn_arcdir;

typedef struct PetriArc PetriArc;

struct PetriArc {
    pn_arcdir dir; /* direction */
    pn_place place;
    pn_trans trans;
    pn_weight weight;
    PetriArc *next;
};

struct PetriNet {
    size_t nplaces;
    size_t ntrans;
    pn_weight *marking; /* size = nplaces */
    PetriArc **arcs;    /* size = ntrans */
};

/******************** PRIVATE ***********************************/

static
PetriArc * petri_arc_find(const PetriNet *net, pn_arcdir d, pn_trans t, pn_place p)
{
    PetriArc *arc = net->arcs[t];

    while (arc != NULL){
        if ((arc->dir == d) && (arc->place == p) && (arc->trans == t)){
            return arc;
        }
        arc = arc->next;
    }/* while */

    return NULL;
}/* petri_arc_find */

/* Get the arc that match (D,T,P).
 * If it does not exists, create it when alloc is true.
 * return NULL if reallocation of memory fails or not found.
 */
static
PetriArc * petri_arc(PetriNet *net, pn_arcdir d, pn_trans t, pn_place p, bool alloc)
{
    PetriArc *arc = petri_arc_find(net, d, t, p);
    if (arc != NULL){
        return arc;
    }

    /* not found */
    if (!alloc){
        return NULL;
    }

    /* allocate a new arc */
    arc = (PetriArc*)malloc(sizeof(PetriArc));
    if (arc == NULL){
        return NULL;
    }

    arc->next = net->arcs[t];
    net->arcs[t] = arc;

    /* return the new arc */
    arc->dir = d;
    arc->place = p;
    arc->trans = t;
    arc->weight = 0;

    return arc;
}/* petri_arc */

static
bool petri_conf_arc(PetriNet *net, pn_arcdir d, pn_place p, pn_trans t, pn_weight w)
{
    if (net == NULL){
        return false;
    }

    if (p >= net->nplaces || t >= net->ntrans){
        return false;
    }

    /* setting a weight at 0 is the same to do not have the arc.
     * If the arc already exists, it will be overwritten.
     * If the arc does not exists, do no create it.
     */
    bool alloc = (w > 0);
    PetriArc *arc = petri_arc(net, d, t, p, alloc);
    if (arc == NULL && alloc){
        /* it is a problem only when the allocation fails */
        return false;
    }

    if (arc != NULL){
        /* arc could be NULL if not found and alloc=false */
        arc->weight = w;
    }

    return true;
}/* petri_conf_arc */

/******************** PUBLIC ***********************************/

PetriNet * petri_new(size_t nplaces, size_t ntrans)
{
    PetriArc **arcs = (PetriArc**)calloc(ntrans, sizeof(PetriArc*));
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
    net->marking = marking;
    net->arcs = arcs;

    return net;
}/* petri_new */

void petri_free(PetriNet *net)
{
    if (net == NULL){
        return;
    }

    /* free arc lists */
    for (pn_trans i=0; i < net->ntrans; i++){
        PetriArc *curr = net->arcs[i];
        PetriArc *next = NULL;

        while (curr != NULL){
            next = curr->next;
            free(curr);
            curr = next;
        }/* while */

        net->arcs[i] = NULL;
    }/* for */

    free(net->arcs);
    free(net->marking);
    free(net);
}/* petri_free */

size_t petri_nplaces(const PetriNet *net)
{
    if (net == NULL){
        return 0;
    }
    return net->nplaces;
}/* petri_nplaces */

size_t petri_ntrans(const PetriNet *net)
{
    if (net == NULL){
        return 0;
    }
    return net->ntrans;
}/* petri_ntrans */

bool petri_conf_input(PetriNet *net, pn_place p, pn_trans t, pn_weight w)
{
    return petri_conf_arc(net, ARC_IN, p, t, w);
}/* petri_conf_input */

bool petri_conf_output(PetriNet *net, pn_trans t, pn_place p, pn_weight w)
{
    return petri_conf_arc(net, ARC_OUT, p, t, w);
}/* petri_conf_output */

void petri_marking_get(const PetriNet *net, pn_weight *outmark)
{
    if (net == NULL || outmark == NULL){
        return;
    }

    memmove(outmark, net->marking, net->nplaces * sizeof(pn_weight));
}/* petri_marking_get */

void petri_marking_set(PetriNet *net, const pn_weight *inmark)
{
    if (net == NULL || inmark == NULL){
        return;
    }

    memmove(net->marking, inmark, net->nplaces * sizeof(pn_weight));
}/* petri_marking_set */

/* M(p) >= I(t,p) for all p */
bool petri_trans_enabled(const PetriNet *net, pn_trans t)
{
    if (net == NULL || t >= net->ntrans){
        return false;
    }

    /* since only the places connected to T are involved */
    PetriArc *arc = net->arcs[t];
    while (arc != NULL){
        pn_place p = arc->place;
        pn_weight m = net->marking[p];

        if ((arc->dir == ARC_IN) && (m < arc->weight)){
            return false;
        }

        arc = arc->next;
    }/* while */

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

    /* since only the places connected to T are involved */
    PetriArc *arc = net->arcs[t];
    while (arc != NULL){
        pn_place p = arc->place;
        pn_weight w = arc->weight;

        switch (arc->dir){
        case ARC_IN:
            net->marking[p] -= w;
            break;
        case ARC_OUT:
            net->marking[p] += w;
            break;
        default:
            return false;
        }/* switch */

        arc = arc->next;
    }/* while */

    return true;
}/* petri_fire */

/* Get the marking of place P.
 * p: the place
 *
 * return the number of tokens in P. Zero is p out of range.
 */
pn_weight petri_weight_of(const PetriNet *net, pn_place p)
{
    if (net == NULL || p >= net->nplaces){
        return 0;
    }

    return net->marking[p];
}/* petri_weight_of */

pn_weight petri_weight_in(const PetriNet *net, pn_place p, pn_trans t)
{
    const PetriArc *arc = petri_arc_find(net, ARC_IN, t, p);
    if (arc == NULL){
        return 0;
    }

    return arc->weight;
}/* petri_weight_in */

pn_weight petri_weight_out(const PetriNet *net, pn_trans t, pn_place p)
{
    const PetriArc *arc = petri_arc_find(net, ARC_OUT, t, p);
    if (arc == NULL){
        return 0;
    }

    return arc->weight;
}/* petri_weight_out */


#pragma once

/*
 * Petri Net.
 *
 * Author: Omar Rampado <omar@ognibit.it>
 * Version: 1.0
 */

#include <stdbool.h>
#include <stddef.h>

typedef struct PetriNet PetriNet;
typedef size_t pn_place;
typedef size_t pn_trans;
typedef unsigned int pn_weight;

/* Allocate the network without arcs and no marking.
 * Use 'petri_marking_set' to set the initial marking.
 * Use 'petri_conf_input' and 'petri_conf_output' to set the arcs.
 * nplaces: number of places
 * ntrans: number of trans
 *
 * return The network or NULL if cannot allocate it
 */
PetriNet * petri_new(size_t nplaces, size_t ntrans);

/* Deallocate the network memory */
void petri_free(PetriNet *net);

/* Get the number of places in the net */
size_t petri_nplaces(PetriNet *net);

/* Get the number of transitions in the net */
size_t petri_ntrans(PetriNet *net);

/* Assign the weight W to the input arc from place P to transition T.
 * p: input place
 * t: transition
 * w: weight. If zero, it remove the arc.
 *
 * return True if succeed, False if P or T out of range
 */
bool petri_conf_input(PetriNet *net, pn_place p, pn_trans t, pn_weight w);

/* Assign the weight W to the output arc from transition T to place P.
 * t: transition
 * p: output place
 * w: weight. If zero, it remove the arc.
 *
 * return True if succeed, False if P or T out of range
 */
bool petri_conf_output(PetriNet *net, pn_trans t, pn_place p, pn_weight w);

/* Copy the current marking into OUTMARK, place as index, tokens as value.
 * outmark: must be at least 'petri_nplaces' long
 */
void petri_marking_get(PetriNet *net, pn_weight *outmark);

/* Set the current marking in the NET as INMARK, place as index, tokens as
 * value.
 * inmark: must be at least 'petri_nplaces' long
 */
void petri_marking_set(PetriNet *net, const pn_weight *inmark);

/* Check if the transition is enabled in the current marking.
 * t: the transition to check.
 *
 * return True if it is enabled, False if it is not or out of range.
 */
bool petri_trans_enabled(PetriNet *net, pn_trans t);

/* Fire the transition T in the net.
 * It checks if the transition is enabled before firing.
 * t: the transition to fire.
 *
 * return True if fired, False if t was not enabled.
 */
bool petri_fire(PetriNet *net, pn_trans t);

/* Get the marking of place P.
 * p: the place
 *
 * return the number of tokens in P. Zero is p out of range.
 */
pn_weight petri_weight_of(PetriNet *net, pn_place p);

/* Get the weight of the input arc from P to T.
 * p: the input place
 * t: the transition
 *
 * return the configured weight of the input arc
 */
pn_weight petri_weight_in(PetriNet *net, pn_place p, pn_trans t);

/* Get the weight of the output arc from T to P.
 * t: the transition
 * p: the output place
 *
 * return the configured weight of the output arc
 */
pn_weight petri_weight_out(PetriNet *net, pn_trans t, pn_place p);

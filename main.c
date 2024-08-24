/* Petri Net tests */

#include <stdio.h>
#include <assert.h>
#include "petri.h"

static void test_new()
{
    printf("test_new...");

    PetriNet *net;
    /* empty net */
    net = petri_new(0, 0);
    assert(net != NULL);
    assert(petri_nplaces(net) == 0);
    assert(petri_ntrans(net) == 0);
    assert(petri_conf_input(net, 0, 0, 1) == false);
    assert(petri_conf_output(net, 0, 0, 1) == false);
    petri_free(net);

    net = petri_new(3, 2);
    assert(net != NULL);
    assert(petri_nplaces(net) == 3);
    assert(petri_ntrans(net) == 2);
    assert(petri_conf_input(net, 0, 0, 1) == true);
    assert(petri_conf_output(net, 0, 0, 1) == true);
    assert(petri_conf_input(net, 3, 0, 1) == false);
    assert(petri_conf_output(net, 0, 2, 1) == true);
    petri_free(net);

    /* memory stress */
    net = petri_new(10, 10);
    assert(net != NULL);
    for (pn_place p=0; p < 10; p++){
        for (pn_trans t=0; t < 10; t++){
            assert(petri_conf_input(net, p, t, p));
            assert(petri_conf_output(net, t, p, t));
            assert(petri_weight_in(net, p, t) == p);
            assert(petri_weight_out(net, t, p) == t);
        }
    }
    petri_free(net);

    puts("OK");
}/* test_new */

static void test_weights()
{
    printf("test_weights...");

    PetriNet *net;
    net = petri_new(5, 3);

    petri_conf_input(net, 1, 1, 2);
    petri_conf_input(net, 4, 1, 1);
    petri_conf_input(net, 2, 2, 2);

    petri_conf_output(net, 1, 2, 2);
    petri_conf_output(net, 2, 4, 1);
    petri_conf_output(net, 2, 3, 2);

    assert(petri_weight_in(net, 1, 1) == 2);
    assert(petri_weight_in(net, 1, 1) == 2);
    assert(petri_weight_in(net, 4, 1) == 1);

    assert(petri_weight_out(net, 1, 2) == 2);
    assert(petri_weight_out(net, 2, 4) == 1);
    assert(petri_weight_out(net, 2, 3) == 2);

    petri_free(net);
    puts("OK");
}/* test_weights */

static void test_tokens()
{
    printf("test_tokens...");

    PetriNet *net;
    pn_weight m[5] = {9,9,9,9,9};
    pn_weight marks[5] = {1,2,3,4,5};
    net = petri_new(5, 3);

    petri_marking_get(net, m);
    for (pn_place p=0; p < 5; p++){
        assert(petri_weight_of(net, p) == 0);
        assert(m[p] == 0);
    }

    petri_marking_set(net, marks);

    petri_marking_get(net, m);
    for (pn_place p=0; p < 5; p++){
        assert(petri_weight_of(net, p) == p+1);
        assert(m[p] == p+1);
    }

    petri_free(net);
    puts("OK");
}/* test_weights */


static void test_fire()
{
    printf("test_fire...");

    PetriNet *net;
    pn_weight marks[5] = {0,5,0,0,1};

    net = petri_new(5, 3);

    petri_conf_input(net, 1, 1, 2);
    petri_conf_input(net, 4, 1, 1);
    petri_conf_input(net, 2, 2, 2);

    petri_conf_output(net, 1, 2, 2);
    petri_conf_output(net, 2, 4, 1);
    petri_conf_output(net, 2, 3, 2);

    assert(petri_trans_enabled(net, 0) == true); /* no inputs */
    assert(petri_trans_enabled(net, 1) == false);
    assert(petri_trans_enabled(net, 2) == false);

    petri_marking_set(net, marks);

    /* Fire T1 */
    assert(petri_trans_enabled(net, 0) == true);
    assert(petri_trans_enabled(net, 1) == true);
    assert(petri_trans_enabled(net, 2) == false);

    assert(petri_fire(net, 0) == true);
    assert(petri_fire(net, 2) == false);
    assert(petri_fire(net, 1) == true);

    assert(petri_weight_of(net, 0) == 0);
    assert(petri_weight_of(net, 1) == 3);
    assert(petri_weight_of(net, 2) == 2);
    assert(petri_weight_of(net, 3) == 0);
    assert(petri_weight_of(net, 4) == 0);

    /* FIRE T2 */
    assert(petri_trans_enabled(net, 0) == true);
    assert(petri_trans_enabled(net, 1) == false);
    assert(petri_trans_enabled(net, 2) == true);

    assert(petri_fire(net, 0) == true);
    assert(petri_fire(net, 1) == false);
    assert(petri_fire(net, 2) == true);

    assert(petri_weight_of(net, 0) == 0);
    assert(petri_weight_of(net, 1) == 3);
    assert(petri_weight_of(net, 2) == 0);
    assert(petri_weight_of(net, 3) == 2);
    assert(petri_weight_of(net, 4) == 1);

    /* FIRE T1 */
    assert(petri_trans_enabled(net, 0) == true);
    assert(petri_trans_enabled(net, 1) == true);
    assert(petri_trans_enabled(net, 2) == false);

    assert(petri_fire(net, 0) == true);
    assert(petri_fire(net, 2) == false);
    assert(petri_fire(net, 1) == true);

    assert(petri_weight_of(net, 0) == 0);
    assert(petri_weight_of(net, 1) == 1);
    assert(petri_weight_of(net, 2) == 2);
    assert(petri_weight_of(net, 3) == 2);
    assert(petri_weight_of(net, 4) == 0);

    /* FIRE T2 */
    assert(petri_trans_enabled(net, 0) == true);
    assert(petri_trans_enabled(net, 1) == false);
    assert(petri_trans_enabled(net, 2) == true);

    assert(petri_fire(net, 0) == true);
    assert(petri_fire(net, 1) == false);
    assert(petri_fire(net, 2) == true);

    assert(petri_weight_of(net, 0) == 0);
    assert(petri_weight_of(net, 1) == 1);
    assert(petri_weight_of(net, 2) == 0);
    assert(petri_weight_of(net, 3) == 4);
    assert(petri_weight_of(net, 4) == 1);

    /* T1, T2 disabled */
    assert(petri_trans_enabled(net, 0) == true);
    assert(petri_trans_enabled(net, 1) == false);
    assert(petri_trans_enabled(net, 2) == false);

    petri_free(net);
    puts("OK");
}/* test_fire */

static void test_source_sink()
{
    printf("test_source_sink...");

    PetriNet *net;
    pn_weight marks[2] = {1,1};

    net = petri_new(2, 2);

    /* T0 source */
    petri_conf_output(net, 0, 0, 1);
    petri_conf_output(net, 0, 1, 2);

    /* T1 sink */
    petri_conf_input(net, 0, 1, 2);
    petri_conf_input(net, 1, 1, 1);

    petri_marking_set(net, marks);

    /* Fire T0 */
    assert(petri_trans_enabled(net, 0) == true);
    assert(petri_trans_enabled(net, 1) == false);

    assert(petri_fire(net, 0) == true);

    assert(petri_weight_of(net, 0) == 2);
    assert(petri_weight_of(net, 1) == 3);

    /* FIRE T1 */
    assert(petri_trans_enabled(net, 0) == true);
    assert(petri_trans_enabled(net, 1) == true);

    assert(petri_fire(net, 1) == true);

    assert(petri_weight_of(net, 0) == 0);
    assert(petri_weight_of(net, 1) == 2);

    /* done */
    petri_free(net);

    puts("OK");
}/* test_source_sink */

int main()
{
    test_new();
    test_weights();
    test_tokens();
    test_fire();
    test_source_sink();
    return 0;
}/* main */

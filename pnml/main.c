// Run the test

#include "petri.h"
#include "test.h"
#include <stdio.h>
#include <assert.h>

void mark_print(PetriNet *net)
{
    pn_weight mark[TEST_PLACES_ALL];
    petri_marking_get(net, mark);

    puts("+++++++++++++++++++++++++++++++++++++++++");
    for (int i=0; i<TEST_PLACES_ALL; i++){
        printf("P%i: %u\n", i+1, mark[i]);
    }
    puts("-----------------------------------------");
}

int main()
{
    PetriNet *net = test_petri_new();

    mark_print(net);

    puts("T1");
    assert(petri_fire(net, T1));
    mark_print(net);

    for (int i=0; i < 3; i++){
        puts("T2");
        assert(petri_fire(net, T2));
        mark_print(net);
    }

    petri_free(net);
    return 0;
}

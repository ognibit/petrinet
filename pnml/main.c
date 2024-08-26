/*
 * Get in input the command that fire a trasition.
 * It returns if the firing succeeded and the current marking
 */

#include "petri.h"
#include "test.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>


#define BUFSIZE 512

void mark_print(PetriNet *net)
{
    pn_weight mark[TEST_PLACES_ALL];
    petri_marking_get(net, mark);

    for (int i=0; i<TEST_PLACES_ALL; i++){
        printf("%s: %u\n", test_petri_place_str(i), mark[i]);
    }
}

void strupr(char *s)
{
    for (size_t i=0; s[i] != '\0'; i++){
        s[i] = toupper(s[i]);
    }
}

int main()
{
    char buf[BUFSIZE];
    bool active = true;
    PetriNet *net = test_petri_new();

    mark_print(net);
    while (active && (fgets(buf, BUFSIZE-1, stdin) != NULL)){
        char *nl = strchr(buf, '\n');
        if (nl == NULL){
            break;
        }
        *nl = '\0';
        strupr(buf);
        pn_trans t = test_petri_str_trans(buf);
        if (t == TEST_TRANS_ALL){
            puts("UNKNOWN");
            continue;
        }

        if (petri_fire(net, t)){
            mark_print(net);
        } else {
            puts("NOT ENABLED");
        }
    }

    petri_free(net);
    return 0;
}

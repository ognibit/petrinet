"""
Generate the C source code from a PNML file.

Use: <file> <module>

File: the filename of the PNML file to read.
Module: the name of the C module. It is used to generate module.h and module.c

In the header file there the declaration of the places and transitions as
enumeration MODULE_PLACES, MODULE_TRANSITIONS. There is also the declaration
of the 'module_petri_new()' function to initialize the network with all the
arcs and the initial marking.

In the definition of `module_petri_new()'.
"""

from pntools.petrinet import parse_pnml_file

IND = " " * 4

def load_net(filename):
    return parse_pnml_file(filename)[0]


def gen_header(net, modulename):
    headerBegin = """
/* Generated from PNML file. */
#include "petri.h"

"""
    header = headerBegin

    # PLACES
    placeEnum = modulename.capitalize() + "Place"
    header += "enum %s {\n"%(placeEnum)
    places = [(i, p.label.upper(), p.marking)
              for i,p in enumerate(net.places.values())]
    nplaces = modulename.upper()+"_PLACES_ALL"
    pls = [IND+p[1] for p in places]
    header += ",\n".join(pls + [IND+nplaces])
    header += "\n};\n"

    header += "\n"

    # TRANSITIONS
    transEnum = modulename.capitalize() + "Transition"
    header += "enum %s {\n"%(transEnum)
    trans = [IND+t.label.upper() for t in net.transitions.values()]
    ntrans = modulename.upper()+"_TRANS_ALL"
    trans.append(IND+ntrans)
    header += ",\n".join(trans)
    header += "\n};\n"

    header += "\n"

    # FUNCTIONS
    funNew = "PetriNet * %s_petri_new()"%(modulename)

    header += funNew + ";\n"

    headerFn = modulename + ".h"
    with open(headerFn, "w") as f:
        f.write(header)

    return {"filename": headerFn,
            "function": funNew,
            "nplaces": nplaces,
            "ntrans": ntrans,
            "places": places,

            }
# gen_header

def gen_source(net, modulename, hinfo):
    begin = """/* Generated from PNML file */
#include "petri.h"
#include "%s"

%s {\n
"""
    content = begin % (hinfo["filename"], hinfo["function"])

    content += IND + "PetriNet *net = petri_new(%s, %s);\n" % (
              hinfo["nplaces"], hinfo["ntrans"])

    content += IND + "if (net == NULL) return NULL;\n"

    # INITIAL MARKING
    content += IND + "pn_weight m[] = {"
    for p in hinfo["places"]:
        content += str(p[2]) + ","
    content += "};\n"
    content += IND + "petri_marking_set(net, m);\n"

    # FLOW
    for arc in net.edges:
        a = arc.find_source().label.upper()
        b = arc.find_target().label.upper()
        w = int(arc.inscription)
        if arc.source in net.transitions:
            content += IND + "petri_conf_output(net, %s, %s, %i);\n" % (a, b, w)
        else:
            content += IND + "petri_conf_input(net, %s, %s, %i);\n" % (a, b, w)

    # FINISH
    content += IND + "return net;\n"
    content += "}/* %s */\n" % (hinfo["function"])

    fn = modulename + ".c"

    with open(fn, "w") as f:
        f.write(content)

    return fn
# gen_source

def main(filename, modulename):

    modulename = modulename.lower()

    net = load_net(filename)

    h = gen_header(net, modulename)
    print("Generated", h["filename"])

    c = gen_source(net, modulename, h)
    print("Generated", c)
# main

if __name__ == "__main__":
    import sys
    filename = sys.argv[1]
    modulename = sys.argv[2]

    main(filename, modulename)

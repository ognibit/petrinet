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
#pragma once
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
    trans = [t.label.upper() for t in net.transitions.values()]
    itrans = [IND+t for t in trans]
    ntrans = modulename.upper()+"_TRANS_ALL"
    itrans.append(IND+ntrans)
    header += ",\n".join(itrans)
    header += "\n};\n"

    header += "\n"


    # FUNCTIONS
    funNew = "PetriNet * %s_petri_new()"%(modulename)
    fplacestr = "const char * %s_petri_place_str(pn_place p)"%(modulename)
    fstrtrans = "pn_trans %s_petri_str_trans(const char *s)"%(modulename)

    header += "\n" + funNew + ";\n"
    header += "\n" + fplacestr + ";\n"
    header += "\n" + fstrtrans + ";\n"

    headerFn = modulename + ".h"
    with open(headerFn, "w") as f:
        f.write(header)

    return {"filename": headerFn,
            "fnew": funNew,
            "fplacestr": fplacestr,
            "fstrtrans": fstrtrans,
            "nplaces": nplaces,
            "ntrans": ntrans,
            "places": places,
            "trans": trans,

            }
# gen_header

def gen_fplacestr(net, modulename, hinfo):
    content = "%s {\n" % (hinfo["fplacestr"])
    content += IND+'if (p >= %s) return NULL;\n' % (hinfo["nplaces"])
    content += IND+"return %s[p];\n" % (hinfo["pnames"]);
    content += "}/* %s */\n" % (hinfo["fplacestr"])
    return content;
# gen_fplacestr


def gen_fstrtrans(net, modulename, hinfo):
    content = "%s {\n" % (hinfo["fstrtrans"])
    content += IND+'if (s == NULL) return %s;\n'%(hinfo["ntrans"])
    content += IND+"for (pn_trans i=0; i < %s; i++){\n"%(hinfo["ntrans"])
    content += (IND*2)+"if (strcmp(s, %s[i]) == 0)\n"%(hinfo["tnames"])
    content += (IND*3)+"return i;\n"
    content += IND+"}/* for */\n"
    content += IND+"return %s;\n" % (hinfo["ntrans"]);
    content += "}/* %s */\n" % (hinfo["fstrtrans"])
    return content;
# gen_fstrtrans


def gen_fnew(net, modulename, hinfo):

    content = "%s {\n" % (hinfo["fnew"])

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
    content += "}/* %s */\n" % (hinfo["fnew"])

    return content
# gen_fnew

def gen_source(net, modulename, hinfo):
    begin = """/* Generated from PNML file */
#include "petri.h"
#include "%s"
#include <string.h>

"""
    # PLACES NAMES
    pnames = modulename.upper()+"_PLACES_NAMES";
    hinfo["pnames"] = pnames
    begin += "const char *%s[%s] = {\n" % (
              pnames, hinfo["nplaces"])
    names = [IND+'[%s] = "%s"'%(p[1],p[1]) for p in hinfo["places"]]
    begin += ",\n".join(names)
    begin += "\n};\n";

    # TRANSITIONS NAMES
    tnames = modulename.upper()+"_TRANS_NAMES";
    hinfo["tnames"] = tnames
    begin += "const char *%s[%s] = {\n" % (
              tnames, hinfo["ntrans"])
    names = [IND+'[%s] = "%s"'%(t,t) for t in hinfo["trans"]]
    begin += ",\n".join(names)
    begin += "\n};\n";

    content = begin % (hinfo["filename"])
    content += gen_fplacestr(net, modulename, hinfo) + '\n'
    content += gen_fstrtrans(net, modulename, hinfo) + '\n'
    content += gen_fnew(net, modulename, hinfo) + '\n'

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

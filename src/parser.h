#ifndef PARSER_H
#define PARSER_H

/*
 * parser.h - PNML file parser (Task 1)
 * Reads Petri Net Markup Language files and builds Model structure
 */

#include <string>
#include "utils.h"

// Parse PNML file, optionally export DOT graph
Model parsePNML(const std::string& filename,
                bool exportDot = true,
                const std::string& dotPath = "../output/petri_net.dot");

void printModelSummary(const Model& model);
bool validateModel(const Model& model);

#endif

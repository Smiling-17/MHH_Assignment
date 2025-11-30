#ifndef PARSER_H
#define PARSER_H

#include <string>
#include "utils.h"

// Hàm parse trả về Model đã định nghĩa trong utils.h
Model parsePNML(const std::string& filename,
                bool exportDot = true,
                const std::string& dotPath = "../output/petri_net.dot");

// Hàm utility để in thông tin model (hữu ích cho debug)
void printModelSummary(const Model& model);

// Hàm validate model cơ bản
bool validateModel(const Model& model);

#endif

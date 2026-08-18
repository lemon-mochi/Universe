#pragma once

#include "SimParams.h"
#include <string>

void printDivider();
double promptDouble(const std::string& label, double current);
void configureParams(SimParams& params);
void printWelcome();
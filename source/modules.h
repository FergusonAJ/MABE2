/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019
 *
 *  @file  modules.h
 *  @brief A full set of all standard modules available in MABE.
 */

// Evaluation Modules
#include "evaluate/EvalCountBits.h"
#include "evaluate/EvalMatchBits.h"
#include "evaluate/EvalNK.h"
#include "evaluate/EvalTest.h"

// Interface Modules
#include "interface/CommandLine.h"
#include "interface/DumpPopulation.h"

// Placement Modules
#include "placement/GrowthPlacement.h"

// Selection Modules
#include "select/SelectElite.h"
#include "select/SelectTournament.h"

// Other schema
#include "schema/MovePopulation.h"
#include "schema/Mutate.h"
#include "schema/OneStepMutants.h"
#include "schema/EmptyPopulation.h"

// Organism Types
#include "orgs/BitsOrg.h"

/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2024.
 *
 *  @file  modules.hpp
 *  @brief A full set of all standard modules available in MABE.
 */

// Analyze Modules
#include "analyze/SystematicsModule.hpp"
#include "analyze/TrackAncestor.hpp"

// Evaluation Modules
#include "evaluate/FixedTrait.hpp"
#include "evaluate/AddTraits.hpp"
#include "evaluate/games/EvalMancala.hpp"
#include "evaluate/games/EvalPathFollow.hpp"
#include "evaluate/games/EvalPatchHarvest.hpp"
#include "evaluate/games/EvalDoors.hpp"
#include "evaluate/games/EvalCues.hpp"
#include "evaluate/static/Eval2DGrid.hpp"
#include "evaluate/games/EvalSudoku.hpp"
#include "evaluate/static/EvalCountBits.hpp"
#include "evaluate/static/EvalDiagnostic.hpp"
#include "evaluate/static/EvalMatchBits.hpp"
#include "evaluate/static/EvalNK.hpp"
#include "evaluate/static/EvalSawtooth.hpp"
#include "evaluate/static/EvalSawtooth_Regular.hpp"
#include "evaluate/static/EvalNK-MultiAllele.hpp"
#include "evaluate/static/EvalRoyalRoad.hpp"
#include "evaluate/callable/EvalTaskNot.hpp"
#include "evaluate/callable/EvalTaskNand.hpp"
#include "evaluate/callable/EvalTaskAnd.hpp"
#include "evaluate/callable/EvalTaskOr.hpp"
#include "evaluate/callable/EvalTaskAndnot.hpp"
#include "evaluate/callable/EvalTaskOrnot.hpp"
#include "evaluate/callable/EvalTaskNor.hpp"
#include "evaluate/callable/EvalTaskXor.hpp"
#include "evaluate/callable/EvalTaskEqu.hpp"
#include "evaluate/static/EvalPacking.hpp"
#include "evaluate/static/EvalRandom.hpp"
#include "evaluate/static/EvalGeneric.hpp"
#include "evaluate/static/EvalInterpolatedFitnessGraph.hpp"

// Placement Modules
#include "placement/AnnotatePlacement_Position.hpp"
#include "placement/RandomReplacement.hpp"
#include "placement/MaxSizePlacement.hpp"
#include "placement/NeighborPlacement.hpp"
#include "placement/Spatial1DPlacement.hpp"
#include "placement/AdjacencyPlacement.hpp"

// Selection Modules
#include "select/SelectElite.hpp"
#include "select/SelectFitnessSharing.hpp"
#include "select/SelectLexicase.hpp"
#include "select/SchedulerProbabilistic.hpp"
#include "select/SelectRoulette.hpp"
#include "select/SelectTournament.hpp"

// Organism Types
#include "orgs/BitsOrg.hpp"
#include "orgs/MultiAlleleOrg.hpp"
#include "orgs/BitSummaryOrg.hpp"
#include "orgs/IntsOrg.hpp"
#include "orgs/GenericIntsOrg.hpp"
#include "orgs/StatesOrg.hpp"
#include "orgs/ValsOrg.hpp"
#include "orgs/AvidaGPOrg.hpp"
#include "orgs/VirtualCPUOrg.hpp"
#include "orgs/instructions/VirtualCPU_Inst_Nop.hpp"
#include "orgs/instructions/VirtualCPU_Inst_Math.hpp"
#include "orgs/instructions/VirtualCPU_Inst_IO.hpp"
#include "orgs/instructions/VirtualCPU_Inst_Flow.hpp"
#include "orgs/instructions/VirtualCPU_Inst_Label.hpp"
#include "orgs/instructions/VirtualCPU_Inst_Manipulation.hpp"
#include "orgs/instructions/VirtualCPU_Inst_Replication.hpp"
#include "orgs/instructions/VirtualCPU_Inst_Dictionary.hpp"

// Tools
#include "tools/MutantGenerator.hpp"
#include "tools/StringUtils.hpp"
#include "tools/RandomNumberGenerator.hpp"

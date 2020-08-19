/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2020.
 *
 *  @file  OneStepMutants.h
 *  @brief Collects the specified number of one-step mutants from a population
 */

#ifndef MABE_SCHEMA_ONE_STEP_MUT_H
#define MABE_SCHEMA_ONE_STEP_MUT_H

#include "../core/MABE.h"
#include "../core/Module.h"

namespace mabe {

  /// Add elite selection with the current population.
  class OneStepMutants : public Module {
  private:
    int source_pop_id = 0;   ///< Which population are we mutating?
    int mutant_pop_id = 0;   ///< Where to store mutants?
    size_t update_step = 1;   ///< How many updates between operations?
    int min_mutation_idx = 0; ///< Where to start the mutations (defined by org.Mutate_Deterministic)
    int max_mutation_idx = 0; ///< Where to stop the mutations (exclusive)
    std::string fitness_trait;
  public:
    OneStepMutants(mabe::MABE & control,
           const std::string & name="One Step Mutants",
           const std::string & desc="Generates one step mutants from a population.",
           size_t _source_pop_id=0,
           size_t _mutant_pop_id=1,
           size_t _update_step=1,
           int _min_mutation_idx=0,
           int _max_mutation_idx=0,
           const std::string & _ftrait="fitness")
      : Module(control, name, desc)
        , source_pop_id(_source_pop_id)
        , mutant_pop_id(_mutant_pop_id)
        , update_step(_update_step)
        , min_mutation_idx(_min_mutation_idx)
        , max_mutation_idx(_max_mutation_idx)
        , fitness_trait(_ftrait)
    {
    }

    void SetupConfig() override {
      LinkPop(source_pop_id, "source_pop", "Which population should we mutate?");
      LinkPop(mutant_pop_id, "mutant_pop", "Which population will store the mutants?");
      LinkVar(update_step, "update_step", "How many updates between operations?");
      LinkVar(min_mutation_idx, "min_mutation_idx", "Where to start mutations (start as defined by the organism)?");
      LinkVar(max_mutation_idx, "max_mutation_idx", "Where to stop the mutations?");
      LinkVar(fitness_trait, "fitness_trait", "Which trait should we use to rank organisms?");
    }

    void OnUpdate(size_t update) override {
      if(update % update_step == 0){
        Population & source_pop = control.GetPopulation(source_pop_id);
        Population & mutant_pop = control.GetPopulation(mutant_pop_id);
        emp_assert(source_pop.GetSize() > 0);
        // Pick out the best-performing organism
        double max_fitness = 0.0;
        size_t max_idx = 0;
        for(size_t org_idx = 0; org_idx < source_pop.GetSize(); ++org_idx){
          if(source_pop[org_idx].GetVar<double>(fitness_trait) > max_fitness){
            max_idx = org_idx;
            max_fitness = source_pop[org_idx].GetVar<double>(fitness_trait);
          }
        }
        const Organism & org = source_pop[max_idx];
        // Loop through the organisms (skipping any at the beginning that we need to) and
        // run Mutate() on each of them.
        size_t num_mutants = max_mutation_idx - min_mutation_idx;
        control.DoBirth(org, source_pop.begin().AsPosition(), mutant_pop, num_mutants);
        for (int mut_idx = min_mutation_idx; mut_idx < max_mutation_idx; mut_idx++){
          mutant_pop[mut_idx].Mutate_Deterministic(mut_idx);
        }
      }
    }
  };

  MABE_REGISTER_MODULE(OneStepMutants, "Generates one step mutants from a population.");
}

#endif

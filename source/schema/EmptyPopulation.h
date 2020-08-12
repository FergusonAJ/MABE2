/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2020.
 *
 *  @file  EmptyPopulation.h
 *  @brief Removes all organisms from a population 
 */

#ifndef MABE_SCHEMA_EMPTY_POPULATION_H
#define MABE_SCHEMA_EMPTY_POPULATION_H

#include "../core/Module.h"

namespace mabe {

  class EmptyPopulation : public Module {
  private:
    int pop_id = 1;       ///< Which population are we moving from?
    size_t update_step = 1;   ///< How many updates between operations?

  public:
    EmptyPopulation(mabe::MABE & control,
           const std::string & name="EmptyPopulation",
           const std::string & desc="Remove all organisms from a population",
           int _pop_id = 0,
           size_t _update_step=1)
      : Module(control, name, desc), pop_id(_pop_id), update_step(_update_step)
    {
      SetManageMod(true);         ///< Mark this module as a population  module.
    }

    void SetupConfig() override {
      LinkPop(pop_id, "pop", "Population to remove organisms from.");
      LinkVar(update_step, "update_step", "How many updates between operations?");
    }

    void OnUpdate(size_t update) override {
      if(update % update_step == 0){
        Population & pop = control.GetPopulation(pop_id);
        control.EmptyPop(pop, 0);  // Clear out the population.
      }
    }
  };

  MABE_REGISTER_MODULE(EmptyPopulation, "Empty organisms from one populaiton to another.");
}

#endif

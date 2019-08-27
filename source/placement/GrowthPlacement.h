/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019
 *
 *  @file  GrowthPlacement.h
 *  @brief Default placement using one or two unstructured populations, "main" and "next".
 */

#ifndef MABE_GROWTH_PLACEMENT_H
#define MABE_GROWTH_PLACEMENT_H

#include "../core/MABE.h"
#include "../core/Module.h"

namespace mabe {

  class GrowthPlacement : public Module {
  private:
    int main_pop=0;
    int next_pop=1;

  public:
    GrowthPlacement(mabe::MABE & control)
      : Module(control, "GrowthPlacement", "Module to place births into the next generation.")
    {
      SetPlacementMod(true);
      RequireSync();
    }
    ~GrowthPlacement() { }

    void SetupConfig(ConfigScope & config_scope) override {
      // For now, nothing here.
    }

    void SetupModule() override {
      // For now, nothing here.
    }

    void OnUpdate(size_t update) override {
      // Move next population to current; delete current.
    }

    OrgPosition DoPlaceBirth(Organism & org, OrgPosition ppos) override {
      (void) org;  // By default, organism doesn't matter.

      // If birth is not coming from monitored population, don't place!
      if (ppos.PopID() != main_pop) return OrgPosition();
      return control.PushEmpty(control.GetPopulation(next_pop));  // @CAO Should use next pop...
    }

    // Injections always go into the active population.
    OrgPosition DoPlaceInject(Organism & org) override {
      (void) org;  // By default, organism doesn't matter.
      return control.PushEmpty(control.GetPopulation(main_pop));  // @CAO Should use main pop...
    }

    OrgPosition DoFindNeighbor(OrgPosition pos) override {
      emp::Ptr<Population> pop_ptr = pos.PopPtr();

      // If the current position is either not a population or not one monitored, don't find!
      if (pop_ptr.IsNull() || pos.PopID() != main_pop) return OrgPosition();
      return OrgPosition(pop_ptr, control.GetRandom().GetUInt(pop_ptr->GetSize()));
    }


  };

}

#endif
/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021.
 *
 *  @file  SelectRoulette.hpp
 *  @brief MABE module to enable roulette selection.
 */

#ifndef MABE_SELECT_ROULETTE_H
#define MABE_SELECT_ROULETTE_H

#include "../core/MABE.hpp"
#include "../core/Module.hpp"

#include "emp/datastructs/IndexMap.hpp"

namespace mabe {

  /// \brief Selects organisms with roulette (fitness-proportional) selection
  class SelectRoulette : public Module {
  private:
    std::string fit_equation;    ///< Which equation should we select on?

    /// Select num_births organisms from select_pop and replicate them into birth_pop
    Collection Select(Population & select_pop, Population & birth_pop, size_t num_births) {
      if (select_pop.GetID() == birth_pop.GetID()) {
        emp::notify::Error("SelectRoulette currently requires birth_pop and select_pop to be different.");
        return Collection{};
      }

      // Build fitness map using the fitness equation
      auto fit_fun = control.BuildTraitEquation(select_pop, fit_equation);
      emp::IndexMap fit_map(select_pop.GetSize(), 0.0);
      for (size_t org_pos = 0; org_pos < select_pop.GetSize(); org_pos++) {
        if (select_pop.IsEmpty(org_pos)) continue;
        fit_map[org_pos] = fit_fun(select_pop[org_pos]);
      }

      // Loop through picking IDs proportional to fitness_trait, replicating each
      emp::Random & random = control.GetRandom();
      Collection placement_list;
      for (size_t birth_id = 0; birth_id < num_births; birth_id++) {
        size_t org_id = fit_map.Index( random.GetDouble(fit_map.GetWeight()) );
        placement_list += control.Replicate(select_pop.IteratorAt(org_id), birth_pop);
      }

      return placement_list;
    }
    
    Collection SelectSpatial(Population & select_pop, Population & birth_pop) {
      // Do some quick error checking
      if (select_pop.GetID() == birth_pop.GetID()) {
        emp::notify::Error("SelectRoulette currently requires birth_pop and select_pop to be different.");
        return Collection{};
      }
      if (select_pop.GetNumOrgs() == 0) {
        emp::notify::Error("Trying to run Roulette Selection on an Empty Population.");
        return Collection();
      }

      emp::Random & random = control.GetRandom();
      const size_t N = select_pop.GetSize();

      // Setup the fitness function - redo this each time in case it changes.
      auto fit_fun = control.BuildTraitEquation(select_pop, fit_equation);

      // Track where all organisms are placed.
      Collection placement_list;
  
      for(size_t idx = 0; idx < N; idx++){
        auto neighbor_pos_vec = select_pop.FindAllNeighbors(OrgPosition(select_pop, idx));
        emp::IndexMap fit_map(N, 0.0); // Still use full pop size, but most will be 0
        // Add self first
        if (!select_pop.IsEmpty(idx)){
          fit_map[idx] = fit_fun(select_pop[idx]);
        }
        for(OrgPosition& neighbor_pos : neighbor_pos_vec){
          if (select_pop.IsEmpty(neighbor_pos.Pos())) continue;
          fit_map[neighbor_pos.Pos()] = fit_fun(select_pop[neighbor_pos.Pos()]);
        }
        size_t org_id = fit_map.Index( random.GetDouble(fit_map.GetWeight()) );
        placement_list += control.Replicate(select_pop.IteratorAt(org_id), birth_pop);

      }
      return placement_list;
    }

  public:
    SelectRoulette(
      mabe::MABE & control,
      const std::string & name="SelectRoulette",
      const std::string & desc="Module to choose random organisms for replication, proportional to their fitness."
    ) : Module(control, name, desc)
    {
      SetSelectMod(true);               ///< Mark this module as a selection module.
    } 
    ~SelectRoulette() { }

    // Set up member functions associated with this class.
    static void InitType(emplode::TypeInfo & info) {
      info.AddMemberFunction(
        "SELECT",
        [](SelectRoulette & mod, Population & from, Population & to, double count) {
          return mod.Select(from,to,count);
        },
        "Perform roulette selection on the provided organisms.");
      info.AddMemberFunction(
        "SELECT_SPATIAL",
        [](SelectRoulette & mod, Population & from, Population & to) {
          return mod.SelectSpatial(from,to);
        },
        "Perform roulette selection on the provided organisms using a spatial structure");
    }

    // Set up variables for configuration file
    void SetupConfig() override {
      LinkVar(fit_equation, "fitness_fun", "Function used as fitness for selection?");
    }

    /// Validate fitness equation from configuration file
    void SetupModule() override {
      AddRequiredEquation(fit_equation);   // The fitness traits must be set by another module
    }

  };

  MABE_REGISTER_MODULE(SelectRoulette, "Randomly choose organisms to replicate, with odds proportional to their fitness.");
}

#endif

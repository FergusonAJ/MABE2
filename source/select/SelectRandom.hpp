/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021.
 *
 *  @file  SelectRandom.hpp
 *  @brief MABE module to enable roulette selection.
 */

#ifndef MABE_SELECT_RANDOM_H
#define MABE_SELECT_RANDOM_H

#include "../core/MABE.hpp"
#include "../core/Module.hpp"

#include "emp/datastructs/IndexMap.hpp"

namespace mabe {

  /// \brief Selects organisms randomly
  class SelectRandom : public Module {
  private:
    std::string fit_equation;    ///< Which equation should we select on?

    /// Select num_births organisms from select_pop and replicate them into birth_pop
    Collection Select(Population & select_pop, Population & birth_pop, size_t num_births) {
      if (select_pop.GetID() == birth_pop.GetID()) {
        emp::notify::Error("SelectRandom currently requires birth_pop and select_pop to be different.");
        return Collection{};
      }

      // Loop through picking IDs randomly, replicating each
      emp::Random & random = control.GetRandom();
      Collection placement_list;
      for (size_t birth_id = 0; birth_id < num_births; birth_id++) {
        size_t org_id = random.GetUInt(select_pop.GetSize());
        placement_list += control.Replicate(select_pop.IteratorAt(org_id), birth_pop);
      }

      return placement_list;
    }
    
    Collection SelectSpatial(Population & select_pop, Population & birth_pop) {
      // Do some quick error checking
      if (select_pop.GetID() == birth_pop.GetID()) {
        emp::notify::Error("SelectRandom currently requires birth_pop and select_pop to be different.");
        return Collection{};
      }
      if (select_pop.GetNumOrgs() == 0) {
        emp::notify::Error("Trying to run Random Selection on an Empty Population.");
        return Collection();
      }

      emp::Random & random = control.GetRandom();
      const size_t N = select_pop.GetSize();

      // Track where all organisms are placed.
      Collection placement_list;
  
      // TODO: optimize, currently a quick swap from Roulette
      emp::UnorderedIndexMap fit_map(N, 0.0); // Still use full pop size, but most will be 0
      for(size_t idx = 0; idx < N; idx++){
        fit_map.Clear();
        auto neighbor_pos_vec = select_pop.HasStaticNeighbors() 
          ? select_pop.FindStaticNeighbors(OrgPosition(select_pop, idx)) 
          : select_pop.FindAllNeighbors(OrgPosition(select_pop, idx));
        // Add self first
        if (!select_pop.IsEmpty(idx)){
          fit_map[idx] = 1;
        }
        for(OrgPosition& neighbor_pos : neighbor_pos_vec){
          if (select_pop.IsEmpty(neighbor_pos.Pos())) continue;
          fit_map[neighbor_pos.Pos()] = 1;
        }
        size_t org_id = fit_map.Index( random.GetDouble(fit_map.GetWeight()) );
        placement_list += control.Replicate(select_pop.IteratorAt(org_id), birth_pop);

      }
      return placement_list;
    }

  public:
    SelectRandom(
      mabe::MABE & control,
      const std::string & name="SelectRandom",
      const std::string & desc="Module to choose random organisms for replication."
    ) : Module(control, name, desc)
    {
      SetSelectMod(true);               ///< Mark this module as a selection module.
    } 
    ~SelectRandom() { }

    // Set up member functions associated with this class.
    static void InitType(emplode::TypeInfo & info) {
      info.AddMemberFunction(
        "SELECT",
        [](SelectRandom & mod, Population & from, Population & to, double count) {
          return mod.Select(from,to,count);
        },
        "Perform roulette selection on the provided organisms.");
      info.AddMemberFunction(
        "SELECT_SPATIAL",
        [](SelectRandom & mod, Population & from, Population & to) {
          return mod.SelectSpatial(from,to);
        },
        "Perform roulette selection on the provided organisms using a spatial structure");
    }

    // Set up variables for configuration file
    void SetupConfig() override {
    }

    /// Validate configuration file settings
    void SetupModule() override {
    }

  };

  MABE_REGISTER_MODULE(SelectRandom, "Randomly choose organisms to replicate.");
}

#endif

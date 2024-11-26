/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2024.
 *
 *  @file  SelectTournament.hpp
 *  @brief MABE module to enable tournament selection (choose T random orgs and return "best")
 */

#ifndef MABE_SELECT_TOURNAMENT_H
#define MABE_SELECT_TOURNAMENT_H

#include "../core/MABE.hpp"
#include "../core/Module.hpp"

namespace mabe {

  /// Add elite selection with the current population.
  class SelectTournament : public Module {
  private:
    emp::String fit_equation;  ///< Trait function that we should select on
    size_t tourny_size;        ///< Number of organisms in each tournament

    Collection Select(Population & select_pop, Population & birth_pop, size_t num_births) {
      emp::Random & random = control.GetRandom();
      const size_t N = select_pop.GetSize();

      if (select_pop.GetNumOrgs() == 0) {
        emp::notify::Error("Trying to run Tournament Selection on an Empty Population.");
        return Collection();
      }

      // Setup the fitness function - redo this each time in case it changes.
      auto fit_fun = control.BuildTraitEquation(select_pop, fit_equation);

      // Track where all organisms are placed.
      Collection placement_list;

      // Loop through each round of tournament selection.
      for (size_t round = 0; round < num_births; round++) {
        // Find a random organism in the population and call it "best"
        size_t best_id = random.GetUInt(N);
        while (select_pop[best_id].IsEmpty()) best_id = random.GetUInt(N); // @CAO: better way for sparse pop?
        double best_fit = fit_fun(select_pop[best_id]);

        // Loop through other organisms for the rest of the tournament size, and pick best.
        for (size_t test=1; test < tourny_size; test++) {
          size_t test_id = random.GetUInt(N);
          while (select_pop[test_id].IsEmpty()) test_id = random.GetUInt(N);
          double test_fit = fit_fun(select_pop[test_id]);          
          if (test_fit > best_fit) {
            best_id = test_id;
            best_fit = test_fit;
          }
        }

        // Replicate the organism that did best in this tournament.
        placement_list += control.Replicate(select_pop.IteratorAt(best_id), birth_pop, 1);
      }

      return placement_list;
    }
    
    Collection SelectSpatial(Population & select_pop, Population & birth_pop) {
      emp::Random & random = control.GetRandom();
      const size_t N = select_pop.GetSize();

      if (select_pop.GetNumOrgs() == 0) {
        emp::notify::Error("Trying to run Tournament Selection on an Empty Population.");
        return Collection();
      }

      // Setup the fitness function - redo this each time in case it changes.
      auto fit_fun = control.BuildTraitEquation(select_pop, fit_equation);

      // Track where all organisms are placed.
      Collection placement_list;

      for(size_t idx = 0; idx < N; idx++){
        emp::vector<size_t> best_ids;
        best_ids.push_back(idx);
        double best_fit = fit_fun(select_pop[idx]);
        for(OrgPosition& neighbor_pos : 
            select_pop.FindAllNeighbors(OrgPosition(select_pop, idx))){
          double test_fit = fit_fun(select_pop[neighbor_pos.Pos()]);          
          if(test_fit > best_fit){
            best_ids.clear();
            best_ids.push_back(neighbor_pos.Pos());
            best_fit = test_fit;
          }
          else if (test_fit == best_fit){
            best_ids.push_back(neighbor_pos.Pos());
          }
        }
        size_t best_id = best_ids[random.GetUInt(best_ids.size())];
        placement_list += control.Replicate(select_pop.IteratorAt(best_id), birth_pop, 1);
      }
      return placement_list;
    }

  public:
    SelectTournament(mabe::MABE & control,
                     const emp::String & name="SelectTournament",
                     const emp::String & desc="Replicate most fit organisms from random subgroups.",
                     const emp::String & in_fit="fitness",
                     size_t t_size=7)
      : Module(control, name, desc)
      , fit_equation(in_fit), tourny_size(t_size)
    {
      SetSelectMod(true);              ///< Mark this module as a selection module.
    }
    ~SelectTournament() { }

    // Setup member functions associated with this class.
    static void InitType(emplode::TypeInfo & info) {
      info.AddMemberFunction(
        "SELECT",
        [](SelectTournament & mod, Population & from, Population & to, double count) {
          return mod.Select(from,to,count);
        },
        "Perform tournament selection on the provided organisms.");
      info.AddMemberFunction(
        "SELECT_SPATIAL",
        [](SelectTournament & mod, Population & from, Population & to) {
          return mod.SelectSpatial(from,to);
        },
        "Perform tournament selection for each index of the population using its neighborhood");
    }

    void SetupConfig() override {
      LinkVar(tourny_size, "tournament_size", "Number of orgs in each tournament");
      LinkVar(fit_equation, "fitness_fun", "Trait equation that produces fitness value to use");
    }

    void SetupModule() override {
      AddRequiredEquation(fit_equation); ///< The fitness traits must be set by another module.
    }

  };

  MABE_REGISTER_MODULE(SelectTournament, "Replicate top fitness organisms from random subgroups.");
}

#endif

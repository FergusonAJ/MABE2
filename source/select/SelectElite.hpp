/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2024.
 *
 *  @file  SelectElite.hpp
 *  @brief MABE module to enable elite selection (flexible to handle mu-lambda selection)
 */

#ifndef MABE_SELECT_ELITE_H
#define MABE_SELECT_ELITE_H

#include "../core/MABE.hpp"
#include "../core/Module.hpp"

#include "emp/datastructs/valsort_map.hpp"

namespace mabe {

  /// Add elite selection with the current population.
  class SelectElite : public Module {
  private:
    emp::String fit_equation;    ///< Which equation should we select on?
    size_t top_count=1;          ///< Top how-many should we select?

    Collection Select(Population & select_pop, Population & birth_pop, size_t num_births) {
      auto fit_fun = control.BuildTraitEquation(select_pop, fit_equation);

      // Construct a map of all IDs to their associated fitness values.
      emp::valsort_map<OrgPosition, double> id_fit_map;  // @CAO: Better to use a heap?
      for (auto it = select_pop.begin(); it != select_pop.end(); it++) {
        id_fit_map.Set(it.AsPosition(), fit_fun(*it));
      }

      // Loop through the IDs in fitness order (from highest), replicating each
      Collection placement_list;
      for (auto it = id_fit_map.crvbegin(); it != id_fit_map.crvend() && top_count; it++) {
        size_t copy_count = std::ceil(((double)num_births) / (double) top_count--);
        num_births -= copy_count;
        placement_list += control.Replicate(it->first, birth_pop, copy_count);
      }
      return placement_list;
    }

  public:
    SelectElite(mabe::MABE & control,
                const emp::String & name="SelectElite",
                const emp::String & desc="Module to choose the top fitness organisms for replication.",
                const emp::String & in_fit_equation="fitness", size_t tcount=1)
      : Module(control, name, desc)
      , fit_equation(in_fit_equation), top_count(tcount)
    {
      SetSelectMod(true);               ///< Mark this module as a selection module.
    } 
    ~SelectElite() { }

    // Setup member functions associated with this class.
    static void InitType(emplode::TypeInfo & info) {
      info.AddMemberFunction(
        "SELECT",
        [](SelectElite & mod, Population & from, Population & to, double count) {
          return mod.Select(from,to,count);
        },
        "Perform elite selection on the provided organisms.");
    }

    void SetupConfig() override {
      LinkVar(fit_equation, "fitness_fun", "Function used as fitness for selection?");
      LinkVar(top_count, "top_count", "Number of top-fitness orgs to be replicated");
    }

    void SetupModule() override {
      AddRequiredEquation(fit_equation);   // The fitness traits must be set by another module.
    }
  };

  MABE_REGISTER_MODULE(SelectElite, "Choose the top fitness organisms for replication.");
}

/*
== New Version:

module SelectElite {
  desc: "Module to choose a set of top scoring organisms."
  module_type: "selection"

  config fit_fun : TraitEquation { default: "fitness"; desc: "Function to use for selection"; }
  config top_count : Int { default: 1; desc: "Number of top-scoring orgs to select"; }

  function(Population select_pop, Int count) : PositionList
    desc: "Select the top scoring organisms from the population.";

    OrgList top_orgs = select_pop.FindMax(fit_fun, top_count);

    OrgList selected;
    while (selected.Size() < count) selected += top_orgs;
    selected.Resize(count); // Trim off any extra orgs.
    return selected;
  }
}

*/

#endif

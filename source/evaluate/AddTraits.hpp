/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2023.
 *
 *  @file  AddTraits.hpp
 *  @brief MABE module for adding two existing traits
 */

#ifndef MABE_ADD_TRAITS_H
#define MABE_ADD_TRAITS_H

#include "../core/MABE.hpp"
#include "../core/Module.hpp"

namespace mabe {

  class AddTraits : public Module {
  private:
    std::string trait_a_name = "trait_a";
    std::string trait_b_name = "trait_b";
    std::string output_trait_name = "output";
    std::string trait_type = "int";

  public:
    AddTraits(mabe::MABE & control,
           const std::string & name="AddTraits",
           const std::string & desc="Module to add pre-existing traits")
      : Module(control, name, desc){
      SetEvaluateMod(true);
    }
    ~AddTraits() { }

    // Setup member functions associated with this class.
    static void InitType(emplode::TypeInfo & info) {
      info.AddMemberFunction("ADD",
                              [](AddTraits & mod, mabe::Collection list) { 
                                return mod.Add(list); 
                             },
                             "Add the traits for each org in the collection");
    }
    
    void SetupConfig() override {
      LinkVar(trait_a_name, "trait_a_name", "The name of the first trait");
      LinkVar(trait_b_name, "trait_b_name", "The name of the second trait");
      LinkVar(output_trait_name, "output_trait_name", "The name of the output (sum) trait");
      LinkVar(trait_type, "trait_type", "Type of the trait (int or double)");
    }

    void SetupModule() override {
      // Setup the traits.
      if(trait_type == "int"){
        AddRequiredTrait<int>(trait_a_name);
        AddRequiredTrait<int>(trait_b_name);
        AddOwnedTrait<int>(output_trait_name, "A summed integer", {});
      }
      else if(trait_type == "double"){
        AddRequiredTrait<double>(trait_a_name);
        AddRequiredTrait<double>(trait_b_name);
        AddOwnedTrait<double>(output_trait_name, "A summed double", {});
      }
      else{
        emp_assert(false, "AddTraits currently only supports int or double");
      }
    }

    double Add(const mabe::Collection & orgs) {
      mabe::Collection alive_orgs( orgs.GetAlive() );
      if(trait_type == "int"){
        for (Organism & org : alive_orgs) {
          int a = org.GetTrait<int>(trait_a_name);
          int b = org.GetTrait<int>(trait_b_name);
          org.SetTrait<int>(output_trait_name, a + b);
        }
      }
      else if(trait_type == "double"){
        for (Organism & org : alive_orgs) {
          double a = org.GetTrait<double>(trait_a_name);
          double b = org.GetTrait<double>(trait_b_name);
          org.SetTrait<double>(output_trait_name, a + b);
        }
      }
      else{
        emp_assert(false, "Can only handle int or double");
      }
      return 0;
    }
    
  };

  MABE_REGISTER_MODULE(AddTraits, "Add two numeric traits");
}

#endif

/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2023.
 *
 *  @file  FixedTrait.hpp
 *  @brief MABE module for setting a particular trait in the population 
 */

#ifndef MABE_FIXED_TRAIT_H
#define MABE_FIXED_TRAIT_H

#include "../core/MABE.hpp"
#include "../core/Module.hpp"

namespace mabe {

  class FixedTrait : public Module {
  private:
    std::string trait_name = "fixed_trait";
    std::string trait_type = "int";

  public:
    FixedTrait(mabe::MABE & control,
           const std::string & name="FixedTrait",
           const std::string & desc="Module to set a trait in the population")
      : Module(control, name, desc){
      SetEvaluateMod(true);
    }
    ~FixedTrait() { }

    // Setup member functions associated with this class.
    static void InitType(emplode::TypeInfo & info) {
      info.AddMemberFunction("SET_DOUBLE",
                              [](FixedTrait & mod, mabe::Collection list, double val) { 
                                return mod.Set_Double(list, val); 
                             },
                             "Set a double for the entire collection");
      info.AddMemberFunction("SET_INT",
                              [](FixedTrait & mod, mabe::Collection list, int val) { 
                                return mod.Set_Int(list, val); 
                             },
                             "Set an int for the entire collection");
    }
    
    void SetupConfig() override {
      LinkVar(trait_name, "trait_name", "Name of the trait");
      LinkVar(trait_type, "trait_type", "Type of the trait (int or double)");
    }

    void SetupModule() override {
      // Setup the traits.
      if(trait_type == "int"){
        AddOwnedTrait<int>(trait_name, "A fixed integer", {});
      }
      else if(trait_type == "double"){
        AddOwnedTrait<double>(trait_name, "A fixed double", {});
      }
      else{
        emp_assert(false, "FixedTrait currently only supports int or double");
      }
    }

    double Set_Int(const mabe::Collection & orgs, int val) {
      mabe::Collection alive_orgs( orgs.GetAlive() );
      for (Organism & org : alive_orgs) {
        org.SetTrait<int>(trait_name, val);
      }
      return static_cast<double>(val);
    }
    
    double Set_Double(const mabe::Collection & orgs, double val) {
      mabe::Collection alive_orgs( orgs.GetAlive() );
      for (Organism & org : alive_orgs) {
        org.SetTrait<double>(trait_name, val);
      }
      return val;
    }
  };

  MABE_REGISTER_MODULE(FixedTrait, "Evaluate integer vectors on a sawtooth lanscape.");
}

#endif

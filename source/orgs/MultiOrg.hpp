/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2025.
 *
 *  @file  MultiOrg.hpp
 *  @brief An organism containing multiple other organisms
 *  @note Status: ALPHA
 */

#ifndef MABE_MULTI_ORGANISM_H
#define MABE_MULTI_ORGANISM_H

#include "../core/MABE.hpp"
#include "../core/Organism.hpp"
#include "../core/OrganismManager.hpp"

namespace mabe {

  class MultiOrg : public OrganismTemplate<MultiOrg> {
  protected:
    emp::Ptr<Population> pop_ptr = nullptr;
  public:
    MultiOrg(OrganismManager<MultiOrg> & _manager)
      : OrganismTemplate<MultiOrg>(_manager){ }
    MultiOrg(const MultiOrg &) = default;
    MultiOrg(MultiOrg &&) = default;
    ~MultiOrg() { ; }

    struct ManagerData : public Organism::ManagerData {
      emp::Ptr<Module> org_manager_ptr;
      emp::String org_module_name;
      SharedTrait<OrgPosition> position_trait{this, "position", "Organism's position"};
      std::unordered_map<size_t, emp::Ptr<Population>> pos_to_pop_map;
    };

    emp::String ToString() const override {
      std::stringstream sstr;
      sstr << "MultiOrg of size: " << pop_ptr->GetSize();
      return sstr.str();
    }

    void ClaimOrg(Organism& org) {
      org.SetContainerOrg(this);
    }

    void InjectOrg(size_t count = 1){
      Collection new_orgs = GetManager().GetControl().Inject(*pop_ptr, 
          SharedData().org_module_name, count);
      for(auto it = new_orgs.begin(); it != new_orgs.end(); it++){
        ClaimOrg(*it);
      }
    }

    size_t Mutate(emp::Random & /*random*/) override {
      return 0;
    }

    void Randomize(emp::Random & /*random*/) override {
      //emp::RandomizeBitVector(bits, random, 0.5);
    }

    void Initialize(emp::Random & /*random*/) override {
      MarkAsContainer();
      MABE& control = GetManager().GetControl();
      const size_t pos = SharedData().position_trait(*this).Pos();
      if(!emp::Has(SharedData().pos_to_pop_map, pos)){
        std::stringstream sstr;
        sstr << "multi_org_pop_" << pos;
        Population& new_pop = control.AddPopulation(sstr.str(), 0);
        SharedData().pos_to_pop_map[pos] = &new_pop;
        pop_ptr = &new_pop;
      }
      else{
        pop_ptr = SharedData().pos_to_pop_map[pos];
        control.EmptyPop(*pop_ptr); // Remove any existing orgs
      }
      InjectOrg(1);
      //if (SharedData().init_random) emp::RandomizeBitVector(bits, random, 0.5);
    }

    /// TODO
    void GenerateOutput() override {
      //SetTrait<emp::BitVector>(SharedData().output_name, bits);
    }

    /// Setup this organism type to be able to load from config.
    void SetupConfig() override {
      GetManager().LinkVar(SharedData().org_module_name, "org_module_name",
          "Name of the organism module we are holding (as defined in config file)");
    }

    /// Setup this organism type with the traits it need to track.
    void SetupModule() override {
      MABE& control = GetManager().GetControl();
      emp::Ptr<ModuleBase> base_ptr = &(control.GetModule(SharedData().org_module_name));
      SharedData().org_manager_ptr = base_ptr.DynamicCast<Module>();
    }

    size_t GetNumContainedOrgs() { return pop_ptr->GetSize(); }

    emp::Ptr<Organism> GetContainedOrg(size_t index) { return &pop_ptr->At(index); }
  };

  MABE_REGISTER_ORG_TYPE(MultiOrg, "An organism containing other organisms");
}

#endif

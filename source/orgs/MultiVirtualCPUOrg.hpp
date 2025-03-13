/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021-2022.
 *
 *  @file  MultiVCPUOrg.hpp
 *  @brief A wrapper of VirtualCPUOrg to support multiple organisms operating together 
 *  
 *  @note Status: ALPHA
 *
 */

#ifndef MABE_MULTI_VIRTUAL_CPU_ORGANISM_H
#define MABE_MULTI_VIRTUAL_CPU_ORGANISM_H

#include "VirtualCPUOrg.hpp"

namespace mabe {

  class MultiVirtualCPUOrg;
  
  class VirtualCPUOrg_Multi : public VirtualCPUOrg{
    public: 
      using container_t = MultiVirtualCPUOrg;
      using this_t = VirtualCPUOrg_Multi;
      
      emp::Ptr<container_t> container_ptr;
      
      VirtualCPUOrg_Multi(OrganismManager<VirtualCPUOrg> & _manager)
        : VirtualCPUOrg(_manager){ }
      VirtualCPUOrg_Multi(const VirtualCPUOrg_Multi &) = default;
      VirtualCPUOrg_Multi(VirtualCPUOrg_Multi &&) = default;
      ~VirtualCPUOrg_Multi() { ; }

  };
  
  mabe::ManagerModuleRegistrar<ManagerModule<VirtualCPUOrg, Organism>> MABE_VirtualCPUOrg_Multi_Registrar("VirtualCPUOrg_Multi", "foo");

  class MultiVirtualCPUOrg : 
    public OrganismTemplate<MultiVirtualCPUOrg>
  {
    protected:
      emp::vector<emp::Ptr<VirtualCPUOrg_Multi>> org_ptrs;
      size_t active_org_index = 0;
      
    public: 
      using this_t = MultiVirtualCPUOrg;
      using manager_t = OrganismManager<this_t>;
      using org_t = VirtualCPUOrg_Multi;
      using org_base_t = org_t::base_t;
      using org_manager_t = org_t::manager_t;
      
      // Organism constructor
      MultiVirtualCPUOrg(OrganismManager<MultiVirtualCPUOrg> & _manager)
          : OrganismTemplate<MultiVirtualCPUOrg>(_manager){ } 
      MultiVirtualCPUOrg(const MultiVirtualCPUOrg &) = default;
      MultiVirtualCPUOrg(MultiVirtualCPUOrg &&) = default;
      ~MultiVirtualCPUOrg() { ; }
      
      struct ManagerData : public Organism::ManagerData {
        emp::Ptr<org_manager_t> org_manager_ptr;
        emp::Ptr<Population> pop_ptr;
        emp::String org_module_name = "[not set]";
      };
      
      size_t Mutate(emp::Random & random) override { return 0; }
      
      void Randomize(emp::Random & random) override { ; }
        
      void Initialize(emp::Random & random) override { 
        org_ptrs.push_back(SharedData().org_manager_ptr->Make<org_t>(random));
      }
      
      emp::Ptr<Organism> MakeOffspringOrganism(emp::Random & random) const override {
        return nullptr;
      }
      
      virtual emp::Ptr<Organism> CloneOrganism() const override {
        return nullptr;
      }
        
      void GenerateOutput() override { ; }
        
      void SetupConfig() override { 
        GetManager().LinkVar(SharedData().org_module_name, "org_module_name",
                        "Name (not type) of the VirtualCPUOrg_Multi module to link to");
      }
        
      void SetupModule() override {
        SharedData().org_manager_ptr = GetManager().GetControl().GetModulePtr(SharedData().org_module_name).DynamicCast<org_manager_t>();
      }
        
      bool ProcessStep() override { 
        bool result = org_ptrs[active_org_index]->ProcessStep(); 
        active_org_index = (active_org_index + 1) % org_ptrs.size();
      }
      
      emp::String ToString() const override { return "(Not implemented)"; }
      
      void GenomeFromString(const std::string & new_genome) override { ; }
    
      //static void InitType(emplode::TypeInfo & info) {
      //  std::cout << "Running InitType!" << std::endl;
      //  info.AddMemberFunction("TEST",
      //                         [](MultiVirtualCPUOrg::manager_t & mod) { 
      //                           std::cout << "The test worked!" << std::endl;
      //                           return 0; 
      //                        },
      //                        "test");
      //}

      void Replicate_Internal(org_base_t& org_ptr, OrgPosition org_pos){
        std::cout << "Local repro" << std::endl;
      }

   };


  MABE_REGISTER_ORG_TYPE(MultiVirtualCPUOrg, "Organism consisting multiple VirtualCPUOrgs.");

}

#endif

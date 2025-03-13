/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021-2022.
 *
 *  @file  MultiVirtualCPU_Inst_Replication.hpp
 *  @brief Provides replication instructions to a VirtualCPUOrg_Multi inside a MultiVirtualCPUOrg
 *
 * 
 */

#ifndef MABE__MULTI_VIRTUAL_CPU_INST_REPLICATION_H
#define MABE__MULTI_VIRTUAL_CPU_INST_REPLICATION_H

#include "../../core/MABE.hpp"
#include "../../core/Module.hpp"
#include "../MultiVirtualCPUOrg.hpp"

namespace mabe {

  /// A collection of replication instructions to be used by VirtualCPUOrgs 
  class MultiVirtualCPU_Inst_Replication : public Module {
  public:
    using org_t = VirtualCPUOrg_Multi;
    using org_base_t = VirtualCPUOrg;
    using inst_func_t = org_t::inst_func_t;
    using this_t = MultiVirtualCPU_Inst_Replication;
  private:
    int pop_id = 0; ///< ID of the population which will receive these instructions
    std::string org_pos_trait = "org_pos"; ///< Name of the trait storing organism's position
    std::string offspring_genome_trait = "offspring_genome"; ///< Name of the trait storing the genome of the offspring organism 

  public:
    MultiVirtualCPU_Inst_Replication(mabe::MABE & control,
                    const std::string & name="MultiVirtualCPU_Inst_Replication",
                    const std::string & desc="Replication instructions for VirtualCPUOrg population")
      : Module(control, name, desc) {;}
    ~MultiVirtualCPU_Inst_Replication() { }

    void Inst_ReproIntra(org_base_t& hw, const org_t::inst_t& /* inst */){
      // TODO: Re-add requirements to repro
      if(true){
        OrgPosition& org_pos = hw.GetTrait<OrgPosition>(org_pos_trait);
        // Store the soon-to-be offspring's genome (just a copy of ours)
        org_t::genome_t& offspring_genome = hw.GetTrait<org_t::genome_t>(
            offspring_genome_trait);
        offspring_genome.resize(hw.genome.size(), hw.GetDefaultInst());
        std::copy(
            hw.genome.begin(),
            hw.genome.end(),
            offspring_genome.begin());
        ((org_t&)hw).container_ptr->Replicate_Internal(hw, org_pos);
        // Reset the parent
        hw.Reset();
        // Set to end so completion of this inst moves it 0 
        hw.inst_ptr = hw.genome_working.size() - 1; 
      }
    }

    void Inst_ReproInter(org_base_t& hw, const org_t::inst_t& /* inst */){
      // TODO: Re-add requirements to repro
      if(true){
        emp::Ptr<MultiVirtualCPUOrg> container_ptr = ((org_t&)hw).container_ptr;
        OrgPosition& org_pos = container_ptr->GetTrait<OrgPosition>(org_pos_trait);
        // Store the soon-to-be offspring's genome
        org_t::genome_t& offspring_genome = hw.GetTrait<org_t::genome_t>(
            offspring_genome_trait);
        offspring_genome.resize(hw.genome.size(), hw.GetDefaultInst());
        std::copy(
            hw.genome.begin(),
            hw.genome.end(),
            offspring_genome.begin());
        // Replicate 
        control.Replicate(org_pos, *org_pos.PopPtr());
        // Reset the parent
        hw.Reset();
        // Set to end so completion of this inst moves it 0 
        hw.inst_ptr = hw.genome_working.size() - 1; 
      }
    }

    /// Set up variables for configuration file
    void SetupConfig() override {
      LinkPop(pop_id, "target_pop", "Population(s) to manage.");
      LinkVar(org_pos_trait, "pos_trait", "Name of trait that holds organism's position");
      LinkVar(offspring_genome_trait, "offspring_genome_trait", 
          "Name of trait that holds the offspring organism's genome");
    }

    /// When config is loaded, create traits and set up functions
    void SetupModule() override {
      AddRequiredTrait<OrgPosition>(org_pos_trait);
      AddRequiredTrait<org_t::genome_t>(offspring_genome_trait);
      SetupFuncs();
    }

    /// Add the instruction specified by the config file
    void SetupFuncs(){
      ActionMap& action_map = control.GetActionMap(pop_id);
      { // ReproIntra
        const inst_func_t func_repro_intra = 
          [this](org_base_t& hw, const org_t::inst_t& inst){ Inst_ReproIntra(hw, inst); };
        Action& action = action_map.AddFunc<void, org_base_t&, const org_t::inst_t&>(
            "ReproIntra", func_repro_intra);
        action.data.AddVar<bool>("is_non_speculative", true);
      }
      { // ReproInter
        const inst_func_t func_repro_inter = 
          [this](org_base_t& hw, const org_t::inst_t& inst){ Inst_ReproInter(hw, inst); };
        Action& action = action_map.AddFunc<void, org_base_t&, const org_t::inst_t&>(
            "ReproInter", func_repro_inter);
        action.data.AddVar<bool>("is_non_speculative", true);
      }
    }

  };

  MABE_REGISTER_MODULE(MultiVirtualCPU_Inst_Replication, "Replication instructions for VirtualCPUOrg");
}

#endif

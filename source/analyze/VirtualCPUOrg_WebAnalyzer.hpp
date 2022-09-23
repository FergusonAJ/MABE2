/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2022-2022.
 *
 *  @file  VirtualCPUWebAnalyzer.h
 *  @brief Web-based module that visualizes the execution of VirtualCPUOrgs 
 **/

#ifndef MABE_VCPUORG_WEB_ANALYZER_H
#define MABE_VCPUORG_WEB_ANALYZER_H

// MABE includes
#include "../core/MABEWebController.hpp"
#include "../orgs/VirtualCPUOrg.hpp"

// Empirical includes
#include "emp/web/web.hpp"

namespace mabe {

  /// Rations out updates to organisms based on a specified attribute, using a method akin to roulette selection  
  class VirtualCPUOrg_WebAnalyzer : public Module {
  public: 
    using org_t = emp::VirtualCPU<VirtualCPUOrg>;
  protected:
    int pop_id = 0;     ///< Which population are we selecting from?
    emp::Ptr<MABEWebController> web_control_ptr = nullptr;
    emp::web::Div ip_div;
    bool is_web_enabled = false;
  public:
    VirtualCPUOrg_WebAnalyzer(mabe::MABE & control,
                     const std::string & name="VirtualCPUOrg_WebAnalyzer",
                     const std::string & desc="Visualizes VirtualCPUOrg execution on the web")
      : Module(control, name, desc)
    {
    }
    ~VirtualCPUOrg_WebAnalyzer() { }

    /// Set up variables for configuration file
    void SetupConfig() override {
      LinkPop(pop_id, "pop", "Which population should we select parents from?");
    }

    /// Register traits
    void SetupModule() override {
    }

    void WebInit() override {
      is_web_enabled = true;
      emp::Ptr<MABE> ptr = emp::Ptr<MABE>(&control);
      web_control_ptr = ptr.DynamicCast<MABEWebController>();
      ip_div = emp::web::Div("vcpu_analyzer_ip");
      web_control_ptr->GetDocument() << ip_div;
    }

    void Render(){
      Population & pop = control.GetPopulation(pop_id);
      VirtualCPUOrg& org = *dynamic_cast<VirtualCPUOrg*>(&pop[0]);
      ip_div.Clear();
      ip_div << "IP: " << org.inst_ptr << "<br/>"; 
      ip_div << "RH: " << org.read_head << "<br/>"; 
      ip_div << "WH: " << org.write_head << "<br/>"; 
      ip_div << "FH: " << org.flow_head << "<br/>"; 
      for(size_t reg_idx = 0; reg_idx < org.GetNumRegs(); ++reg_idx){
        ip_div << "Reg " << reg_idx << ": " << org.regs[reg_idx] << "<br/>";
      } 
      std::stringstream sstr;
      for(size_t inst_idx = 0; inst_idx < org.GetGenomeSize(); ++inst_idx){
        sstr.str("");
        sstr << "inst_select_" << inst_idx;
        emp::web::Selector inst_selector = emp::web::Selector(sstr.str());
        for(size_t new_inst_id = 0; new_inst_id < org.GetInstLib().GetSize(); ++new_inst_id){
          size_t new_inst_idx = org.GetInstLib().GetIndex(new_inst_id);
          inst_selector.SetOption(org.GetInstLib().GetName(new_inst_idx), 
                [this, &org, new_inst_id, new_inst_idx, inst_idx](){
                  org.SetInst(inst_idx, org_t::inst_t(new_inst_idx, new_inst_id));
                  org.ResetWorkingGenome();
                  org.ResetHardware();
                  Render();
                }, 
                new_inst_id);
        }
        inst_selector.SelectID(org.genome[inst_idx].id);
        ip_div << "[" << inst_idx << "] " 
               << inst_selector;
        if(inst_idx == org.inst_ptr) ip_div << " (IP)"; 
        if(inst_idx == org.read_head) ip_div << " (RH)"; 
        if(inst_idx == org.write_head) ip_div << " (WH)"; 
        if(inst_idx == org.flow_head) ip_div << " (FH)"; 
        ip_div << "<br/>"; 
      }
    }

    double DoStep() {
      Population & pop = control.GetPopulation(pop_id);
      pop[0].ProcessStep();
      Render();
      return 0;
    }
    /// Set up member functions associated with this class.
    static void InitType(emplode::TypeInfo & info) {
      info.AddMemberFunction(
          "STEP",
          [](VirtualCPUOrg_WebAnalyzer & mod) {
            if(mod.IsWebEnabled()) return mod.DoStep();
            return 0.0;
          },
          "Perform one round of scheduling");
    }
    bool IsWebEnabled() { return is_web_enabled; }
  };

  MABE_REGISTER_MODULE(VirtualCPUOrg_WebAnalyzer, "Rations out updates to organisms based on a specified attribute, using a method akin to roulette wheel selection.");
}

#endif

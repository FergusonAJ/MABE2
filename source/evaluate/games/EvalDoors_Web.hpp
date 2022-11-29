/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2022-2022.
 *
 *  @file  EvalDoors_Web.hpp
 *  @brief Wraps EvalDoors to provide relevant stats of an organism through a web interface 
 *
 */

#ifndef MABE_EVAL_DOORS_WEB_HPP
#define MABE_EVAL_DOORS_WEB_HPP

#include "./EvalDoors.hpp"
#include "../../core/MABEWebController.hpp"

#include "emp/web/web.hpp"

namespace mabe {
    
  /// \brief MABE module that displays an org's "doors" info via a web interface
  class EvalDoors_Web : public EvalDoors {
  public:
    using base_t = EvalDoors;
    using data_t = uint32_t;
    using org_t = VirtualCPUOrg;
    using inst_func_t = org_t::inst_func_t;

  protected:
    emp::Ptr<MABEWebController> web_control_ptr = nullptr;
    emp::web::Div div;
    bool is_web_enabled = false;
    std::string div_id = "";
    std::string div_class = "";

  public:
    EvalDoors_Web(mabe::MABE & control,
                const std::string & name="EvalDoors_Web",
                const std::string & desc="Displays an org's 'doors' info via the web")
      : base_t(control, name, desc)
    {
    }
    ~EvalDoors_Web() { }

    void AssignDiv(const std::string& id){
      div_id = id;
    }

    void WebInit() override {
      is_web_enabled = true;
      emp::Ptr<MABE> ptr = emp::Ptr<MABE>(&control);
      web_control_ptr = ptr.DynamicCast<MABEWebController>();
      if(div_id.size() == 0){
        div = emp::web::Div("eval_doors_web");
        div.SetAttr("class", "");
        web_control_ptr->GetDocument() << div;
      }
      else div = web_control_ptr->GetDocument().Div(div_id);
      div_class = div.GetAttr("class");
    }
    
    double Render(){
      Population & pop = control.GetPopulation(pop_id);
      VirtualCPUOrg& org = *dynamic_cast<VirtualCPUOrg*>(&pop[0]);
      DoorsState& state = org.GetTrait<DoorsState>(trait_names.state_trait);
      div.Clear();
      div.SetAttr("class", div_class);
      if(!state.initialized){
        div << "State of EvalDoors has not been initialized for this organism.<br/>";
      }
      else{
        div << "Current cue: " << state.current_cue;
        for(size_t cue_idx = 0; cue_idx < state.cue_vec.size(); ++cue_idx){
          if(state.current_cue == state.cue_vec[cue_idx]){
            div << " (Door #" << cue_idx << ")";
          }
        } 
        div << "<br/>";
      }
      return 0;
    }

    /// Set up member functions associated with this class.
    static void InitType(emplode::TypeInfo & info) {
      info.AddMemberFunction(
          "RENDER",
          [](EvalDoors_Web & mod) {
            if(mod.IsWebEnabled()) return mod.Render();
            return 0.0;
          },
          "Perform one round of scheduling");
      
      info.AddMemberFunction(
          "ASSIGN_DIV",
          [](EvalDoors_Web & mod, const std::string& div_id) {
            mod.AssignDiv(div_id);
            return 0.0;
          },
          "Assign module to use an existing div");
    }
    bool IsWebEnabled() { return is_web_enabled; }

  };
  MABE_REGISTER_MODULE(EvalDoors_Web, 
      "Displays an organism's 'doors' info through a web interface");
}

#endif

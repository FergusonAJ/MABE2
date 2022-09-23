/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2022-2022.
 *
 *  @file  MABEWebController.hpp
 *  @brief A derived version of MABE.hpp that provides additional support for building on 
 *    the web 
 */

#ifndef MABE_MABE_WEB_CONTROLLER_HPP
#define MABE_MABE_WEB_CONTROLLER_HPP

// MABE includes
#include "MABE.hpp"
#include "EmptyOrganism.hpp"

// Empirical includes
#include "emp/web/web.hpp"

namespace mabe{
  class MABEWebController : public MABE, emp::web::Animate{
    protected:
    emp::web::Document doc;   // Div that contains all our web elements
    emp::web::TextArea config_input;
    size_t steps_per_draw = 1;

    void LoadConfig(){
      std::string input_str = config_input.GetText();
      std::stringstream sstr;
      sstr << input_str;
      Load(sstr, "Web input");
      // Setup all of the modules
      if (Setup() == false){
        std::cout << "Error! Setup() failed on MABE object!" << std::endl;
      }
      // Print to the console to make sure MABE object was initialized correctly
      Execute("PRINT(\"Main MABE control object created and setup!\");");
    }

    void InitializeModules(){
      for(auto mod_ptr : modules){
        mod_ptr->WebInit();
      }
    }

    void SetupWebpage(){
      config_input = emp::web::TextArea("config_input");
      config_input.SetCSS("width", "50%");
      config_input.SetCSS("height", "400px");
      doc << config_input;
      doc << "<br/>";
      doc << emp::web::Button([this](){
          LoadConfig();
          InitializeModules();
        }, "Load config", "config_load_button");
      doc << emp::web::Button([this](){
          Start();
        }, "Run", "run_button");
      doc << emp::web::Button([this](){
          Stop();
        }, "Pause", "pause_button");
      doc << GetStepButton("anim_step_btn", "Advance Step");
      // Allow user to change the number of attempted cell births per render update
      auto steps_input = emp::web::Input(
          [this](const std::string & s){
            steps_per_draw = std::stoi(s);
          },
          "number", "", "steps_input"
      );
      doc << steps_input;
      doc.Input("steps_input").Value(steps_per_draw);
    }

    void DoFrame() override{
      Update(steps_per_draw);
    }

    public:
    /// Command-line constructor
    MABEWebController(int argc, char* argv[]): MABE(argc, argv), doc("emp_base"){
      SetupEmpty<mabe::EmptyOrganismManager>();
      SetupWebpage();
    }
    MABEWebController(const MABE &) = delete;
    MABEWebController(MABE &&) = delete;
    ~MABEWebController() { ; }
    
    virtual bool IsWeb() override { return true; }

    emp::web::Document& GetDocument(){ return doc; }
  };
}

#endif

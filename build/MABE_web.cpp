/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2020.
 *
 *  @file  MABE.cpp
 *  @brief Dynamic MABE executable that can be fully configured from a file.
 */

#include <iostream>

// Empirical tools
#include "emp/bits/BitVector.hpp"
#include "emp/config/ArgManager.hpp"
#include "emp/math/Random.hpp"
#include "emp/web/web.hpp"

// Framework
#include "../source/core/MABE.hpp"
#include "../source/core/EmptyOrganism.hpp"

// Include the full set of available modules.
#include "../source/modules.hpp"

class MABEWebController : public emp::web::Animate{
  private:
  emp::web::Document doc;   // Div that contains all our web elements
  mabe::MABE control;
  emp::web::TextArea config_input;

  void SetupWebpage(){
    config_input = emp::web::TextArea("config_input");
    doc << config_input;
    doc << "<br/>";
    doc << emp::web::Button([this](){
        std::string input_str = config_input.GetText();
        std::stringstream sstr;
        sstr << input_str;
        control.Load(sstr, "Web input");
        // Setup all of the modules
        if (control.Setup() == false){
          std::cout << "Error! Setup() failed on MABE object!" << std::endl;
        }
        // Print to the console to make sure MABE object was initialized correctly
        control.Execute("PRINT(\"Main MABE control object created and setup!\");");
      }, "Load config", "config_load_button");
    doc << emp::web::Button([this](){
        control.Update(1000000);
      }, "Run", "run_button");
  }

  public:
  MABEWebController(): doc("emp_base"), control(0, NULL){
    control.SetupEmpty<mabe::EmptyOrganismManager>();
    SetupWebpage();
  }

};

MABEWebController controller;
int main(){
}

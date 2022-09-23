/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2022-2022.
 *
 *  @file  WebPlot.hpp
 *  @brief Web-based module that visualizes data as a line plot 
 **/

#ifndef MABE_WEB_PLOT_H
#define MABE_WEB_PLOT_H

// STD includes
#include <utility>

// MABE includes
#include "../core/MABEWebController.hpp"

#ifdef __EMSCRIPTEN__
// Empirical includes
#include "emp/web/web.hpp"
#include "emp/web/d3/visualizations.hpp"
#endif

namespace mabe {

  /// Rations out updates to organisms based on a specified attribute, using a method akin to roulette selection  
  class WebPlot : public Module {
  protected:
#ifdef __EMSCRIPTEN__
    emp::Ptr<MABEWebController> web_control_ptr = nullptr;
    emp::web::Div div;
    bool is_web_enabled = false;
    emp::vector<std::pair<double, double>> coordinate_vec;
    emp::web::LineGraph<emp::array<double, 2>, D3::LinearScale, D3::LinearScale> plot;
    std::string x_axis_label = "x";
    std::string y_axis_label = "y";
    size_t id = 0;
#endif
  public:
    WebPlot(mabe::MABE & control,
                     const std::string & name="WebPlot",
                     const std::string & desc="Web-based module that creates a line plot")
      : Module(control, name, desc)
    {
    }
    ~WebPlot() { }

#ifdef __EMSCRIPTEN__
    /// Set up variables for configuration file
    void SetupConfig() override {
      LinkVar(x_axis_label, "x_axis_label", "Label for the x axis");
      LinkVar(y_axis_label, "y_axis_label", "Label for the y axis");
    }

    /// Register traits
    void SetupModule() override {
    }

    void WebInit() override {
      is_web_enabled = true;
      emp::Ptr<MABE> ptr = emp::Ptr<MABE>(&control);
      web_control_ptr = ptr.DynamicCast<MABEWebController>();
      std::stringstream sstr;
      sstr << "mabe_web_plot_" << id;
      while(web_control_ptr->GetDocument().HasChild(sstr.str())){
        id++;
        sstr.str("");
        sstr << "mabe_web_plot_" << id;
      }
      div = emp::web::Div(sstr.str());
      web_control_ptr->GetDocument() << div;
      plot.variables[0] = x_axis_label;
      plot.variables[1] = y_axis_label;
      web_control_ptr->GetDocument() << plot;
    }

    void Render(){
      if(is_web_enabled){
        div.Clear();
        div << "Points: " << coordinate_vec.size() << "<br/>"; 
      }
    }

    double AddPoint(double x, double y) {
      coordinate_vec.push_back(std::make_pair(x, y));
      plot.AddDataPoint({x, y});
      Render();
      return 0;
    }
    /// Set up member functions associated with this class.
    static void InitType(emplode::TypeInfo & info) {
      info.AddMemberFunction(
          "ADD_POINT",
          [](WebPlot & mod, double x, double y) {
            if(mod.IsWebEnabled()) return mod.AddPoint(x, y);
            return 0.0;
          },
          "Add a new point to the plot");
    }
    bool IsWebEnabled() { return is_web_enabled; }
#endif
  };

  MABE_REGISTER_MODULE(WebPlot, "Creates a line plot on the web");
}

#endif

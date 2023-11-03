#pragma once

/// Interface for adding additional GUI modules of SoC

#include <wx/frame.h>

namespace lime {
class SDRDevice;
}

class IModuleFrame : public wxFrame
{
  public:
    //IModuleFrame() : mDevice(nullptr) {};
    IModuleFrame(wxWindow* parent, wxWindowID id)
        : wxFrame(parent, id, "Default")
        , mDevice(nullptr){};
    IModuleFrame(wxWindow* parent,
        wxWindowID id,
        const wxString& title,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long styles = 0)
        : wxFrame(parent, id, title, pos, size, styles)
        , mDevice(nullptr){};
    virtual ~IModuleFrame(){};

    virtual bool Initialize(lime::SDRDevice* device) = 0;
    virtual void Update() = 0;

  protected:
    lime::SDRDevice* mDevice;
};

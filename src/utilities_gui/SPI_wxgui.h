#ifndef __SPI_wxgui__
#define __SPI_wxgui__

#include "utilities_gui.h"
#include "IModuleFrame.h"
#include <vector>
#include <unordered_map>

namespace lime {
class SDRDevice;
}

#include <wx/combobox.h>

class wxTextCtrl;
class wxChoice;
class wxStaticText;

class SPI_wxgui : public IModuleFrame
{
    protected:
      struct SPIFields
      {
          wxChoice *devSelection;
          wxTextCtrl *address;
          wxTextCtrl *value;
          wxStaticText *status;
      };

      // Handlers for SPI_view events.
      void onSPIwrite(wxCommandEvent &event);
      void onSPIread(wxCommandEvent &event);

      wxFlexGridSizer *CreateSPIControls(wxWindow *parent, uint8_t rowCount,
                                         wxChoice *outDeviceSelector);
      void InsertSPIControlsRow(wxWindow *parent, wxWindowID id, wxFlexGridSizer *row,
                                SPI_wxgui::SPIFields *controls);

    public:
      SPI_wxgui(wxWindow *parent, wxWindowID id = wxID_ANY, const wxString &title = wxEmptyString,
                const wxPoint &pos = wxDefaultPosition, const wxSize &size = wxDefaultSize,
                long styles = wxDEFAULT_DIALOG_STYLE);
      virtual bool Initialize(lime::SDRDevice *device);
      virtual void Update(){};

    protected:
      std::vector<wxChoice *> mSPIselection;
      std::unordered_map<wxWindowID, SPIFields> mSPIElements;
};

#endif // __SPI_wxgui__

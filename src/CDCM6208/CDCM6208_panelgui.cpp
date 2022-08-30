#include "CDCM6208_panelgui.h"
#include "lms7_device.h"
#include "Logger.h"

CDCM6208_panelgui::CDCM6208_panelgui( wxWindow* parent )
:
CDCM_controlpanel( parent )
{

}

CDCM6208_panelgui::CDCM6208_panelgui( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style)
:
CDCM_controlpanel(parent, id, pos, size, style)
{

}

CDCM6208_panelgui::~CDCM6208_panelgui()
{
}

void CDCM6208_panelgui::Initialize(CDCM_Dev* cdcm)
{
   this->CDCM = cdcm;
   
   UpdateGUI();
   
   m_Baseaddr->SetValue(wxString::Format(_("%i"),CDCM->GetSPIBaseAddr()));
}

void CDCM6208_panelgui::OnChange( wxCommandEvent& event )
{
   auto obj = event.GetEventObject();
   std::string str;
   double d_val;
   int i_val;
   CDCM_VCO VCO = CDCM->GetVCO();
   CDCM_Outputs Outputs = CDCM->getOutputs();

   try{
      if (obj == m_PrimaryFreq)
      {
         str = m_PrimaryFreq->GetValue();
         VCO.prim_freq = std::stod(str)*1e6;
      }
      else if (obj == m_SecondaryFreq)
      {
         str = m_SecondaryFreq->GetValue();
         VCO.sec_freq = std::stod(str)*1e6;
      }
      else if (obj == m_RDivider)
      {
         str = m_RDivider->GetValue();
         i_val = std::stoi(str);

         if(i_val < 1)
            i_val = 1;
         else if(i_val > 16)
            i_val = 16;
         
         VCO.R_div = i_val;
      }
      else if (obj == m_MDivider)
      {
         str = m_MDivider->GetValue();
         i_val = std::stoi(str);
         int max_val = (1<<14); //max 14bit value + 1

         if(i_val < 1)
            i_val = 1;
         else if(i_val > max_val)
            i_val = max_val;
         
         VCO.M_div = i_val;
      }
      else if (obj == m_NMultiplier)
      {
         str = m_NMultiplier->GetValue();
         i_val = std::stoi(str);
         //N consists of two multipliers 8 bit and 10 bit
         int max_val = (1<<10) * (1<<8);
         if(i_val < 1)
            i_val = 1;
         else if(i_val > max_val)
            i_val = max_val;
         
         CDCM->SetVCOMultiplier(i_val);
         VCO = CDCM->GetVCO();
      }
      else if (obj == m_Y0Y1_DIV)
      {
         str = m_Y0Y1_DIV->GetValue();
         i_val = std::stoi(str);
         int max_val = 1<<8;
         if(i_val < 1)
            i_val = 1;
         else if(i_val > max_val)
            i_val = max_val;
         
         Outputs.Y0Y1.divider_val = i_val;
      }
      else if (obj == m_Y2Y3_DIV)
      {
         str = m_Y2Y3_DIV->GetValue();
         i_val = std::stoi(str);
         int max_val = 1<<8;
         if(i_val < 1)
            i_val = 1;
         else if(i_val > max_val)
            i_val = max_val;
         Outputs.Y2Y3.divider_val = i_val;
      }
      else if (obj == m_Y4_DIV)
      {
         str = m_Y4_DIV->GetValue();
         d_val = std::stod(str);
         Outputs.Y4.divider_val = CDCM->SolveFracDiv(d_val, &Outputs.Y4);
      }
      else if (obj == m_Y5_DIV)
      {
         str = m_Y5_DIV->GetValue();
         d_val = std::stod(str);
         Outputs.Y5.divider_val = CDCM->SolveFracDiv(d_val, &Outputs.Y5);
      }
      else if (obj == m_Y6_DIV)
      {
         str = m_Y6_DIV->GetValue();
         d_val = std::stod(str);
         Outputs.Y6.divider_val = CDCM->SolveFracDiv(d_val, &Outputs.Y6);
      }
      else if (obj == m_Y7_DIV)
      {
         str = m_Y7_DIV->GetValue();
         d_val = std::stod(str);
         Outputs.Y7.divider_val = CDCM->SolveFracDiv(d_val, &Outputs.Y7);
      }
      else if (obj == m_Baseaddr)
      {
         str = m_Baseaddr->GetValue();
         i_val = std::stoi(str);
         CDCM->SetSPIBaseAddr(i_val);
      }
   }
   catch(std::invalid_argument)
   {
      return;
   }

   CDCM->SetVCO(VCO);
   CDCM->SetOutputs(Outputs);
   CDCM->UpdateOutputFrequencies();
   UpdateGUI();
}

void CDCM6208_panelgui::OnRadio( wxCommandEvent& event )
{
   bool PrimarySel;
   bool SecondarySel;
   PrimarySel = m_PrimarySel->GetValue();
   SecondarySel = m_SecondarySel->GetValue();

   if(PrimarySel && !SecondarySel)
      CDCM->SetVCOInput(1);
   else if(!PrimarySel && SecondarySel)
      CDCM->SetVCOInput(2);
   else
      CDCM->SetVCOInput(1);
   
   //Recalculate();
   UpdateGUI();
}

void CDCM6208_panelgui::OnChoice( wxCommandEvent& event )
{
   auto obj = event.GetEventObject();

   if (obj == m_PSA)
   {
      CDCM_VCO VCO = CDCM->GetVCO();
      VCO.prescaler_A = m_PSA->GetSelection() + 4;
      CDCM->SetVCO(VCO);
   }
   else if(obj == m_PSB)
   {
      CDCM_VCO VCO = CDCM->GetVCO();
      VCO.prescaler_B = m_PSB->GetSelection() + 4;
      CDCM->SetVCO(VCO);
   }
   else if(obj == m_CDCM_VER)
   {
      CDCM->SetVersion(m_CDCM_VER->GetSelection());
   }

   //Recalculate();
   UpdateGUI();
}

void CDCM6208_panelgui::OnFreqEntry( wxCommandEvent& event )
{
   try
   {
      int return_val = -1;

      CDCM_Outputs Outputs = CDCM->getOutputs();
      if((bool)m_Y0Y1_chk->GetValue())
         Outputs.Y0Y1.requested_freq = std::stod((std::string)m_Y0Y1_FREQ_req->GetValue())*1e6;
      if((bool)m_Y2Y3_chk->GetValue())
         Outputs.Y2Y3.requested_freq = std::stod((std::string)m_Y2Y3_FREQ_req->GetValue())*1e6;
      if((bool)m_Y4_chk->GetValue())
         Outputs.Y4.requested_freq = std::stod((std::string)m_Y4_FREQ_req->GetValue())*1e6;
      if((bool)m_Y5_chk->GetValue())
         Outputs.Y5.requested_freq = std::stod((std::string)m_Y5_FREQ_req->GetValue())*1e6;
      if((bool)m_Y6_chk->GetValue())
         Outputs.Y6.requested_freq = std::stod((std::string)m_Y6_FREQ_req->GetValue())*1e6;
      if((bool)m_Y7_chk->GetValue())
         Outputs.Y7.requested_freq = std::stod((std::string)m_Y7_FREQ_req->GetValue())*1e6;

      CDCM->SetOutputs(Outputs);
      return_val = CDCM->RecalculateFrequencies();

      if (return_val == 0)
      {
         m_FrequencyPlanRes->SetLabel("Valid Config Found");
         m_FrequencyPlanRes->SetForegroundColour(wxColour("#00ff00"));
      }
      else
      {
         m_FrequencyPlanRes->SetLabel("No Valid Config Found");
         m_FrequencyPlanRes->SetForegroundColour(wxColour("#ff0000"));
      }

      UpdateGUI();
   }
   catch(std::invalid_argument)
   {
      m_FrequencyPlanRes->SetLabel("Invalid Freq Request");
      m_FrequencyPlanRes->SetForegroundColour(wxColour("#ff0000"));
   }
}

void CDCM6208_panelgui::onFP_chk( wxCommandEvent& event )
{
   CDCM_Outputs Outputs = CDCM->getOutputs();

   Outputs.Y0Y1.used = (bool)m_Y0Y1_chk->GetValue();
   Outputs.Y2Y3.used = (bool)m_Y2Y3_chk->GetValue();
   Outputs.Y4.used = (bool)m_Y4_chk->GetValue();
   Outputs.Y5.used = (bool)m_Y5_chk->GetValue();
   Outputs.Y6.used = (bool)m_Y6_chk->GetValue();
   Outputs.Y7.used = (bool)m_Y7_chk->GetValue();
   CDCM->SetOutputs(Outputs);
   CDCM->UpdateOutputFrequencies();

   UpdateGUI();
}

void CDCM6208_panelgui::OnButton( wxCommandEvent& event )
{
   auto obj = event.GetEventObject();

   if(obj == m_WriteAll)
      CDCM->UploadConfiguration();
   else if(obj == m_ReadAll)
      CDCM->DownloadConfiguration();   
   else if(obj == m_Reset)
      CDCM->Reset(30.72e6, 25e6);
   //Recalculate();
   UpdateGUI();
}

void CDCM6208_panelgui::UpdateGUI()
{
   CDCM_VCO VCO = CDCM->GetVCO();
   CDCM_Outputs Outputs = CDCM->getOutputs();
   //Primary input
   auto str = wxString::Format(_("%.3f"), VCO.prim_freq/1e6);
   m_PrimaryFreq->SetValue(str);
   
   //Secondary Input
   str = wxString::Format(_("%.3f"), VCO.sec_freq/1e6);
   m_SecondaryFreq->SetValue(str);
   
   //R Divider
   str = wxString::Format(_("%i"), VCO.R_div);
   m_RDivider->SetValue(str);
   
   //MUX Selection
   if (VCO.input_mux == 2)
   {
      m_PrimarySel->SetValue(false);
      m_SecondarySel->SetValue(true);
   }
   else if (VCO.input_mux == 1)
   {
      m_PrimarySel->SetValue(true);
      m_SecondarySel->SetValue(false);
   }

   //M Divider
   str = wxString::Format(_("%i"),VCO.M_div);
   m_MDivider->SetValue(str);
   
   //N Multiplier
   str = wxString::Format(_("%i"),VCO.N_mul_full);
   m_NMultiplier->SetValue(str);
   
   //VCO prescalers
   m_PSA->SetSelection(VCO.prescaler_A-4);
   m_PSB->SetSelection(VCO.prescaler_B-4);

   
   //CDCM Version
   m_CDCM_VER->SetSelection(CDCM->GetVersion());
   
   //VCO Range
   m_VCORANGE->SetLabel(wxString::Format(_("%.0f - %.0f"), VCO.min_freq/1e6, VCO.max_freq/1e6));
   
   //VCO Frequency
   m_VCOFREQ->SetLabel(wxString::Format(_("%.2f MHz"), VCO.output_freq/1e6));
   if(VCO.output_freq > VCO.max_freq || VCO.output_freq < VCO.min_freq)
      m_VCOFREQ->SetForegroundColour(wxColour("#ff0000"));
   else
      m_VCOFREQ->SetForegroundColour(wxColour("#000000"));
   
   //Y0Y1 divider
   str = wxString::Format(_("%i"),(int)Outputs.Y0Y1.divider_val);
   m_Y0Y1_DIV->SetValue(str);
   //Y0Y1 Frequency
   str = wxString::Format(_("%f"),Outputs.Y0Y1.output_freq/1e6);
   m_Y0Y1_FREQ->SetValue(str);

   str = wxString::Format(_("%f"),Outputs.Y0Y1.requested_freq/1e6);
   m_Y0Y1_FREQ_req->SetValue(str);
   
   //Y2Y3 divider
   str = wxString::Format(_("%i"),(int)Outputs.Y2Y3.divider_val);
   m_Y2Y3_DIV->SetValue(str);
   //Y2Y3 Frequency
   str = wxString::Format(_("%f"),Outputs.Y2Y3.output_freq/1e6);
   m_Y2Y3_FREQ->SetValue(str);

   str = wxString::Format(_("%f"),Outputs.Y2Y3.requested_freq/1e6);
   m_Y2Y3_FREQ_req->SetValue(str);
   
   //Y4 divider
   str = wxString::Format(_("%f"),Outputs.Y4.divider_val);
   m_Y4_DIV->SetValue(str);
   
   //Y4 Frequency
   str = wxString::Format(_("%f"),Outputs.Y4.output_freq/1e6);
   m_Y4_FREQ->SetValue(str);

   str = wxString::Format(_("%f"),Outputs.Y4.requested_freq/1e6);
   m_Y4_FREQ_req->SetValue(str);
   
   //Y5 divider
   str = wxString::Format(_("%f"),Outputs.Y5.divider_val);
   m_Y5_DIV->SetValue(str);
   //Y5 Frequency
   str = wxString::Format(_("%f"),Outputs.Y5.output_freq/1e6);
   m_Y5_FREQ->SetValue(str);

   str = wxString::Format(_("%f"),Outputs.Y5.requested_freq/1e6);
   m_Y5_FREQ_req->SetValue(str);

   //Y6 divider
   str = wxString::Format(_("%f"),Outputs.Y6.divider_val);
   m_Y6_DIV->SetValue(str);
   //Y6 Frequency
   str = wxString::Format(_("%f"),Outputs.Y6.output_freq/1e6);
   m_Y6_FREQ->SetValue(str);

   str = wxString::Format(_("%f"),Outputs.Y6.requested_freq/1e6);
   m_Y6_FREQ_req->SetValue(str);
   
   //Y7 divider
   str = wxString::Format(_("%f"),Outputs.Y7.divider_val);
   m_Y7_DIV->SetValue(str);
   //Y7 Frequency
   str = wxString::Format(_("%f"),Outputs.Y7.output_freq/1e6);
   m_Y7_FREQ->SetValue(str);

   str = wxString::Format(_("%f"),Outputs.Y7.requested_freq/1e6);
   m_Y7_FREQ_req->SetValue(str);

   //PLL Lock status
   if(CDCM->IsLocked())
   {
      m_LockStatus->SetLabel("PLL LOCKED");
      m_LockStatus->SetForegroundColour(wxColour("#00ff00"));
   }
   else
   {
      m_LockStatus->SetLabel("NOT LOCKED");
      m_LockStatus->SetForegroundColour(wxColour("#ff0000"));
   }

   m_Y0Y1_FREQ_req->Enable(Outputs.Y0Y1.used);
   m_Y2Y3_FREQ_req->Enable(Outputs.Y2Y3.used);
   m_Y4_FREQ_req->Enable(Outputs.Y4.used);
   m_Y5_FREQ_req->Enable(Outputs.Y5.used);
   m_Y6_FREQ_req->Enable(Outputs.Y6.used);
   m_Y7_FREQ_req->Enable(Outputs.Y7.used);

   m_Y0Y1_chk->SetValue(Outputs.Y0Y1.used);
   m_Y2Y3_chk->SetValue(Outputs.Y2Y3.used);
   m_Y4_chk->SetValue(Outputs.Y4.used);
   m_Y5_chk->SetValue(Outputs.Y5.used);
   m_Y6_chk->SetValue(Outputs.Y6.used);
   m_Y7_chk->SetValue(Outputs.Y7.used);
}

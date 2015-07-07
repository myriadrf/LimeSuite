#include "lms7002_pnlSPI_view.h"
#include "LMS7002M.h"
#include "lmsComms.h"
#include <assert.h>

lms7002_pnlSPI_view::lms7002_pnlSPI_view(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style)
    : pnlSPI_view(parent, id, pos, size, style), ctrPort(nullptr), dataPort(nullptr)
{

}

void lms7002_pnlSPI_view::Initialize(LMScomms* ctr, LMScomms* data)
{
    ctrPort = ctr;
    dataPort = data;
}

void lms7002_pnlSPI_view::onLMSwrite( wxCommandEvent& event )
{
    wxString address = txtLMSwriteAddr->GetValue();
    long addr = 0;    
    address.ToLong(&addr, 16);
    wxString value = txtLMSwriteValue->GetValue();
    long data = 0;
    value.ToLong(&data, 16);

    LMScomms::GenericPacket pkt;
    pkt.cmd = CMD_LMS7002_WR;
    pkt.outBuffer[0] = (addr >> 8) & 0xFF;
    pkt.outBuffer[1] = addr & 0xFF;
    pkt.outBuffer[2] = (data >> 8) & 0xFF;
    pkt.outBuffer[3] = data & 0xFF;
    pkt.outLen = 4;

    assert(ctrPort != nullptr);
    LMScomms::TransferStatus status = ctrPort->TransferPacket(pkt);

    if (status == LMScomms::TRANSFER_SUCCESS)    
        lblLMSwriteStatus->SetLabel(wxString::FromUTF8(status2string(pkt.status)));
    else
        lblLMSwriteStatus->SetLabel(_("Write failed"));
}

void lms7002_pnlSPI_view::onLMSread( wxCommandEvent& event )
{
    wxString address = txtLMSreadAddr->GetValue();
    long addr = 0;
    address.ToLong(&addr, 16);    

    LMScomms::GenericPacket pkt;
    pkt.cmd = CMD_LMS7002_RD;
    pkt.outBuffer[0] = (addr >> 8) & 0xFF;
    pkt.outBuffer[1] = addr & 0xFF;
    pkt.outLen = 2;
    assert(ctrPort != nullptr);
    LMScomms::TransferStatus status = ctrPort->TransferPacket(pkt);

    if (status == LMScomms::TRANSFER_SUCCESS)
    {
        lblLMSreadStatus->SetLabel(wxString::FromUTF8(status2string(pkt.status)));
        unsigned short value = (pkt.inBuffer[2] * 256) | pkt.inBuffer[3];
        lblLMSreadValue->SetLabel(wxString::Format(_("0x%04X"), value));
    }
    else
        lblLMSreadStatus->SetLabel(_("Read failed"));
}

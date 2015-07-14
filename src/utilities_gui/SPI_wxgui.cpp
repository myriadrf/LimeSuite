#include "SPI_wxgui.h"
#include "lmsComms.h"

SPI_wxgui::SPI_wxgui(wxWindow* parent, wxWindowID id, const wxString &title, const wxPoint& pos, const wxSize& size, long styles)
:
SPI_view( parent )
{
    ctrPort = nullptr;
    dataPort = nullptr;
}

void SPI_wxgui::Initialize(LMScomms* pCtrPort, LMScomms* pDataPort)
{
    ctrPort = pCtrPort;
    dataPort = pDataPort;
    assert(ctrPort != nullptr);
    assert(dataPort != nullptr);
}

void SPI_wxgui::onLMSwrite( wxCommandEvent& event )
{
    wxString address = txtLMSwriteAddr->GetValue();
    long addr = 0;
    address.ToLong(&addr, 16);
    wxString value = txtLMSwriteValue->GetValue();
    long data = 0;
    value.ToLong(&data, 16);

    LMScomms::GenericPacket pkt;
    pkt.cmd = CMD_LMS7002_WR;
    pkt.outBuffer.push_back((addr >> 8) & 0xFF);
    pkt.outBuffer.push_back(addr & 0xFF);
    pkt.outBuffer.push_back((data >> 8) & 0xFF);
    pkt.outBuffer.push_back(data & 0xFF);

    assert(ctrPort != nullptr);
    if (ctrPort == nullptr)
        return;
    LMScomms::TransferStatus status = ctrPort->TransferPacket(pkt);

    if (status == LMScomms::TRANSFER_SUCCESS)
        lblLMSwriteStatus->SetLabel(wxString::FromUTF8(status2string(pkt.status)));
    else
        lblLMSwriteStatus->SetLabel(_("Write failed"));
}

void SPI_wxgui::onLMSread( wxCommandEvent& event )
{
    wxString address = txtLMSreadAddr->GetValue();
    long addr = 0;
    address.ToLong(&addr, 16);

    LMScomms::GenericPacket pkt;
    pkt.cmd = CMD_LMS7002_RD;
    pkt.outBuffer.push_back((addr >> 8) & 0xFF);
    pkt.outBuffer.push_back(addr & 0xFF);
    assert(ctrPort != nullptr);
    if (ctrPort == nullptr)
        return;
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

void SPI_wxgui::onBoardWrite( wxCommandEvent& event )
{
    wxString address = txtBoardwriteAddr->GetValue();
    long addr = 0;
    address.ToLong(&addr, 16);
    wxString value = txtBoardwriteValue->GetValue();
    long data = 0;
    value.ToLong(&data, 16);

    LMScomms::GenericPacket pkt;
    pkt.cmd = CMD_BRDSPI_WR;
    pkt.outBuffer.push_back((addr >> 8) & 0xFF);
    pkt.outBuffer.push_back(addr & 0xFF);
    pkt.outBuffer.push_back((data >> 8) & 0xFF);
    pkt.outBuffer.push_back(data & 0xFF);

    assert(dataPort != nullptr);
    if (dataPort == nullptr)
        return;
    LMScomms::TransferStatus status = dataPort->TransferPacket(pkt);

    if (status == LMScomms::TRANSFER_SUCCESS)
        lblBoardwriteStatus->SetLabel(wxString::FromUTF8(status2string(pkt.status)));
    else
        lblBoardwriteStatus->SetLabel(_("Write failed"));
}

void SPI_wxgui::OnBoardRead( wxCommandEvent& event )
{
    wxString address = txtBoardreadAddr->GetValue();
    long addr = 0;
    address.ToLong(&addr, 16);

    LMScomms::GenericPacket pkt;
    pkt.cmd = CMD_BRDSPI_RD;
    pkt.outBuffer.push_back((addr >> 8) & 0xFF);
    pkt.outBuffer.push_back(addr & 0xFF);
    assert(dataPort != nullptr);
    if (dataPort == nullptr)
        return;
    LMScomms::TransferStatus status = dataPort->TransferPacket(pkt);

    if (status == LMScomms::TRANSFER_SUCCESS)
    {
        lblBoardreadStatus->SetLabel(wxString::FromUTF8(status2string(pkt.status)));
        unsigned short value = (pkt.inBuffer[2] * 256) | pkt.inBuffer[3];
        lblBoardreadValue->SetLabel(wxString::Format(_("0x%04X"), value));
    }
    else
        lblBoardreadStatus->SetLabel(_("Read failed"));
}

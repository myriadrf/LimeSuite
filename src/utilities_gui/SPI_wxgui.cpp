#include "SPI_wxgui.h"
#include "IConnection.h"

SPI_wxgui::SPI_wxgui(wxWindow* parent, wxWindowID id, const wxString &title, const wxPoint& pos, const wxSize& size, long styles)
:
SPI_view( parent )
{
    ctrPort = nullptr;
    dataPort = nullptr;
}

void SPI_wxgui::Initialize(IConnection* pCtrPort, IConnection* pDataPort, const size_t devIndex)
{
    ctrPort = pCtrPort;
    dataPort = pDataPort;
    if (ctrPort != nullptr)
    {
        m_rficSpiAddrCtr = ctrPort->GetDeviceInfo().addrsLMS7002M.at(devIndex);
    }
    if (dataPort != nullptr)
    {
        m_rficSpiAddrData = dataPort->GetDeviceInfo().addrsLMS7002M.at(devIndex);
    }
}

void SPI_wxgui::onLMSwrite( wxCommandEvent& event )
{
    wxString address = txtLMSwriteAddr->GetValue();
    long addr = 0;
    address.ToLong(&addr, 16);
    wxString value = txtLMSwriteValue->GetValue();
    long data = 0;
    value.ToLong(&data, 16);

    if (ctrPort == nullptr)
        return;
    uint32_t dataWr = (1 << 31);
    dataWr |= (addr & 0xFFFF) << 16;
    dataWr |=  data & 0xFFFF;
    OperationStatus status;
    status = ctrPort->TransactSPI(m_rficSpiAddrCtr, &dataWr, nullptr, 1);

    if (status == OperationStatus::SUCCESS)
        lblLMSwriteStatus->SetLabel(_("Write success"));
    else
        lblLMSwriteStatus->SetLabel(_("Write failed"));
}

void SPI_wxgui::onLMSread( wxCommandEvent& event )
{
    wxString address = txtLMSreadAddr->GetValue();
    long addr = 0;
    address.ToLong(&addr, 16);

    if (ctrPort == nullptr)
        return;

    const uint32_t dataWr = (addr & 0x7FFF) << 16;
    uint32_t dataRd = 0;
    OperationStatus status = ctrPort->TransactSPI(m_rficSpiAddrCtr, &dataWr, &dataRd, 1);

    if (status == OperationStatus::SUCCESS)
    {
        lblLMSreadStatus->SetLabel(_("Read success"));
        unsigned short value = dataRd & 0xFFFF;
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

    assert(dataPort != nullptr);
    if (dataPort == nullptr)
        return;

    uint32_t dataWr = (1 << 31);
    dataWr |= (addr & 0xFFFF) << 16;
    dataWr |=  data & 0xFFFF;
    OperationStatus status;
    status = dataPort->TransactSPI(m_rficSpiAddrData, &dataWr, nullptr, 1);

    if (status == OperationStatus::SUCCESS)
        lblBoardwriteStatus->SetLabel(_("Write success"));
    else
        lblBoardwriteStatus->SetLabel(_("Write failed"));
}

void SPI_wxgui::OnBoardRead( wxCommandEvent& event )
{
    wxString address = txtBoardreadAddr->GetValue();
    long addr = 0;
    address.ToLong(&addr, 16);

    assert(dataPort != nullptr);
    if (dataPort == nullptr)
        return;

    const uint32_t dataWr = (addr & 0x7FFF) << 16;
    uint32_t dataRd = 0;
    OperationStatus status = dataPort->TransactSPI(m_rficSpiAddrData, &dataWr, &dataRd, 1);

    if (status == OperationStatus::SUCCESS)
    {
        lblBoardreadStatus->SetLabel(_("Read success"));
        unsigned short value = dataRd & 0xFFFF;
        lblBoardreadValue->SetLabel(wxString::Format(_("0x%04X"), value));
    }
    else
        lblBoardreadStatus->SetLabel(_("Read failed"));
}

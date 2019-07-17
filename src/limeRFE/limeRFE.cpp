#include "wx/wxprec.h"
//#include "hamFEsuite.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif //__BORLANDC__

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif //WX_PRECOMP

#include "limeRFE.h"

limeRFE::limeRFE(wxWindow* parent, wxWindowID id, const wxString &title, const wxPoint& pos, const wxSize& size, long styles)
:
limeRFE_view( parent, id, title, pos, size, styles)
{
    lmsControl = nullptr;

	this->fd = -1;

	//	bmpHAMFEBlockDiagram = wxBITMAP_PNG_FROM_DATA(LIMERFE_block_diagram);

	cTX2TXRX->Clear();
	for (int i = 0; i < LIMERFE_TX2TXRX_INDEX_COUNT; i++) {
		if (i == LIMERFE_TX2TXRX_INDEX_TXRX) cTX2TXRX->AppendString("TX/RX Connector (J3)");
		if (i == LIMERFE_TX2TXRX_INDEX_TX) cTX2TXRX->AppendString("TX Connector (J4)");
	}
	cTX2TXRX->SetSelection(LIMERFE_TX2TXRX_INDEX_DEFAULT);
	lastTX2TXRXSelection = LIMERFE_TX2TXRX_INDEX_DEFAULT;

	cbNotch->SetValue(LIMERFE_NOTCH_DEFAULT);
	cTX2TXRX->SetSelection(LIMERFE_TX2TXRX_INDEX_DEFAULT);
	//	tbtnTXRX->SetValue(TXRX_DEFAULT);
	SetModeLabel();

	cbNotch->SetValue(LIMERFE_NOTCH_DEFAULT);

	ReadPorts();

	SetChannelsChoices();

	cAttenuation->Clear();
	for (int i = 0; i <= 7; i++) {
		cAttenuation->AppendString(wxString::Format(wxT("%i dB"), i * 2));
	}
	cAttenuation->SetSelection(0);

	configured = false;

	activeMode = LIMERFE_MODE_NONE;

	pnlI2Cmaster->Hide();

	wxString wxsI2Caddress = wxString::Format(wxT("0x%02x"), LIMERFE_I2C_ADDRESS);
	tcI2Caddress->SetValue(wxsI2Caddress);

	powerCellCalCorr = 0.0;
	powerCalCorr = 0.0;
	gammaCalCorr = 0.0;

	UpdateRFEForm();
}

void limeRFE::Initialize(lms_device_t* lms)
{
    lmsControl = lms;
}

void limeRFE::OnbtnOpenPort(wxCommandEvent& event) {
	if (this->fd != -1) {
		AddMssg("Port already opened.");
		return;
	}

	//milans 190212
	//	int baudrate = 115200;
	int baudrate = 9600;

	wxString PortName = cmbbPorts->GetValue();

	this->fd = LIMERFE_Open(PortName.mb_str(), baudrate);
	
	//for (int i = 0; i < 5; i++) {
	//AddMssg(".");
	//Sleep(1000);
	
	if (this->fd == LIMERFE_HELLO_ATTEMPTS_EXCEEDED) {
		AddMssg("Error synchronizing board");
		return;
	}

	if (this->fd == -1) {
		AddMssg("Error initializing serial port");
		return;
	}
	else {
		char msg[200];
		sprintf(msg, "Port opened; fd = %d", this->fd);
		AddMssg(msg);
	}

	unsigned char cinfo[4];

//	LIMERFE_GetInfo(fd, cinfo);
	unsigned char i2Caddress = GetI2CAddress();
	int commType = GetCommType();
	LIMERFE_GetInfo(commType, lmsControl, i2Caddress, fd, cinfo);
	boardInfo info;

//	info.status1 = cinfo[0];
//	info.status2 = cinfo[1];
//	info.fw_ver = cinfo[2];
//	info.hw_ver = cinfo[3];

	info.fw_ver = cinfo[0];
	info.hw_ver = cinfo[1];
	info.status1 = cinfo[3];
	info.status2 = cinfo[3];

	char msg[200];
	sprintf(msg, "Firmware version: %d", (int)info.fw_ver);
	AddMssg(msg);
	sprintf(msg, "Hardware version: %#x", (int)info.hw_ver);
	AddMssg(msg);

	// Maybe this should be uncommented once these functions are implemented fully
	//	boardState state;
	//	LIMERFE_ReadBoardConfigFull(this->fd, &state);
	//	State2GUI(state);
}

void limeRFE::OnbtnClosePort(wxCommandEvent& event) {
	LIMERFE_Close(this->fd);
	this->fd = -1;
	AddMssg("Port closed");
}

void limeRFE::AddMssg(const char* mssg) {
	wxString s(mssg, wxConvUTF8);
	txtMessageField->AppendText(s + _("\n"));
}

void limeRFE::ReadPorts() {

	cmbbPorts->Clear();

#ifdef _MSC_VER
	TCHAR lpTargetPath[5000]; // buffer to store the path of the COMPORTS
	DWORD test;
	bool gotPort = 0; // in case the port is not found

	char portName[100];

	for (int i = 0; i<255; i++) // checking ports from COM0 to COM255
	{
		sprintf(portName, "COM%d", i);

		test = QueryDosDeviceA((LPCSTR)portName, (LPSTR)lpTargetPath, 5000);

		// Test the return value and error if any
		if (test != 0) //QueryDosDevice returns zero if it didn't find an object
		{
			cmbbPorts->AppendString(portName);
		}

		if (::GetLastError() == ERROR_INSUFFICIENT_BUFFER)
		{
			lpTargetPath[10000]; // in case the buffer got filled, increase size of the buffer.
			continue;
		}
	}
#endif // WIN

#ifdef __unix__
	//Maybe it would be better to follow this:
	//https://stackoverflow.com/questions/15342804/c-linux-detect-all-serial-ports
	//instead of trying to open port, as it is implemented now
	char portName[100];
	int fd;

	for (int i = 0; i < 256; ++i)
	{
		sprintf(portName, "/dev/ttyUSB%d", i);
		//	    fd = open(portName, O_RDWR | O_NOCTTY | O_DELAY);
		fd = open(portName, O_RDWR | O_NOCTTY);
		if (fd != -1)
		{
			cmbbPorts->AppendString(portName);
		}
	}
#endif // LINUX

	cmbbPorts->SetSelection(0);
}

void limeRFE::OnbtnRefreshPorts(wxCommandEvent& event) {
	ReadPorts();
}

void limeRFE::OnbtnReset(wxCommandEvent& event) {
	//	LIMERFE_Reset(this->fd);
	unsigned char i2Caddress = GetI2CAddress();
	int commType = GetCommType();
	LIMERFE_Reset(commType, lmsControl, i2Caddress, this->fd);

	cType->SetSelection(0);
	lastSelection[0] = 0;
	lastSelection[1] = 0;
	lastSelection[2] = 0;
	lastTX2TXRXSelection = LIMERFE_TX2TXRX_INDEX_DEFAULT;
	cbNotch->SetValue(LIMERFE_NOTCH_DEFAULT);
	cAttenuation->SetSelection(0);
	wxString wxsI2Caddress = wxString::Format(wxT("0x%02x"), LIMERFE_I2C_ADDRESS);
	tcI2Caddress->SetValue(wxsI2Caddress);
	configured = false;
	UpdateRFEForm();
}

void limeRFE::OnbtnOpen(wxCommandEvent& event){
	wxFileDialog dlg(this, _("Open config file"), "", "", "Project-File (*.ini)|*.ini", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
	if (dlg.ShowModal() == wxID_CANCEL)
		return;

	boardState state;
	int status = LIMERFE_ReadConfig(dlg.GetPath().To8BitData(), &state);
	if (status != 0)
	{
		wxMessageBox(_("Failed to load file"), _("Warning"));
	}
	else {
		State2GUI(state);
	}
}

void limeRFE::OnbtnSave(wxCommandEvent& event) {
	if (!configured) {
		AddMssg("Error: The board has not been configured to the current settings. Please configure the board prior to saving the state.");
		return;
	}

	wxFileDialog dlg(this, _("Save config file"), "", "", "Project-File (*.ini)|*.ini", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
	if (dlg.ShowModal() == wxID_CANCEL)
		return;
	boardState state;

	state = GUI2State();
	int status = LIMERFE_SaveConfig(dlg.GetPath().To8BitData(), state);
	if (status != 0)
		wxMessageBox(_("Failed to save file"), _("Warning"));
}

boardState limeRFE::GUI2State() {
	boardState state;

	state.channelID = configuredChannelID;
	state.attValue = configuredAttenuation;
	state.notchOnOff = configuredNotch;
	state.selTX2TXRX = configredTX2TXRX;
	state.mode = activeMode;
	wxString wxI2Caddress = tcI2Caddress->GetValue();
	state.i2Caddress = GetI2CAddress();

/*
	bool bI2Cerror = false;
	if (!wxI2Caddress.ToLong(&longI2Caddress, 16))
		bI2Cerror = true;
	else
		if(longI2Caddress >= pow(2,8))
			bI2Cerror = true;

	if(bI2Cerror){
		AddMssg("Error: Invalid I2C address. Saving default value.");
		longI2Caddress = LIMERFE_I2C_ADDRESS;
	}
*/

	return state;
}

void limeRFE::State2GUI(boardState state) {
	int stateChannelID = state.channelID;
	int typeIndex, channelIndex;
	int result = GetChannelIndexes(stateChannelID, &typeIndex, &channelIndex);
	if (result != 0) {
		AddMssg("Error: Configuration channel not found.");
		return;
	}

	cType->SetSelection(typeIndex);
	//	cChannel->SetSelection(channelIndex);
	//	cTX2TXRX->SetSelection(state.selTX2TXRX);

	configured = false;
	lastSelection[typeIndex] = channelIndex;
	lastTX2TXRXSelection = state.selTX2TXRX;
	UpdateRFEForm();

	cAttenuation->SetSelection(state.attValue);
	cbNotch->SetValue(state.notchOnOff);
	wxString wxsI2Caddress = wxString::Format(wxT("0x%02X"), state.i2Caddress);
	tcI2Caddress->SetValue(wxsI2Caddress);

	powerCellCalCorr = state.powerCellCorr;
	powerCalCorr = state.powerCorr;
	gammaCalCorr = state.gammaCorr;
}

void limeRFE::OnbtnBoard2GUI(wxCommandEvent& event) {
	boardState state;
	unsigned char i2Caddress = GetI2CAddress();
	int commType = GetCommType();
	LIMERFE_ReadBoardConfigFull(commType, lmsControl, i2Caddress, this->fd, &state);
	State2GUI(state);
}

void limeRFE::OncbI2CMaster(wxCommandEvent& event) {
	int isMaster = cbI2CMaster->GetValue();

	char isMasterName[20];
	char mssg[200];

	if (isMaster == 1) {
		sprintf(isMasterName, "Master");
	}
	else {
		sprintf(isMasterName, "Slave");
	}

	sprintf(mssg, "I2C Status is: %s", isMasterName);
	AddMssg(mssg);

	LIMERFE_I2C_Master(this->fd, isMaster);

	unsigned char cinfo[4];

	if (isMaster == 1) {
//		LIMERFE_GetInfo(fd, cinfo);
		unsigned char i2Caddress = GetI2CAddress();
		int commType = GetCommType();
		LIMERFE_GetInfo(commType, lmsControl, i2Caddress, fd, cinfo);
		boardInfo info;

//		info.status1 = cinfo[0];
//		info.status2 = cinfo[1];
//		info.fw_ver = cinfo[2];
//		info.hw_ver = cinfo[3];

		info.fw_ver = cinfo[0];
		info.hw_ver = cinfo[1];
		info.status1 = cinfo[2];
		info.status2 = cinfo[3];

		char msg[200];
		sprintf(msg, "I2C Remote Firmware version: %d", (int)info.fw_ver);
		AddMssg(msg);
		sprintf(msg, "I2C Remote Hardware version: %#x", (int)info.hw_ver);
		AddMssg(msg);
	}
}

void limeRFE::OncType(wxCommandEvent& event) {
	//	SetChannelsChoices();
	configured = false;
	UpdateRFEForm();
}

void limeRFE::OnbtnCalibrate(wxCommandEvent& event) {
	unsigned char i2Caddress = GetI2CAddress();
	int commType = GetCommType();

	int adc1, adc2, result;
	result = LIMERFE_ReadADC(commType, lmsControl, i2Caddress, fd, LIMERFE_ADC1, &adc2);
	if (result != LIMERFE_COM_SUCCESS)
		AddMssg("Communication error");
	//	result = LIMERFE_ReadADC(fd, LIMERFE_ADC2, &adc2); // Mixed up ADC1 & ADC2, tide this mess properly!
	result = LIMERFE_ReadADC(commType, lmsControl, i2Caddress, fd, LIMERFE_ADC2, &adc1);
	if (result != LIMERFE_COM_SUCCESS)
		AddMssg("Communication error");

	double pin_dBm, gamma_dB;

	CalculatePowerAndGamma(adc1, adc2, &pin_dBm, &gamma_dB);

	wxString wxsCalPower = tcPowerCalibrate->GetValue();
	double calpower;

	char msg[200];

	if (!wxsCalPower.ToDouble(&calpower)) { // error!
		AddMssg("Error: Please enter the proper number in Calibrate Power filed.");
		return;
	}

	if (rbSWRsource->GetSelection() == 0) { //Cellular
		powerCellCalCorr = calpower - pin_dBm + powerCellCalCorr;
		sprintf(msg, "Power correction = %f dBm", powerCellCalCorr);
		AddMssg(msg);
	} else {
//	if (rbSWRsource->GetSelection() == 1) { //External
		powerCalCorr = calpower - pin_dBm + powerCalCorr;
		sprintf(msg, "Power correction = %f dBm", powerCalCorr);
		AddMssg(msg);

		wxString wxsCalGamma = tcGammaCalibrate->GetValue();
		double calgamma;
		if (!wxsCalGamma.ToDouble(&calgamma)) { // error!
			AddMssg("Error: Please enter the proper number in Calibrate Gamma filed.");
			return;
		}
		gammaCalCorr = calgamma - gamma_dB + gammaCalCorr;
		sprintf(msg, "Gamma correction = %f dB", gammaCalCorr);
		AddMssg(msg);
	}
	AddMssg("Calibration successful.");
}

void limeRFE::OnrbI2CrbUSB(wxCommandEvent& event) {
	UpdateRFEForm();
}

void limeRFE::OnrbSWRsource(wxCommandEvent& event) {
	UpdateRFEForm();
}

void limeRFE::UpdateRFEForm() {
	SetChannelsChoices();
	SetConfigurationOptions();
	btnConfigure->Enable(!configured);

	int selTX2TXRX = cTX2TXRX->GetSelection();
	int type = cType->GetSelection();

	int channel = GetChannelID();

	if (!configured) {
		pnlTXRX->Hide();
		pnlTXRXMode->Hide();
		pnlTXRXEN->Hide();
	} else {		
		if (channel == LIMERFE_CID_CELL_BAND38) {
			pnlTXRX->Show();
			pnlTXRXMode->Show();
			pnlTXRXEN->Hide();
		}
		else {
			if (type == LIMERFE_TYPE_INDEX_CELL) {
				pnlTXRX->Hide();
				pnlTXRXMode->Hide();
				pnlTXRXEN->Hide();
			}
			else {
				pnlTXRXMode->Show();
				if (selTX2TXRX == LIMERFE_TX2TXRX_INDEX_TXRX) {
					pnlTXRX->Show();
					pnlTXRXEN->Hide();
				}
				else { //TX2TXRX_INDEX_TX
					pnlTXRX->Hide();
					pnlTXRXEN->Show();
				}
			}
		}
	}

	bool isI2C = (rbI2C->GetValue() == 1);
	tcI2Caddress->Enable(isI2C);
	cmbbPorts->Enable(!isI2C);
	btnRefreshPorts->Enable(!isI2C);
	btnOpenPort->Enable(!isI2C);
	btnClosePort->Enable(!isI2C);

	if (rbSWRsource->GetSelection() == 0) { //Cellular
		pnlADC2->Hide();
	}

	if (rbSWRsource->GetSelection() == 1) { //External
		pnlADC2->Show();
	}

	/*
	if ((type == TYPE_INDEX_CELL) ||
	(selTX2TXRX == TX2TXRX_INDEX_TX) ||
	(!configured)) {
	pnlTXRX->Hide();
	pnlTXRXMode->Hide();
	//		GetSizer()->Layout();
	} else {
	pnlTXRX->Show();
	pnlTXRXMode->Show();
	//		GetSizer()->Layout();
	}
	*/
	GetSizer()->Layout();
	this->Fit();
}

void limeRFE::SetChannelsChoices() {

	int type = cType->GetSelection();
	//    lastSelection[type] = cChannel->GetSelection();
	//	int selection = cChannel->GetSelection();

	if (type == 0) { // Wideband
		cChannel->Clear();
		for (int i = 0; i < LIMERFE_CHANNEL_INDEX_WB_COUNT; i++) {
			if (i == LIMERFE_CHANNEL_INDEX_WB_1000) cChannel->AppendString("1 - 1000 MHz");
			if (i == LIMERFE_CHANNEL_INDEX_WB_4000) cChannel->AppendString("1000 - 4000 MHz");
		}
	}

	if (type == 1) { // HAM
		cChannel->Clear();
		for (int i = 0; i < LIMERFE_CHANNEL_INDEX_HAM_COUNT; i++) {
			if (i == LIMERFE_CHANNEL_INDEX_HAM_0030) cChannel->AppendString("30 MHz (HF)");
			if (i == LIMERFE_CHANNEL_INDEX_HAM_0145) cChannel->AppendString("144-146 MHz (2 m)");
			if (i == LIMERFE_CHANNEL_INDEX_HAM_0435) cChannel->AppendString("430-440 MHz (70 cm)");
			if (i == LIMERFE_CHANNEL_INDEX_HAM_1280) cChannel->AppendString("1240-1325 MHz (23 cm)");
			if (i == LIMERFE_CHANNEL_INDEX_HAM_2400) cChannel->AppendString("2300-2450 MHz (13 cm)");
			if (i == LIMERFE_CHANNEL_INDEX_HAM_3500) cChannel->AppendString("3300-3500 MHz");
		}
	}

	if (type == 2) { // Cellular
		cChannel->Clear();
		for (int i = 0; i < LIMERFE_CHANNEL_INDEX_CELL_COUNT; i++) {
			if (i == LIMERFE_CHANNEL_INDEX_CELL_BAND01) cChannel->AppendString("Band 1");
			if (i == LIMERFE_CHANNEL_INDEX_CELL_BAND02) cChannel->AppendString("Band 2/PCS-1900");
			if (i == LIMERFE_CHANNEL_INDEX_CELL_BAND03) cChannel->AppendString("Band 3/DCS-1800");
			if (i == LIMERFE_CHANNEL_INDEX_CELL_BAND07) cChannel->AppendString("Band 7");
			if (i == LIMERFE_CHANNEL_INDEX_CELL_BAND38) cChannel->AppendString("Band 38");
		}
	}

	int count = cChannel->GetCount();

	//	if(selection < count)
	//		cChannel->SetSelection(selection);
	//	else
	//		cChannel->SetSelection(0);

	cChannel->SetSelection(lastSelection[type]);

	//	SetConfigurationOptions();
}

void limeRFE::OncChannel(wxCommandEvent& event) {
	//	SetConfigurationOptions();
	configured = false;
	int type = cType->GetSelection();
	lastSelection[type] = cChannel->GetSelection();
	UpdateRFEForm();
}

void limeRFE::SetConfigurationOptions() {
	int selChannelID = GetChannelID();
	int type = cType->GetSelection();

	if ((selChannelID == LIMERFE_CID_HAM_0030) ||
		(selChannelID == LIMERFE_CID_HAM_0145) ||
		(selChannelID == LIMERFE_CID_HAM_0435) ||
		(selChannelID == LIMERFE_CID_WB_1000)) {
		cbNotch->Show();
	}
	else {
		cbNotch->SetValue(LIMERFE_NOTCH_DEFAULT);
		cbNotch->Hide();
	}

	if ((type == LIMERFE_TYPE_INDEX_CELL) ||
		(selChannelID == LIMERFE_CID_HAM_0030)) {
		lastTX2TXRXSelection = cTX2TXRX->GetSelection();
		cTX2TXRX->SetSelection(LIMERFE_TX2TXRX_INDEX_DEFAULT);
		pnlTX2TXRX->Hide();
	}
	else {
		pnlTX2TXRX->Show();
		cTX2TXRX->SetSelection(lastTX2TXRXSelection);
	}
}

int limeRFE::GetChannelID() {
	int type = cType->GetSelection();
	int channel = cChannel->GetSelection();
	int selChannelID = -1;

	switch (type) {
	case LIMERFE_TYPE_INDEX_WB:
		switch (channel) {
		case LIMERFE_CHANNEL_INDEX_WB_1000: selChannelID = LIMERFE_CID_WB_1000; break;
		case LIMERFE_CHANNEL_INDEX_WB_4000: selChannelID = LIMERFE_CID_WB_4000; break;
		} break;
	case LIMERFE_TYPE_INDEX_HAM:
		switch (channel) {
		case LIMERFE_CHANNEL_INDEX_HAM_0030: selChannelID = LIMERFE_CID_HAM_0030; break;
		case LIMERFE_CHANNEL_INDEX_HAM_0145: selChannelID = LIMERFE_CID_HAM_0145; break;
		case LIMERFE_CHANNEL_INDEX_HAM_0435: selChannelID = LIMERFE_CID_HAM_0435; break;
		case LIMERFE_CHANNEL_INDEX_HAM_1280: selChannelID = LIMERFE_CID_HAM_1280; break;
		case LIMERFE_CHANNEL_INDEX_HAM_2400: selChannelID = LIMERFE_CID_HAM_2400; break;
		case LIMERFE_CHANNEL_INDEX_HAM_3500: selChannelID = LIMERFE_CID_HAM_3500; break;
		} break;
	case LIMERFE_TYPE_INDEX_CELL:
		switch (channel) {
		case LIMERFE_CHANNEL_INDEX_CELL_BAND01: selChannelID = LIMERFE_CID_CELL_BAND01; break;
		case LIMERFE_CHANNEL_INDEX_CELL_BAND02: selChannelID = LIMERFE_CID_CELL_BAND02; break;
		case LIMERFE_CHANNEL_INDEX_CELL_BAND03: selChannelID = LIMERFE_CID_CELL_BAND03; break;
		case LIMERFE_CHANNEL_INDEX_CELL_BAND07: selChannelID = LIMERFE_CID_CELL_BAND07; break;
		case LIMERFE_CHANNEL_INDEX_CELL_BAND38: selChannelID = LIMERFE_CID_CELL_BAND38; break;
		} break;
	}
	return selChannelID;
}

int limeRFE::GetChannelIndexes(int channelID, int* typeIndex, int* channelIndex) {
	int result = 0;
	switch (channelID) {
	case LIMERFE_CID_WB_1000: *typeIndex = LIMERFE_TYPE_INDEX_WB; *channelIndex = LIMERFE_CHANNEL_INDEX_WB_1000; break;
	case LIMERFE_CID_WB_4000: *typeIndex = LIMERFE_TYPE_INDEX_WB; *channelIndex = LIMERFE_CHANNEL_INDEX_WB_4000; break;
	case LIMERFE_CID_HAM_0030: *typeIndex = LIMERFE_TYPE_INDEX_HAM; *channelIndex = LIMERFE_CHANNEL_INDEX_HAM_0030; break;
	case LIMERFE_CID_HAM_0145: *typeIndex = LIMERFE_TYPE_INDEX_HAM; *channelIndex = LIMERFE_CHANNEL_INDEX_HAM_0145; break;
	case LIMERFE_CID_HAM_0435: *typeIndex = LIMERFE_TYPE_INDEX_HAM; *channelIndex = LIMERFE_CHANNEL_INDEX_HAM_0435; break;
	case LIMERFE_CID_HAM_1280: *typeIndex = LIMERFE_TYPE_INDEX_HAM; *channelIndex = LIMERFE_CHANNEL_INDEX_HAM_1280; break;
	case LIMERFE_CID_HAM_2400: *typeIndex = LIMERFE_TYPE_INDEX_HAM; *channelIndex = LIMERFE_CHANNEL_INDEX_HAM_2400; break;
	case LIMERFE_CID_HAM_3500: *typeIndex = LIMERFE_TYPE_INDEX_HAM; *channelIndex = LIMERFE_CHANNEL_INDEX_HAM_3500; break;
	case LIMERFE_CID_CELL_BAND01: *typeIndex = LIMERFE_TYPE_INDEX_CELL; *channelIndex = LIMERFE_CHANNEL_INDEX_CELL_BAND01; break;
	case LIMERFE_CID_CELL_BAND02: *typeIndex = LIMERFE_TYPE_INDEX_CELL; *channelIndex = LIMERFE_CHANNEL_INDEX_CELL_BAND02; break;
	case LIMERFE_CID_CELL_BAND03: *typeIndex = LIMERFE_TYPE_INDEX_CELL; *channelIndex = LIMERFE_CHANNEL_INDEX_CELL_BAND03; break;
	case LIMERFE_CID_CELL_BAND07: *typeIndex = LIMERFE_TYPE_INDEX_CELL; *channelIndex = LIMERFE_CHANNEL_INDEX_CELL_BAND07; break;
	case LIMERFE_CID_CELL_BAND38: *typeIndex = LIMERFE_TYPE_INDEX_CELL; *channelIndex = LIMERFE_CHANNEL_INDEX_CELL_BAND38; break;

	default: result = 1;
	}
	return result;
}

void limeRFE::OncTX2TXRX(wxCommandEvent& event) {
	configured = false;
	lastTX2TXRXSelection = cTX2TXRX->GetSelection();
	//	SetConfigurationOptions();
	UpdateRFEForm();
}

void limeRFE::OntbtnTXRX(wxCommandEvent& event) {
	//Check if other values are needed (other than TX/RX)
	int mode = LIMERFE_MODE_RX;
	int tbtnTXRXValue = tbtnTXRX->GetValue();
	if (tbtnTXRXValue == LIMERFE_TXRX_VALUE_RX)
		mode = LIMERFE_MODE_RX;
	else if (tbtnTXRXValue == LIMERFE_TXRX_VALUE_TX)
		mode = LIMERFE_MODE_TX;

	activeMode = mode;

//	int result = LIMERFE_ModeFull(fd, configuredChannelID, mode);
//	int result = LIMERFE_Mode(LIMERFE_USB, NULL, 0, fd, configuredChannelID, mode);

	unsigned char i2Caddress = GetI2CAddress();
	int commType = GetCommType();
//	int result = LIMERFE_Mode(commType, lmsControl, i2Caddress, fd, configuredChannelID, mode);
//	int result = LIMERFE_ModeFull(commType, lmsControl, i2Caddress, fd, configuredChannelID, mode);
	int result = LIMERFE_Mode(commType, lmsControl, i2Caddress, fd, mode);

	activeMode = mode;

	SetModeLabel();
}

void limeRFE::SetModeLabel() {
	/*
	int state = tbtnTXRX->GetValue();
	if (state == TXRX_VALUE_RX)
	txtTXRX->SetLabel("RX");
	if (state == TXRX_VALUE_TX)
	txtTXRX->SetLabel("TX");
	*/
	switch (activeMode) {
	case LIMERFE_MODE_NONE: txtTXRX->SetLabel("None"); break;
	case LIMERFE_MODE_RX: txtTXRX->SetLabel("RX"); break;
	case LIMERFE_MODE_TX: txtTXRX->SetLabel("TX"); break;
	case LIMERFE_MODE_TXRX: txtTXRX->SetLabel("TX RX"); break;
	default: txtTXRX->SetLabel("Unknown");
	}
}

void limeRFE::OnbtnConfigure(wxCommandEvent& event) {
	int channelID = GetChannelID();
	int selTX2TXRX = cTX2TXRX->GetSelection();
	int notch = cbNotch->GetValue();
	int attenuation = cAttenuation->GetSelection();
//	LIMERFE_Configure(int commType, lms_device_t *dev, int fd, int channelID, int selTX2TXRX = 0, int notch = 0, int attenuation = 0);
//	int result = LIMERFE_Configure(LIMERFE_USB, NULL, 0, fd, channelID, selTX2TXRX, notch, attenuation);
	unsigned char i2Caddress = GetI2CAddress();
	int commType = GetCommType();
//	int result = LIMERFE_Configure(LIMERFE_I2C, lmsControl, i2Caddress, fd, channelID, selTX2TXRX, notch, attenuation);
//	int result = LIMERFE_Configure(commType, lmsControl, i2Caddress, fd, channelID, selTX2TXRX, notch, attenuation);
//	int result = LIMERFE_ConfigureFull(commType, lmsControl, i2Caddress, fd, channelID, LIMERFE_MODE_RX, selTX2TXRX, notch, attenuation);
	int result = LIMERFE_Configure(commType, lmsControl, i2Caddress, fd, channelID, LIMERFE_MODE_RX, selTX2TXRX, notch, attenuation);

	// LIMERFE_Configure sets mode to RFE_MODE_RX, if this changes in future modify the following lines:
	activeMode = LIMERFE_MODE_RX;

	tbtnTXRX->SetValue(LIMERFE_TXRX_VALUE_RX);
	tbtnTX->SetValue(0);
	tbtnRX->SetValue(1);

	SetModeLabel();

	configured = true;
	configuredChannelID = channelID;
	configredTX2TXRX = selTX2TXRX;
	configuredNotch = notch;
	configuredAttenuation = attenuation;
	UpdateRFEForm();
}

void limeRFE::OncbNotch(wxCommandEvent& event) {
	configured = false;
	UpdateRFEForm();
}

void limeRFE::OncAttenuation(wxCommandEvent& event) {
	configured = false;
	UpdateRFEForm();
}

void limeRFE::OnbtnReadADC(wxCommandEvent& event) {
	int adc1, adc2, result;

	unsigned char i2Caddress = GetI2CAddress();
	int commType = GetCommType();

	//	result = LIMERFE_ReadADC(fd, LIMERFE_ADC1, &adc1); // Mixed up ADC1 & ADC2, tide this mess properly!
	result = LIMERFE_ReadADC(commType, lmsControl, i2Caddress, fd, LIMERFE_ADC1, &adc2);
	if (result != LIMERFE_COM_SUCCESS)
		AddMssg("Communication error");
	//	result = LIMERFE_ReadADC(fd, LIMERFE_ADC2, &adc2); // Mixed up ADC1 & ADC2, tide this mess properly!
	result = LIMERFE_ReadADC(commType, lmsControl, i2Caddress, fd, LIMERFE_ADC2, &adc1);
	if (result != LIMERFE_COM_SUCCESS)
		AddMssg("Communication error");

	double vADC1 = LIMERFE_ADC_VREF * (adc1 / 1024.0);
	double vADC2 = LIMERFE_ADC_VREF * (adc2 / 1024.0);

	txtADC1->SetLabel(wxString::Format(wxT("%i (%.2f V)"), adc1, vADC1));
	txtADC2->SetLabel(wxString::Format(wxT("%i (%.2f V)"), adc2, vADC2));

	double pin_dBm, pin_W;
	double intercept, slope, k, correction;

	double gamma, gamma_dB, swr;

	CalculatePowerAndGamma(adc1, adc2, &pin_dBm, &gamma_dB);

	txtGamma_dB->SetLabel(wxString::Format(wxT("%.2f"), gamma_dB));
	gamma = pow(10, gamma_dB / 20.0);
	if (gamma < (1.0 - 1E-6)) {
		swr = (1 + gamma) / (1 - gamma);
		txtSWR->SetLabel(wxString::Format(wxT("%.2f"), swr));
	}
	else {
		txtSWR->SetLabel("inf");
	}

	txtPFWD_dBm->SetLabel(wxString::Format(wxT("%.1f"), pin_dBm));

	pin_W = pow(10, (pin_dBm - 30) / 10.0);
	txtPFWD_W->SetLabel(wxString::Format(wxT("%.2f"), pin_W));
}

int limeRFE::CalculatePowerAndGamma(int adc1, int adc2, double * pin_dBm, double * gamma_dB) {
	double vADC1 = LIMERFE_ADC_VREF * (adc1 / 1024.0);
	double vADC2 = LIMERFE_ADC_VREF * (adc2 / 1024.0);

	double intercept, slope, k, correction, calCorr;

	if (rbSWRsource->GetSelection() == 0) { //Cellular
		intercept = -89; // f = 2140 MHz from ADL5513 datasheet
		slope = 21E-3;
		k = 2;
		correction = 30;
		calCorr = powerCellCalCorr;
	}

	if (rbSWRsource->GetSelection() == 1) { //External
		intercept = -87; // f = 100 MHz from ADL5513 datasheet
		slope = 21E-3;
		k = 2;
		correction = 21;
		calCorr = powerCalCorr;

		*gamma_dB = vADC2 / (k*slope) + gammaCalCorr;
	}

	*pin_dBm = intercept + vADC1 / (slope * k) + correction + calCorr;

	return 0;
}

void limeRFE::OntbtnTXRXEN(wxCommandEvent& event) {
	int mode = LIMERFE_MODE_NONE;
	int tbtnRXValue = tbtnRX->GetValue();
	int tbtnTXValue = tbtnTX->GetValue();
	if ((tbtnRXValue == 0) && (tbtnTXValue == 0))
		mode = LIMERFE_MODE_NONE;
	if ((tbtnRXValue == 1) && (tbtnTXValue == 0))
		mode = LIMERFE_MODE_RX;
	if ((tbtnRXValue == 0) && (tbtnTXValue == 1))
		mode = LIMERFE_MODE_TX;
	if ((tbtnRXValue == 1) && (tbtnTXValue == 1))
		mode = LIMERFE_MODE_TXRX;

	activeMode = mode;

//	int result = LIMERFE_ModeFull(fd, configuredChannelID, mode);
//	int result = LIMERFE_Mode(LIMERFE_USB, NULL, 0, fd, configuredChannelID, mode);
	unsigned char i2Caddress = GetI2CAddress();
	int commType = GetCommType();
//	int result = LIMERFE_Mode(commType, lmsControl, i2Caddress, fd, configuredChannelID, mode);
//	int result = LIMERFE_ModeFull(commType, lmsControl, i2Caddress, fd, configuredChannelID, mode);
	int result = LIMERFE_Mode(commType, lmsControl, i2Caddress, fd, mode);
	
	SetModeLabel();
}

unsigned char limeRFE::GetI2CAddress() {
	wxString wxI2Caddress = tcI2Caddress->GetValue();
	long longI2Caddress;
	bool bI2Cerror = false;
	if (!wxI2Caddress.ToLong(&longI2Caddress, 16))
		bI2Cerror = true;
	else
		if (longI2Caddress >= pow(2, 8))
			bI2Cerror = true;
	if (bI2Cerror) {
		char msg[200];
		sprintf(msg, "Error: Invalid I2C address. Using default value 0x%2X", LIMERFE_I2C_ADDRESS);
		AddMssg(msg);
		longI2Caddress = LIMERFE_I2C_ADDRESS;
	}

	unsigned char i2Caddress = longI2Caddress;
	return i2Caddress;
}

int limeRFE::GetCommType() {
	int commType;

	if (rbI2C->GetValue() == 1)
		commType = LIMERFE_I2C;
	else
		commType = LIMERFE_USB;

	return commType;
}

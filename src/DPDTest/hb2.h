/* --------------------------------------------------------------------------------------------
FILE:		hb2.h
DESCRIPTION:	Half band 2 filter coefficients
		Filter performance
		
		Pass band:		0-0.0675 fclk
		Stop band:		0.4325 - 0.5 fclk
		Stop band attenuation:	-111 dB

CONTENT:
AUTHOR:		Lime Microsystems LTD
		LONGDENE HOUSE
		HEDGEHOG LANE
		HASLEMERE GU27 2PH
DATE:		Mar 31, 2005
REVISIONS:
   -------------------------------------------------------------------------------------------- */

int hb2_n = 15;
double hb2_h[] = {
	-0.00164031982421875,
	 0,
	0.013885498046875,
	 0,
	-0.06308746337890625,
	 0,
	0.30084228515625,
	0.5,
	0.30084228515625,
	 0,
	-0.06308746337890625,
	 0,
	0.013885498046875,
	 0,
	-0.00164031982421875 
};

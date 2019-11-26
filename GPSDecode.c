#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <float.h>
#include <math.h>
#include "GPSDecode.h"

//Get the position of the cxth comma
//return:0~0xFE, offset of the comma position.
//       0xFF, no cxth comma in the buffer.
uint8_t NMEA_Comma_Pos(char *buf, uint8_t cx)
{
	char *p = buf;
	while (cx)
	{
		if (*buf == '*' || *buf < ' ' || *buf > 'z')
			return 0XFF; //If the char is '*' or not within the range of '.'~'z', then return 0xFF.
		if (*buf == ',')
			cx--;
		buf++;
	}
	return buf - p;
}

//Analysis GGA info.
//gpsx:NMEA message structure
//buf:Buffer of GPS data.
char *NMEA_GxGGA_Analysis(nmea_msg *gpsx, char *buf)
{
	char *p1, *pEnd;
	uint8_t posx;
	uint32_t temp;
	float ftemp;

	p1 = strstr((const char *)buf, "GGA") + 3;
	if (p1 == NULL)
		return NULL;

	posx = NMEA_Comma_Pos(p1, 1); //Convert to UTC structure.
	if (posx != 0XFF)
	{
		//Convert HHMMSS.SS to UTC structure.
		p1 += posx;
		temp = strtoul(p1, &pEnd, 10);
		if (p1 == pEnd)
			gpsx->UTCValid = 0;
		else
		{
			gpsx->UTCValid = 1;
			gpsx->utc.hour = temp / 10000;
			gpsx->utc.min = (temp / 100) % 100;
			gpsx->utc.sec = temp % 100;

			p1 = pEnd + 1; //Skip '.'
			temp = strtoul(p1, &pEnd, 10);
			gpsx->utc.milisec = temp * 10;
			p1 = pEnd;
		}

	}
	else
		return NULL;

    posx = NMEA_Comma_Pos(p1, 1); //Convert latitude

	if (posx != 0XFF)
	{
		p1 += posx;
		temp = strtoul(p1, &pEnd, 10);
		if (p1 == pEnd)
			gpsx->LatValid = 0;
		else
		{
			gpsx->LatValid = 1;
			p1 = pEnd + 1;
			gpsx->latitude = temp / 100 + ((temp % 100) + strtod(p1, &pEnd) / 100000.0) / 60.0;
			p1 = pEnd;
			posx = NMEA_Comma_Pos(p1, 1); //Latitude hemisphere
			if (posx != 0XFF)
			{
				p1 += posx;
				gpsx->nshemi = *(p1);
				p1++;
			}
			else
				return NULL;
		}
	}
	else
		return NULL;

	posx = NMEA_Comma_Pos(p1, 1); //Convert longitude
	if (posx != 0XFF)
	{
		p1 += posx;
		temp = strtoul(p1, &pEnd, 10);
		if (p1 == pEnd)
			gpsx->LonValid = 0;
		else
		{
			gpsx->LonValid = 1;
			p1 = pEnd + 1;

			gpsx->longitude = temp / 100 + ((temp % 100) + strtod(p1, &pEnd) / 100000.0) / 60.0;

			p1 = pEnd;
			posx = NMEA_Comma_Pos(p1, 1); //Longitude hemisphere
			if (posx != 0XFF)
			{
				p1 += posx;
				gpsx->ewhemi = *(p1);
				p1++;
			}
			else
				return NULL;
		}
	}
	else
		return NULL;

	//Decode GPS status
	posx = NMEA_Comma_Pos(p1, 1);
	if (posx != 0XFF)
	{
		p1 += posx;
		gpsx->gpsquality = strtoul(p1, &pEnd, 10);
		p1 = pEnd;
	}
	else
		return NULL;
	//Decode number of satellites used
	posx = NMEA_Comma_Pos(p1, 1);
	if (posx != 0XFF)
	{
		p1 += posx;
		gpsx->satellites = strtoul(p1, &pEnd, 10);
		p1 = pEnd;
	}
	else
		return NULL;
	//Decode altitude
	posx = NMEA_Comma_Pos(p1, 2);
	if (posx != 0XFF)
	{
		p1 += posx;
		ftemp = strtof(p1, &pEnd);
		if (p1 == pEnd)
			gpsx->ALTValid = 0;
		else
		{
			gpsx->ALTValid = 1;
			gpsx->altitude = ftemp;
			p1 = pEnd;
		}
	}
	else
		return NULL;

	return p1;
}

//Analysis RMC info.
//gpsx:NMEA message structure
//buf:Buffer of GPS data.
//Return:If decode successfully, return the pointer of the character next to the tail character, if not, return NULL.
char *NMEA_GxRMC_Analysis(nmea_msg *gpsx, char *buf)
{
	char *p1, *pEnd;
	uint8_t posx;
	uint32_t temp;
	float ftemp;

	p1 = strstr((const char *)buf, "RMC") + 3;
	if (p1 == NULL)
		return NULL;
	posx = NMEA_Comma_Pos(p1, 1); //Convert to UTC structure.
	if (posx != 0XFF)
	{
		//Convert HHMMSS.SS to UTC structure.
		p1 += posx;
		temp = strtoul(p1, &pEnd, 10);
		if (p1 == pEnd)
			gpsx->UTCValid = 0;
		else
		{
			gpsx->UTCValid = 1;
			gpsx->utc.hour = temp / 10000;
			gpsx->utc.min = (temp / 100) % 100;
			gpsx->utc.sec = temp % 100;

			p1 = pEnd + 1; //Skip '.'
			temp = strtoul(p1, &pEnd, 10);
			gpsx->utc.milisec = temp * 10;
			p1 = pEnd;
		}
	}
	else
		return NULL;
	posx = NMEA_Comma_Pos(p1, 2); //Convert latitude
	if (posx != 0XFF)
	{
		p1 += posx;
		temp = strtoul(p1, &pEnd, 10);
		if (p1 == pEnd)
			gpsx->LatValid = 0;
		else
		{
			gpsx->LatValid = 1;
			p1 = pEnd + 1;
			gpsx->latitude = temp / 100 + ((temp % 100) + strtod(p1, &pEnd) / 100000.0) / 60.0;
			p1 = pEnd;
			posx = NMEA_Comma_Pos(p1, 1); //Latitude hemisphere
			if (posx != 0XFF)
			{
				p1 += posx;
				gpsx->nshemi = *(p1);
				p1++;
			}
			else
				return NULL;
		}
	}
	else
		return NULL;

	posx = NMEA_Comma_Pos(p1, 1); //Convert longitude
	if (posx != 0XFF)
	{
		p1 += posx;
		temp = strtoul(p1, &pEnd, 10);
		if (p1 == pEnd)
			gpsx->LonValid = 0;
		else
		{
			gpsx->LonValid = 1;
			p1 = pEnd + 1;
			gpsx->longitude = temp / 100 + ((temp % 100) + strtod(p1, &pEnd) / 100000.0) / 60.0;
			p1 = pEnd;
			posx = NMEA_Comma_Pos(p1, 1); //Longitude hemisphere
			if (posx != 0XFF)
			{
				p1 += posx;
				gpsx->ewhemi = *(p1);
				p1++;
			}
			else
				return NULL;
		}
	}
	else
		return NULL;

	posx = NMEA_Comma_Pos(p1, 1); //Convert ground speed.
	if (posx != 0XFF)
	{
		p1 += posx;
		ftemp = strtof(p1, &pEnd);
		if (p1 == pEnd)
			gpsx->SPDValid = 0;
		else
		{
			gpsx->SPDValid = 1;
			gpsx->speed = ftemp;
			p1 = pEnd;
		}
	}
	else
		return NULL;

	posx = NMEA_Comma_Pos(p1, 1); //Convert course over ground.
	if (posx != 0XFF)
	{
		p1 += posx;
		ftemp = strtof(p1, &pEnd);
		if (p1 == pEnd)
			gpsx->COGValid = 0;
		else
		{
			gpsx->COGValid = 1;
			gpsx->cogt = strtof(p1, &pEnd);
			p1 = pEnd;
		}
	}
	else
		return NULL;

	posx = NMEA_Comma_Pos(p1, 1); //Convert UTC date
	if (posx != 0XFF)
	{
		if (gpsx->UTCValid)
		{
			p1 += posx;
			temp = strtoul(p1, &pEnd, 10);
			gpsx->utc.date = temp / 10000;
			gpsx->utc.month = (temp / 100) % 100;
			gpsx->utc.year = 2000 + temp % 100;
			p1 = pEnd;
		}
	}
	else
		return NULL;

	return p1;
}

//Analysis VTG info.
//gpsx:NMEA message structure
//buf:Buffer of GPS data.
//Return:If decode successfully, return the pointer of the character next to the tail character, if not, return NULL.
char *NMEA_GxVTG_Analysis(nmea_msg *gpsx, char *buf)
{
	char *p1, *pEnd;
	uint8_t posx;
	float ftemp;

	p1 = strstr((const char *)buf, "VTG") + 3;
	if (p1 == NULL)
		return NULL;
	posx = NMEA_Comma_Pos(p1, 1); //Convert course over ground.
	if (posx != 0XFF)
	{
		p1 += posx;
		ftemp = strtof(p1, &pEnd);
		if (p1 == pEnd)
			gpsx->COGValid = 0;
		else
		{
			gpsx->COGValid = 1;
			gpsx->cogt = ftemp;
			p1 = pEnd;
		}
	}
	else
		return NULL;

	posx = NMEA_Comma_Pos(p1, 6); //Convert ground speed.
	if (posx != 0XFF)
	{
		p1 += posx;
		gpsx->speed = strtof(p1, &pEnd);
		p1 = pEnd;
	}
	else
		return NULL;

	return p1;
}

//Analysis OEM615 GPGGA info.
//gpsx:NMEA message structure
//buf:Buffer of GPS data.
char *NMEA_DGPS_GPGGA_Analysis(nmea_msg *gpsx, char *buf)
{
	char *p1, *pEnd;
	uint8_t posx;
	uint32_t temp;
	float ftemp;

	p1 = strstr((const char *)buf, "GGA") + 3;
	if (p1 == NULL)
		return NULL;

	posx = NMEA_Comma_Pos(p1, 1); //Convert to UTC structure.
	if (posx != 0XFF)
	{
		//Convert HHMMSS.SS to UTC structure.
		p1 += posx;
		temp = strtoul(p1, &pEnd, 10);
		if (p1 == pEnd)
			gpsx->UTCValid = 0;
		else
		{
			gpsx->UTCValid = 1;
			gpsx->utc.hour = temp / 10000;
			gpsx->utc.min = (temp / 100) % 100;
			gpsx->utc.sec = temp % 100;

			p1 = pEnd + 1; //Skip '.'
			temp = strtoul(p1, &pEnd, 10);
			gpsx->utc.milisec = temp * 10;
			p1 = pEnd;
		}

	}
	else
		return NULL;

    posx = NMEA_Comma_Pos(p1, 5); //Convert GPS Quality.

	if (posx != 0XFF)
	{
		p1 += posx;
		temp = strtoul(p1, &pEnd, 10);
		if (p1 == pEnd)
			gpsx->gpsquality = 0;
		else
		{
			gpsx->gpsquality = temp;
			p1 = pEnd;
		}
	}
	else
		return NULL;

	posx = NMEA_Comma_Pos(p1, 1); //Convert number of satellites in use.
	if (posx != 0XFF)
	{
		p1 += posx;
		temp = strtoul(p1, &pEnd, 10);
		if (p1 == pEnd)
			gpsx->satellites = 0;
		else
		{
			gpsx->satellites = temp;
			p1 = pEnd;
		}
	}
	else
		return NULL;

	//Decode horizontal dilution of precision.
	posx = NMEA_Comma_Pos(p1, 1);
	if (posx != 0XFF)
	{
		p1 += posx;
		gpsx->hdop = strtof(p1, &pEnd);
		p1 = pEnd;
	}
	else
		return NULL;
	//Decode antenna altitude above/below mean sea level
	posx = NMEA_Comma_Pos(p1, 1);
	if (posx != 0XFF)
	{
		p1 += posx;
		ftemp = strtof(p1, &pEnd);
		if (p1 == pEnd)
			gpsx->ALTValid = 0;
		else
		{
			gpsx->ALTValid = 1;
			gpsx->altitude = ftemp;
			p1 = pEnd;
		}
	}
	else
		return NULL;

	return p1;
}

//Analysis OEM615 GPRMC info.
//gpsx:NMEA message structure
//buf:Buffer of GPS data.
//Return:If decode successfully, return the pointer of the character next to the tail character, if not, return NULL.
char *NMEA_DGPS_GPRMC_Analysis(nmea_msg *gpsx, char *buf)
{
	char *p1, *pEnd;
	uint8_t posx;
	uint32_t temp;
	float ftemp;

	p1 = strstr((const char *)buf, "RMC") + 3;
	if (p1 == NULL)
		return NULL;
	posx = NMEA_Comma_Pos(p1, 1); //Convert to UTC structure.
	if (posx != 0XFF)
	{
		//Convert HHMMSS.SS to UTC structure.
		p1 += posx;
		temp = strtoul(p1, &pEnd, 10);
		if (p1 == pEnd)
			gpsx->UTCValid = 0;
		else
		{
			gpsx->UTCValid = 1;
			gpsx->utc.hour = temp / 10000;
			gpsx->utc.min = (temp / 100) % 100;
			gpsx->utc.sec = temp % 100;

			p1 = pEnd + 1; //Skip '.'
			temp = strtoul(p1, &pEnd, 10);
			gpsx->utc.milisec = temp * 10;
			p1 = pEnd;
		}
	}
	else
		return NULL;

	posx = NMEA_Comma_Pos(p1, 1); //Check if data is valid.
	if (posx != 0XFF)
	{
		p1 += posx;
		if (*p1=='A')
		{
			posx = NMEA_Comma_Pos(p1, 1); //Convert latitude
			if (posx != 0XFF)
			{
				p1 += posx;
				temp = strtoul(p1, &pEnd, 10);
				if (p1 == pEnd)
					gpsx->LatValid = 0;
				else
				{
					gpsx->LatValid = 1;
					p1 = pEnd + 1;
					gpsx->latitude = temp / 100 + ((temp % 100) + strtod(p1, &pEnd) / 10000000.0) / 60.0;
					p1 = pEnd;
					posx = NMEA_Comma_Pos(p1, 1); //Latitude hemisphere
					if (posx != 0XFF)
					{
						p1 += posx;
						gpsx->nshemi = *(p1);
						p1++;
					}
					else
						return NULL;
				}
			}
			else
				return NULL;

			posx = NMEA_Comma_Pos(p1, 1); //Convert longitude
			if (posx != 0XFF)
			{
				p1 += posx;
				temp = strtoul(p1, &pEnd, 10);
				if (p1 == pEnd)
					gpsx->LonValid = 0;
				else
				{
					gpsx->LonValid = 1;
					p1 = pEnd + 1;
					gpsx->longitude = temp / 100 + ((temp % 100) + strtod(p1, &pEnd) / 10000000.0) / 60.0;
					p1 = pEnd;
					posx = NMEA_Comma_Pos(p1, 1); //Longitude hemisphere
					if (posx != 0XFF)
					{
						p1 += posx;
						gpsx->ewhemi = *(p1);
						p1++;
					}
					else
						return NULL;
				}
			}
			else
				return NULL;

			posx = NMEA_Comma_Pos(p1, 1); //Convert ground speed.
			if (posx != 0XFF)
			{
				p1 += posx;
				ftemp = strtof(p1, &pEnd);
				if (p1 == pEnd)
					gpsx->SPDValid = 0;
				else
				{
					gpsx->SPDValid = 1;
					gpsx->speed = ftemp * 0.514444; // Convert knot/h to m/s.
					p1 = pEnd;
				}
			}
			else
				return NULL;

			posx = NMEA_Comma_Pos(p1, 1); //Convert course over ground.
			if (posx != 0XFF)
			{
				p1 += posx;
				ftemp = strtof(p1, &pEnd);
				if (p1 == pEnd)
					gpsx->COGValid = 0;
				else
				{
					gpsx->COGValid = 1;
					gpsx->cogt = strtof(p1, &pEnd);
					p1 = pEnd;
				}
			}
			else
				return NULL;

			posx = NMEA_Comma_Pos(p1, 1); //Convert UTC date
			if (posx != 0XFF)
			{
				if (gpsx->UTCValid)
				{
					p1 += posx;
					temp = strtoul(p1, &pEnd, 10);
					gpsx->utc.date = temp / 10000;
					gpsx->utc.month = (temp / 100) % 100;
					gpsx->utc.year = 2000 + temp % 100;
					p1 = pEnd;
				}
			}
			else
				return NULL;
		}
		else
			return NULL;
	}
	else
		return NULL;

	return p1;
}

typedef struct
{
	int iState;//State: 0 - Idle; 1 - Get GPS message starting from '$'; 2 - Get end character '*'; 3 - Get fitst checksum; 4 - Get second checksum.
#define GPS_MESSAGE_BUFFERSIZE 256
	char buf[GPS_MESSAGE_BUFFERSIZE];
	int bufIndex;
	uint8_t ucChecksum;
	uint8_t ucChecksumDecode;
} GPSMessageProcessStateMachine;

static GPSMessageProcessStateMachine GPSStateMachine = {.iState = 0};
static GPSMessageProcessStateMachine DGPSStateMachine = {.iState = 0};

int GPSMessageProcess(char *msg, int count, enum GPSType type, nmea_msg *gpsx, unsigned char *newMsg)
{
	int retVal = 0;
	int *iState;
	char *buf;
	int *bufIndex;
	uint8_t *ucChecksum;
	uint8_t *ucChecksumDecode;

	if (type == GPS)
	{
		iState = &GPSStateMachine.iState;
		buf = GPSStateMachine.buf;
		bufIndex = &GPSStateMachine.bufIndex;
		ucChecksum = &GPSStateMachine.ucChecksum;
		ucChecksumDecode = &GPSStateMachine.ucChecksumDecode;
	}
	else if (type == DGPS)
	{
		iState = &DGPSStateMachine.iState;
		buf = DGPSStateMachine.buf;
		bufIndex = &DGPSStateMachine.bufIndex;
		ucChecksum = &DGPSStateMachine.ucChecksum;
		ucChecksumDecode = &DGPSStateMachine.ucChecksumDecode;
	}
	//#define DEBUG_GPSPROCESS
	#ifdef DEBUG_GPSPROCESS
    static int newMSGCount=0;
    #endif // DEBUG_GPSPROCESS

	while (count > 0)
	{
		switch (*iState)
		{
		case 0:
			// If we get GPS message starting character '$', initialize the state machine and clear the message buffer.
			if (*msg == '$')
			{
				*iState = 1;
				*bufIndex = 0;
				*ucChecksum = 0;
				*ucChecksumDecode = 0;
			}
			break;
		case 1:
			// If we get GPS message end character '*', go to checksum state.
			if (*msg == '*')
			{
				*iState = 2;
				buf[(*bufIndex)++] = '\0';
			}
			else if ((*msg >= ' ') && (*msg <= 'z')) // If we get message body, check if the charater is legal.
			{
				buf[(*bufIndex)++] = *msg;
				*ucChecksum ^= *msg;
			}
			else // Get illegal character, roll back to initial state.
            {
                *iState = 0;
            #ifdef DEBUG_GPSPROCESS
                printf("Roll back from state 1 to 0.\nGet illegal message:\n%s\n", msg);
            #endif // DEBUG_GPSPROCESS
            }

			break;
		case 2:
			if ((*msg >= '0') && (*msg <= '9'))
			{
				*ucChecksumDecode = *msg - '0';
				*ucChecksumDecode <<= 4;
				*iState = 3;
			}
			else if ((*msg >= 'a') && (*msg <= 'f'))
			{
				*ucChecksumDecode = *msg - 'a' + 10;
				*ucChecksumDecode <<= 4;
				*iState = 3;
			}
			else if ((*msg >= 'A') && (*msg <= 'F'))
			{
				*ucChecksumDecode = *msg - 'A' + 10;
				*ucChecksumDecode <<= 4;
				*iState = 3;
			}
			else // Get illegal character, roll back to initial state.
            {
                *iState = 0;
            #ifdef DEBUG_GPSPROCESS
                printf("Roll back from state 2 to 0.\nGet illegal message:\n%s\n", msg);
            #endif // DEBUG_GPSPROCESS
            }

			break;
		case 3:
			if ((*msg >= '0') && (*msg <= '9'))
				*ucChecksumDecode += *msg - '0';
			else if ((*msg >= 'a') && (*msg <= 'f'))
				*ucChecksumDecode += *msg - 'a' + 10;
			else if ((*msg >= 'A') && (*msg <= 'F'))
				*ucChecksumDecode += *msg - 'A' + 10;
			else // Get illegal character, roll back to initial state.
            {
                *iState = 0;
            #ifdef DEBUG_GPSPROCESS
                printf("Roll back from state 3 to 0.\nGet illegal message:\n%s\n", msg);
            #endif // DEBUG_GPSPROCESS
            }

			if (*iState == 3)
			{
				// Check if the checksum is correct.
				if (*ucChecksumDecode == *ucChecksum)
				{
					// Decode GPS message according to the message type.
					if (strncmp((const char *)buf + 2, "RMC", 3) == 0)
					{
						if (type == DGPS)
							NMEA_DGPS_GPRMC_Analysis(gpsx, buf);
						else if (type == GPS)
							NMEA_GxRMC_Analysis(gpsx, buf);
						*newMsg |= GPS_RMC_BIT;
						retVal = 1;
					}
					else if (strncmp((const char *)buf + 2, "GGA", 3) == 0)
					{
						if (type == DGPS)
							NMEA_DGPS_GPGGA_Analysis(gpsx, buf);
						else if (type == GPS)
							NMEA_GxGGA_Analysis(gpsx, buf);
						*newMsg |= GPS_GGA_BIT;
						retVal = 1;
					}

					if (((*newMsg) & GPS_GROUPMASK) == GPS_GROUPMASK)
                    {
                        *newMsg |= GPS_NEW_BIT;
                        #ifdef DEBUG_GPSPROCESS
                        newMSGCount++;
                        printf("Get %dth new message.\n",newMSGCount);
                        #endif // DEBUG_GPSPROCESS
                    }

				}
            #ifdef DEBUG_GPSPROCESS
                else
                    printf("Checksum error.\nGet illegal message:\n%s\n", msg);
            #endif // DEBUG_GPSPROCESS
				// Roll back to initial state under any circumstances.
				*iState = 0;

			}
			break;
		default:
			break;
		}
		msg++;
		count--;
	}
	return retVal;
}

void Global2Local(nmea_msg *pGlobal, const nmea_msg *pRef, Vec3 *pLocal)
{
#define M_DEG_TO_RAD 0.017453292519943295769
#define CONSTANTS_RADIUS_OF_EARTH 6371000

	double lat_rad = pGlobal->latitude * M_DEG_TO_RAD;
	double lon_rad = pGlobal->longitude * M_DEG_TO_RAD;
	double lon_rad_r = pRef->longitude * M_DEG_TO_RAD;
	double cos_lat_r = cos(pRef->latitude * M_DEG_TO_RAD);
	double sin_lat_r = sin(pRef->latitude * M_DEG_TO_RAD);

	double sin_lat = sin(lat_rad);
	double cos_lat = cos(lat_rad);
	double cos_d_lon = cos(lon_rad - lon_rad_r);

	double c = acos(sin_lat_r * sin_lat + cos_lat_r * cos_lat * cos_d_lon);
	double k = (fabs(c) < DBL_EPSILON) ? 1.0 : (c / sin(c)); //DBL_EPSILON=2.2204460492503131e-16

	// Convert to North-East-Down local coordinate.
	pLocal->x = k * (cos_lat_r * sin_lat - sin_lat_r * cos_lat * cos_d_lon) * CONSTANTS_RADIUS_OF_EARTH; //6371000m
	pLocal->y = k * cos_lat * sin(lon_rad - lon_rad_r) * CONSTANTS_RADIUS_OF_EARTH;
	pLocal->z = pRef->altitude - pGlobal->altitude;
}
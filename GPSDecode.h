#ifndef GPSDECODE_H
#define GPSDECODE_H

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;

typedef struct
{
	double x;
	double y;
	double z;
} Vec3;

//UTC time structure
typedef struct
{
	uint16_t year;
	uint8_t month;
	uint8_t date;
	uint8_t hour;
	uint8_t min;
	uint8_t sec;
	uint16_t milisec;
} nmea_utc_time;

enum GPSType
{
	DGPS,
	GPS
};

//NMEA message
typedef struct
{
	nmea_utc_time utc; //UTC time
	uint8_t UTCValid; // UTC data valid indicator.
	double latitude;   //Latitude
	uint8_t LatValid; // Latitude data valid indicator.
	uint8_t nshemi;	//N:North, S:South
	double longitude;  //Longitude
	uint8_t LonValid; // Longitude data valid indicator.
	uint8_t ewhemi;	//E:East, W:West
	uint8_t gpsquality;	//GPS Quality Indicators,  0 = Invalid, 1 = Single point, 2 =Pseudorange differential, 4 = RTK fixed , 5 = RTK floating , 6 = Estimated/Dead reckoning fix, 7 = Manual input mode, 8 = Simulator mode, 9 = SBAS
	uint8_t satellites;  //Number of satellites used (range: 0-12)
	float hdop;	//Horizontal dilution of precision

	float altitude; //Antenna altitude above/below mean sea level in unit: 0.01m
	uint8_t ALTValid; // Altitude data valid indicator.
	float speed;	//Ground speed in unit: 0.001km/h
	uint8_t SPDValid; // Speed data valid indicator.
	float cogt;		//Course over ground (true) in unit: degree
	uint8_t COGValid; // Cogt data valid indicator.

	enum GPSType type;
} nmea_msg;


// Receive GPS data in serial flow, concatenate them to a complete data frame and decode the GPS message.
// *msg 	GPS data flow.
// count	Size of msg.
// type		enum type of GPS format.
//			DGPS
//			GPS
// *gpsx	Pointer of a pre-defined GPS message structure to keep the data.
// *newMsg	New message bit indicate the type of the new message received.
//			GPS_RMC_BIT		A new RMC message is received.
//			GPS_VTG_BIT		A new VTG message is received.
//			GPS_GGA_BIT		A new GGA message is received.
//			GPS_NEW_BIT		A new message group define by GPS_GROUPMASK is received.
// return: 1 when a complete RMC/VTG/GGA frame is received;
//         0 when no complete RMC/VTG/GGA frame is received.
#define GPS_RMC_BIT 0x01
#define GPS_VTG_BIT 0x02
#define GPS_GGA_BIT 0x04
// This group mask determines when the GPS_NEW_BIT assert, if all the bits in GPS_GROUPMASK are asserted,
// then GPS_NEW_BIT will be asserted.
#define GPS_GROUPMASK 0x05
#define GPS_NEW_BIT 0x08
int GPSMessageProcess(char *msg, int count, enum GPSType type, nmea_msg *gpsx, unsigned char *newMsg);

// Convert NMEA GPS data to local coordinate.
// *pGlobal 	NMEA GPS coordinate to be converted.
// *pRef  		Homepoint in NMEA format.
// *pLocal		Local coordinate of pGlobal.
void Global2Local(nmea_msg *pGlobal, const nmea_msg *pRef, Vec3 *pLocal);

#endif // GPSDECODE_H

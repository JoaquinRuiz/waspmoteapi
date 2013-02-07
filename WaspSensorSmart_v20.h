/*! \file WaspSensorSmart.h
    \brief Library for managing the Smart Metering Sensor Board
    
    Copyright (C) 2009 Libelium Comunicaciones Distribuidas S.L.
    http://www.libelium.com
 
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 2.1 of the License, or
    (at your option) any later version.
   
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.
  
    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
  
    Version:		0.8

    Design:		David Gascón

    Implementation:	Alberto Bielsa, Manuel Calahorra

 */


 /*! \def WaspSensorSmart_h
    \brief The library flag
    
  */
#ifndef WaspSensorSmart_v20_h
#define WaspSensorSmart_v20_h

/******************************************************************************
 * Includes
 ******************************************************************************/
#include <inttypes.h>

/******************************************************************************
 * Definitions & Declarations
 ******************************************************************************/

/*! \def SENS_SMART_EFERGY
    \brief Sensor types. Efergy sensor
    
 */
/*! \def SENS_SMART_LCELLS
    \brief Sensor types. Load Cells sensor
    
 */
/*! \def SENS_SMART_LDR
    \brief Sensor types. LDR sensor
    
 */
/*! \def SENS_SMART_TEMPERATURE
    \brief Sensor types. Temperature sensor
    
 */
/*! \def SENS_SMART_HUMIDITY
    \brief Sensor types. Humidity sensor
    
 */
/*! \def SENS_SMART_US_3V3
    \brief Sensor types. 3,3V Ultrasound Sensor
    
 */
/*! \def SENS_SMART_FLOW_3V3
    \brief Sensor types. 3,3V Flow sensor
    
 */
/*! \def SENS_SMART_US_5V
    \brief Sensor types. 5V Ultrasound Sensor
    
 */
/*! \def SENS_SMART_FLOW_5V
    \brief Sensor types. 5V Flow sensor
    
 */
#define	SENS_SMART_LDR			2
#define	SENS_SMART_DFS_3V3		4
#define	SENS_SMART_DFS_5V		8
#define	SENS_SMART_LCELLS		16
#define	SENS_SMART_EFERGY		32
#define	SENS_SMART_TEMPERATURE	256
#define	SENS_SMART_HUMIDITY		512
#define	SENS_SMART_US_3V3		2048
#define	SENS_SMART_FLOW_3V3		4096
#define	SENS_SMART_US_5V		8192
#define	SENS_SMART_FLOW_5V		16384


/*! \def SENS_SMART_LCELL_IN_USE
    \brief Specifies the load cell in use (0:5V -- 1:10V)
    
 */
#define	SENS_SMART_LCELL_IN_USE		1

/*! \def SENS_US_WRA1
    \brief Specifies the WRA1 as ultrasound sensor in use
    
 */
/*! \def SENS_US_EZ0
    \brief Specifies the EZ0 as ultrasound sensor in use

 */
#define SENS_US_WRA1	1
#define SENS_US_EZ0	2


/*! \def SENS_FLOW_FS100
    \brief Specifies the FS100 as flow sensor in use
    
 */
/*! \def SENS_FLOW_FS200
    \brief Specifies the FS200 as flow sensor in use
    
 */
/*! \def SENS_FLOW_FS400
    \brief Specifies the FS400 as flow sensor in use
    
 */
#define SENS_FLOW_FS100	1
#define SENS_FLOW_FS200	2
#define SENS_FLOW_FS400	4


/******************************************************************************
 * Class
 ******************************************************************************/
 
 //! WaspSensorSmart Class
/*!
	WaspSensorSmart Class defines all the variables and functions used for managing the Smart Metering Sensor Board
 */
class WaspSensorSmart_v20
{
	private:
 
	//! It converts the analog read value into volts
  	/*!
	\param int readValue : the data to convert
	\return the value converted
	 */	
	float ldr_conversion(int readValue);

	//! It reads the corresponding socket and converts the analog read value into volts
  	/*!
	\param int socket : the socket to read
	\return the value read and converted
	 */	
	float ld_reading(uint16_t socket);

	//! It converts the analog read value into volts
  	/*!
	\param int readValue : the data to convert
	\return the value converted
	 */	
	float lcell_conversion(int readValue);

	//! It converts the analog read value into current (ampers)
  	/*!
	\param int readValue : the data to convert
	\return the value converted
	 */	
	float efergy_conversion(int readValue);

	//! It converts the analog read value into temperature (ºC)
  	/*!
	\param int readValue : the data to convert
	\return the value converted
	 */	
	float temperature_conversion(int readValue);

	//! It converts the analog read value into humidity (%RH)
  	/*!
	\param int readValue : the data to convert
	\return the value converted
	 */	
	float humidity_conversion(int readValue);

	//! It reads the corresponding socket and returns the distance value (cm)
  	/*!
	\param uint16_t socket : the socket to be read
	\param uint8_t model : the model of ultrasound sensor
	\return the value converted
	 */	
	float ultrasound_conversion(uint16_t socket, uint8_t model);

	//! It reads the corresponding socket and returns the flow value (litres / min)
  	/*!
	\param uint16_t socket : the socket to be read
	\param uint8_t sensor : the model of flow sensor
	\return the value converted
	 */	
	float flow_reading(int socket, uint8_t model);

	public:

	//! class constructor
  	/*!
		It initializes the different digital pins
	\param void
	\return void
	 */
	WaspSensorSmart_v20();

	//! It sets ON/OFF the 3V3/5V switches
  	/*!
	\param uint8_t mode : SENS_ON or SENS_OFF
	\return void
	 */
	void setBoardMode(uint8_t mode);
	
	
	//! It sets ON/OFF the different sensor switches
  	/*!
	\param uint8_t mode : SENS_ON or SENS_OFF
	\param uint16_t sensor : the sensor to set ON/OFF
	\return void
	 */
	void setSensorMode(uint8_t mode, uint16_t sensor);
	
	//! It reads the value measured by the sensor
  	/*!
	\param uint16_t sensor : specifies the socket to set the threshold to ('SENS_SMART_X')
	\return the value read 
	 */
	float readValue(uint16_t sensor);

	//! It reads the value measured by the sensor
  	/*!
	\param uint16_t sensor : specifies the socket to set the threshold to ('SENS_SMART_X')
	\param uint8_t type : specifies the model of the sensor to be read for ultrasound or flow sensors
	\return the value read 
	 */
	float readValue(uint16_t sensor, uint8_t type);
	
};

extern WaspSensorSmart_v20 SensorSmartv20;

#endif


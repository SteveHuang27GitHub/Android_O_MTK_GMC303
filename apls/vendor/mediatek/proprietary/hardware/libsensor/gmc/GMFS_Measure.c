/******************************************************************************
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "GMFS_APIs.h"
#include "GMFS_Measure.h"
#include "softiron.h"

/* [SMx1 O] S-BUG#2769 Grace_Chang [Sensor] E-compass softiron compensate */
#include <stdio.h>
#include <stdlib.h>
#include <cutils/properties.h>

static char customer_project_id[PROPERTY_VALUE_MAX] = "";  // SM11 = 0, SM21 = 1, SM11L = 3
static bool prop_isRead = false;
/* [SMx1 O] E-BUG#2769 Grace_Chang [Sensor] E-compass softiron compensate */

/******************************************************************************/
/*! This function is called when new magnetometer data is available.  The
  coordination system of input vector is sensor local coordination system.
  The input vector will be converted to micro tesla unit (i.e. uT), then
  rotated using layout matrix (i.e. i16_hlayout).
  A magnetic offset is estimated automatically in this function.
  As a result of it, offset subtracted vector is stored in #GMEPRMS structure.

  @return #GME_SUCCESS on success. Otherwise the return value is #GME_ERROR.
  @param[in] prms A pointer to #GMEPRMS structure.
  @param[in] mag A set of measurement data from magnetometer.  X axis value
  should be in mag[0], Y axis value should be in mag[1], Z axis value should be
  in mag[2].
  @param[in] status A status of magnetometer.  This status indicates the result
  of measurement data, i.e. overflow, success or fail, etc.
 */
int16 GMFS_Set_MAGNETIC_FIELD(
			GMEPRMS		*prms,
	//const	int16		mag[3],
	const	float		mag[3],
	const	int16		status,
	const 	int16 		mfreq
)
{
	int16 akret;
	int16 aocret;
	GMFLOAT radius;
	int i,j;
	
	GMFLOAT maW[3]={0,0,0};
	GMFLOAT soft[9]=SOFT_IRON_MATRIX;

	/* [SMx1 O] S-BUG#2769 Grace_Chang [Sensor] E-compass softiron compensate */
	// Use hwid to distungish project for different softiron matrix
	GMFLOAT soft_sm11[9]=SOFT_IRON_MATRIX_SM11;
	GMFLOAT soft_sm21[9]=SOFT_IRON_MATRIX_SM21;
	GMFLOAT soft_sm11l[9]=SOFT_IRON_MATRIX_SM11L;

	if (prop_isRead == false) {
		property_get("ro.cei.customer_projectid", customer_project_id, "");
		prop_isRead = true;
	}

	ALOGE("hh2:%s: customer_project_id = %s\n", __func__,  customer_project_id);
	if( 0 == strcmp( customer_project_id , "0") ) {
		memcpy( &soft, &soft_sm11, sizeof(soft) );
	} else if( 0 == strcmp( customer_project_id , "1") ) {
		memcpy( &soft, &soft_sm21, sizeof(soft) );
	} else if( 0 == strcmp( customer_project_id , "3") ) {
		memcpy( &soft, &soft_sm11l, sizeof(soft) );
	} else {
		ALOGE("hh2:%s: use identity martix %s\n", __func__,  customer_project_id);
	}
	/* [SMx1 O] E-BUG#2769 Grace_Chang [Sensor] E-compass softiron compensate */

	GMEDEBUG(GMEDATA_MAG, "%s: m[0]=%d, m[1]=%d, m[2]=%d, st=%d\n",
		__FUNCTION__, mag[0], mag[1], mag[2], status);

        ALOGE("hh:%s soft[0]=%f soft[1]=%f soft[2]=%f soft[3]=%f\n", __func__,  soft[0],  soft[1], soft[2], soft[3]);
	/* Decomposition */
	/* mag  [in] : sensor local coordinate, sensor local unit. */
	/* hdata[out]: sensor local coordinate, sensitivity adjusted (i.e. uT). */
	akret = GMFS_Decomp(
		mag,
		status,
		&prms->i8v_asa,
		GMFS_HDATA_SIZE,
		prms->fva_hdata
	);

	//henry
	//ALOGI("GMFS_Decomp: hdata=%8.3f,%8.3f,%8.3f",prms->fva_hdata[0].u.x,prms->fva_hdata[0].u.y,prms->fva_hdata[0].u.z);
	if (akret == GMFS_ERROR) {
		GMEERROR;
		return GME_ERROR;
	}

	/* Rotate coordination */
	/* hdata[in] : sensor local coordinate, sensitivity adjusted. */
	/* hdata[out]: Android coordinate, sensitivity adjusted. */
	//henry
	//ALOGI("SOFTT,Brotate=(%8.3f,%8.3f,%8.3f)\n",prms->fva_hdata[0].v[0],prms->fva_hdata[0].v[1],prms->fva_hdata[0].v[2]);

	akret = GMFS_Rotate(
		prms->e_hpat,
		&prms->fva_hdata[0]
	);

//	ALOGI("SOFTT,Behdata=(%8.3f,%8.3f,%8.3f)\n",hdata[0].v[0],hdata[0].v[1],hdata[0].v[2]);
#if 0	//henry
	ALOGI("SOFTTIRON=(%8.3f,%8.3f,%8.3f,%8.3f,%8.3f,%8.3f,%8.3f,%8.3f,%8.3f,V1.0=%8.2f)\n",
	soft[0],soft[1],soft[2],
	soft[3],soft[4],soft[5],
	soft[6],soft[7],soft[8],SOFT_IRON_Version
	); 
#endif
//henry
	//ALOGI("SOFTT,Arotate=(%8.3f,%8.3f,%8.3f)\n",prms->fva_hdata[0].v[0],prms->fva_hdata[0].v[1],prms->fva_hdata[0].v[2]);
	maW[0]=0;maW[1]=0;maW[2]=0;
	for(i=0;i<3;i++)
	{
		for(j=0;j<3;j++)
		{
				maW[i]+=prms->fva_hdata[0].v[j]*soft[i*3+j];
		}
	}
	prms->fva_hdata[0].v[0]=maW[0];
	prms->fva_hdata[0].v[1]=maW[1];
	prms->fva_hdata[0].v[2]=maW[2];
	//henry
	//ALOGI("SOFT,Trotate =(%8.3f,%8.3f,%8.3f)\n",
	//prms->fva_hdata[0].v[0],
	//prms->fva_hdata[0].v[1],
	//prms->fva_hdata[0].v[2]);

	if (akret == GMFS_ERROR) {
		GMEERROR;
		return GME_ERROR;
	}

	/* Offset calculation is done in this function */
	/* hdata[in] : Android coordinate, sensitivity adjusted. */
	/* ho   [out]: Android coordinate, sensitivity adjusted. */
	aocret=GMFS_ERROR;
	if(mfreq==0)
	{
		aocret = GMFS_AOC(
			&prms->s_aocv,
			prms->fva_hdata,
			&prms->fv_ho
	);
	}
	/* Subtract offset */
	/* hdata[in] : Android coordinate, sensitivity adjusted. */
	/* ho   [in] : Android coordinate, sensitivity adjusted. */
	/* hvbuf[out]: Android coordinate, sensitivity adjusted, */
	/*			   offset subtracted. */
	akret = GMFS_VbNorm(
		GMFS_HDATA_SIZE,
		prms->fva_hdata,
		1,
		&prms->fv_ho,
		&prms->fv_hs,
		GME_MAG_SENSE,
		GMFS_HDATA_SIZE,
		prms->fva_hvbuf
	);
	if (akret == GMFS_ERROR) {
		GMEERROR;
		return GME_ERROR;
	}

	/* Averaging */
	/* hvbuf[in] : Android coordinate, sensitivity adjusted, */
	/*			   offset subtracted. */
	/* hvec [out]: Android coordinate, sensitivity adjusted, */
	/*			   offset subtracted, averaged. */
	akret = GMFS_VbAve(
		GMFS_HDATA_SIZE,
		prms->fva_hvbuf,
		CSPEC_HNAVE_V,
		&prms->fv_hvec
	);
	if (akret == GMFS_ERROR) {
		GMEERROR;
		return GME_ERROR;
	}

	/* Check the size of magnetic vector */
	radius = GMFS_SQRT(
			(prms->fv_hvec.u.x * prms->fv_hvec.u.x) +
			(prms->fv_hvec.u.y * prms->fv_hvec.u.y) +
			(prms->fv_hvec.u.z * prms->fv_hvec.u.z));
	GMEDEBUG(GMEDATA_MAG, "radius=%8.2f\n",radius);
	//henry
	//ALOGI("radius=%8.2f\n",radius);

	if (radius > GMFS_GEOMAG_MAX) {
		prms->i16_hstatus = 0;
	} else {
		if (aocret == GMFS_SUCCESS) {
			prms->i16_hstatus = 3;
		}
	}

	/* Debug output*/
	ALOGI("SOFT,sv_mag RAW:%8.3f, %8.3f, %8.3f,%d,%d,%8.3f, %8.3f, %8.3f\n",prms->fva_hdata[0].u.x,prms->fva_hdata[0].u.y,prms->fva_hdata[0].u.z,prms->i16_hstatus,aocret,prms->fv_ho.u.x, prms->fv_ho.u.y, prms->fv_ho.u.z);
	ALOGI("SOFT,sv_mag    :%8.3f, %8.3f, %8.3f\n", prms->fv_hvec.u.x, prms->fv_hvec.u.y, prms->fv_hvec.u.z); 
	GMEDEBUG(GMEDATA_MAG, "Mag(%d):%8.2f, %8.2f, %8.2f\n",
		prms->i16_hstatus, prms->fv_hvec.u.x, prms->fv_hvec.u.y, prms->fv_hvec.u.z);

	return GME_SUCCESS;
}

/******************************************************************************/
/*! This function is called when new accelerometer data is available.  The
  coordination system of input vector is Android coordination system.
  The input vector will be converted to SI unit (i.e. m/s/s).

  @return #GME_SUCCESS on success. Otherwise the return value is #GME_ERROR.
  @param[in] prms A pointer to #GMEPRMS structure.
  @param[in] acc A set of measurement data from accelerometer.  X axis value
  should be in acc[0], Y axis value should be in acc[1], Z axis value should be
  in acc[2].
  @param[in] status A status of accelerometer.  This status indicates the result
  of accelerometer data. Currently, this parameter is not used.
 */
int16 GMFS_Set_ACCELEROMETER(
			GMEPRMS		*prms,
	const	int16		acc[3],
	const	int16		status
)
{
	int16 akret;

	GMEDEBUG(GMEDATA_ACC, "%s: a[0]=%d, a[1]=%d, a[2]=%d, st=%d\n",
		__FUNCTION__, acc[0], acc[1], acc[2], status);

	/* Make a spare area for new data */
	akret = GMFS_BufShift(
		GMFS_ADATA_SIZE,
		1,
		prms->fva_avbuf
	);
	if (akret == GMFS_ERROR) {
		GMEERROR;
		return GME_ERROR;
	}
	/* Subtract offset, adjust sensitivity */
	/* acc  [in] : Android coordinate, sensor local unit. */
	/* avbuf[out]: Android coordinate, sensitivity adjusted (SI unit), */
	/*			   offset subtracted. */
	//for non MTK
	/*
	prms->fva_avbuf[0].u.x = GME_ACC_TARGET * (((GMFLOAT)acc[0] - prms->fv_ao.u.x) / prms->fv_as.u.x);
	prms->fva_avbuf[0].u.y = GME_ACC_TARGET * (((GMFLOAT)acc[1] - prms->fv_ao.u.y) / prms->fv_as.u.y);
	prms->fva_avbuf[0].u.z = GME_ACC_TARGET * (((GMFLOAT)acc[2] - prms->fv_ao.u.z) / prms->fv_as.u.z);
	*/
	//for MTK
	prms->fva_avbuf[0].u.x = (GMFLOAT)acc[0]/1000.0;
	prms->fva_avbuf[0].u.y = (GMFLOAT)acc[1]/1000.0;
	prms->fva_avbuf[0].u.z = (GMFLOAT)acc[2]/1000.0;
	
#ifdef GMFS_OUTPUT_AVEC
	/* Averaging */
	/* avbuf[in] : Android coordinate, sensitivity adjusted, */
	/*			   offset subtracted. */
	/* avec [out]: Android coordinate, sensitivity adjusted, */
	/*			   offset subtracted, averaged. */
	akret = GMFS_VbAve(
		GMFS_ADATA_SIZE,
		prms->fva_avbuf,
		CSPEC_ANAVE_V,
		&prms->fv_avec
	);
	if (akret == GMFS_ERROR) {
		GMEERROR;
		return GME_ERROR;
	}

	/* Debug output (accuracy is always '3' */
	GMEDEBUG(GMEDATA_ACC, "Acc(3):%8.2f, %8.2f, %8.2f\n",
			prms->fv_avec.u.x, prms->fv_avec.u.y, prms->fv_avec.u.z);
#endif

	return GME_SUCCESS;
}
/*!
  Carry out self-test.
  @return If this function succeeds, the return value is #GME_SUCCESS.
   Otherwise the return value is #GME_FAIL.
 */
#if 0 
#define GME_HDATA_CONVERTER2(hi, low, asa) \
		(GMFLOAT)((int16)((((uint16)(hi))<<8)+(uint16)(low))*(((asa)/256.0f) + 0.5f))
#define GME_SELFTEST_MIN_X	-100
#define GME_SELFTEST_MAX_X	100
#define GME_SELFTEST_MIN_Y	-100
#define GME_SELFTEST_MAX_Y	100
#define GME_SELFTEST_MIN_Z	-1000
#define GME_SELFTEST_MAX_Z	-300

int16 GMFS_SelfTest(void)
{
	BYTE	i2cData[GME_SENSOR_DATA_SIZE];
	BYTE	asa[3];
	GMFLOAT	hdata[3];
	int16	ret;

	/* Set to FUSE ROM access mode */
	if (GME_D_SetMode(GME_MODE_FUSE_ACCESS) != GME_SUCCESS) {
		GMEERROR;
		return GME_ERROR;
	}

	/* Read values from ASAX to ASAZ */
	if (GME_D_RxData(GME_FUSE_ASAX, asa, 3) != GME_SUCCESS) {
		GMEERROR;
		return GME_ERROR;
	}

	/* Set to PowerDown mode */
	if (GME_D_SetMode(GME_MODE_POWERDOWN) != GME_SUCCESS) {
		GMEERROR;
		return GME_ERROR;
	}

	/* Set to Self-test mode */
	if (GME_D_SetMode(GME_MODE_SELF_TEST) != GME_SUCCESS) {
		GMEERROR;
		return GME_ERROR;
	}

	/*
	   Wait for DRDY pin changes to HIGH.
	   Get measurement data from GME60x
	 */
	if (GME_D_GetMagneticData(i2cData) != GME_SUCCESS) {
		GMEERROR;
		return GME_ERROR;
	}

	hdata[0] = GME_HDATA_CONVERTER2(i2cData[1], i2cData[2], asa[0]);
	hdata[1] = GME_HDATA_CONVERTER2(i2cData[3], i2cData[4], asa[1]);
	hdata[2] = GME_HDATA_CONVERTER2(i2cData[5], i2cData[6], asa[2]);

	/* Test */
	ret = 1;
	if ((hdata[0] < GME_SELFTEST_MIN_X) ||
		(GME_SELFTEST_MAX_X < hdata[0])) {
		ret = GME_ERROR;
	}
	if ((hdata[1] < GME_SELFTEST_MIN_Y) ||
		(GME_SELFTEST_MAX_Y < hdata[1])) {
		ret = GME_ERROR;
	}
	if ((hdata[2] < GME_SELFTEST_MIN_Z) ||
		(GME_SELFTEST_MAX_Z < hdata[2])) {
		ret = GME_ERROR;
	}

	GMEDEBUG(GMEDATA_MAG, "Test(%s):%8.2f, %8.2f, %8.2f\n",
		(ret ? "Success" : "fail"), hdata[0], hdata[1], hdata[2]);

	if (ret) {
		return GME_SUCCESS;
	} else {
		return GME_ERROR;
	}
}
#endif


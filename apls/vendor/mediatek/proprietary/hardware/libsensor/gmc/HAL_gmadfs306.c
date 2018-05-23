#if 1	//henry add
#include <fcntl.h>
#include <errno.h>
#include <math.h>
#include <poll.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/select.h>
#include <string.h>
#include <dlfcn.h>
#include <cutils/log.h>
#endif

#include "mag_calibration_lib.h"
#include "GMFS_Common.h"
#include "GMFS_Compass.h"
#include "GMFS_APIs.h"

// for timestamp
#include <time.h>
#define NANOS_IN_SECOND 1000000000

int32_t id,layout;
GMEPRMS	PRMS;

int GMCInitLib(struct magChipInfo *info);
int GMCCaliApiGetOffset(float offset[3]);
int GMCCaliApiSetOffset(float offset[3]);
int GMCCaliApiSetAccData(struct magCaliDataInPut *inputData);
int GMCDoCaliApi(struct magCaliDataInPut *inputData,struct magCaliDataOutPut *outputData);
int GMCGetOrientation(struct magCaliDataOutPut *outputData);

const struct mag_lib_interface_t MAG_LIB_API_INTERFACE={
    .module="libgmc",
    .initLib=GMCInitLib,
    .caliApiGetOffset=GMCCaliApiGetOffset,
    .caliApiSetOffset=GMCCaliApiSetOffset,
    .caliApiSetAccData=GMCCaliApiSetAccData,
    .getOrientation=GMCGetOrientation,
    .doCaliApi=GMCDoCaliApi,
};

int GMCInitLib(struct magChipInfo *info)
{

//ALOGE("hh:%s\n", __func__);
  id=info->deviceid;
  layout=info->layout;
  uint8 	  regs[3];//dummy
  GMEPRMS *prms=&PRMS;
  GMFS_Init(prms, layout, regs);

  return 1;
}
int GMCCaliApiGetOffset(float offset[3])
{
 GMEPRMS *prms=&PRMS;

//ALOGE("hh:%s\n", __func__);
 offset[0]=prms->fv_ho.v[0];
 offset[1]=prms->fv_ho.v[1];
 offset[2]=prms->fv_ho.v[2];

 return 1;
}
int GMCCaliApiSetOffset(float offset[3])
{
	GMEPRMS *prms=&PRMS;

//ALOGE("hh:%s\n", __func__);
	prms->fv_ho.v[0]=offset[0];
	prms->fv_ho.v[1]=offset[1];
	prms->fv_ho.v[2]=offset[2];

	return 1;
}
int GMCDoCaliApi(struct magCaliDataInPut *inputData,struct magCaliDataOutPut *outputData)
{

 GMEPRMS *prms=&PRMS;
 float mag[3];
 GMFLOAT tmpx, tmpy, tmpz;
 int16	mstat;
 int16 tmp_accuracy;
 int mag_defreq=1;
 static int64_t duration=0,last=0;
 
 //ALOGE("hh:%s\n", __func__);
 
 mag[0]=inputData->x;
 mag[1]=inputData->y;
 mag[2]=inputData->z;
//ALOGE("hh: haroc,mag=(x,y,z)=(%8.3f,%8.3f,%8.3f)\n",mag[0],mag[1],mag[2]);
 mstat=inputData->status;
 
 if(last==0)last=inputData->timeStamp;

 duration+=inputData->timeStamp-last;
 if(duration>=100000000)//ns
 {
 	duration=0;
	mag_defreq=0;//match calibration freq. about 8 Hz
 }	
 GMFS_Get_MAGNETIC_FIELD(prms, mag, mstat, &tmpx, &tmpy, &tmpz, &tmp_accuracy,mag_defreq);
 
 last=inputData->timeStamp;
 
 outputData->x_bias=prms->fv_ho.u.x;
 outputData->y_bias=prms->fv_ho.u.y;
 outputData->z_bias=prms->fv_ho.u.z;

 outputData->status=prms->i16_hstatus;
 outputData->x=prms->fv_hvec.u.x;
 outputData->y=prms->fv_hvec.u.y;
 outputData->z=prms->fv_hvec.u.z;

//1229 vendor need debug
//please log out these data

 GMFLOAT radius = GMFS_SQRT(
			 (prms->fv_hvec.u.x * prms->fv_hvec.u.x) +
			 (prms->fv_hvec.u.y * prms->fv_hvec.u.y) +
			 (prms->fv_hvec.u.z * prms->fv_hvec.u.z));

 ALOGE("hh: haroc=%8.3f radius=%8.3f,(x,y,z)=(%8.3f,%8.3f,%8.3f),offset=(%8.3f,%8.3f,%8.3f),accuracy= %d\n",prms->s_aocv.hraoc,radius, prms->fv_hvec.u.x,prms->fv_hvec.u.y,prms->fv_hvec.u.z, 	prms->fv_ho.u.x,prms->fv_ho.u.y,prms->fv_ho.u.z,prms->i16_hstatus);
 	
 return 1;
 
}

int GMCCaliApiSetAccData(struct magCaliDataInPut *inputData)
{
	GMEPRMS *prms=&PRMS;
/*
	prms->fv_avec.v[0]=inputData->x;
	prms->fv_avec.v[1]=inputData->y;
	prms->fv_avec.v[2]=inputData->z;
*/
	int16 acc[3],tmp_accuracy;
	GMFLOAT tmpx,tmpy,tmpz;

	//ALOGE("hh:%s\n", __func__);
	acc[0]=inputData->x*1000;
	acc[1]=inputData->y*1000;
	acc[2]=inputData->z*1000;	
	GMFS_Get_ACCELEROMETER(prms, acc, 0, &tmpx, &tmpy, &tmpz, &tmp_accuracy);
	//ALOGE("hh:%s %8.3f %8.3f %8.3f\n", __func__, tmpx, tmpy, tmpz);
	return 1;
}

// for timestamp
int64_t elapsedRealtimeNanoHh()
{
	struct timespec ts;
	int err = clock_gettime(CLOCK_BOOTTIME, &ts);

	return (ts.tv_sec * NANOS_IN_SECOND) + ts.tv_nsec;
}

int GMCGetOrientation(struct magCaliDataOutPut *outputData)
{
   GMFLOAT tmpx, tmpy, tmpz;
   int16  accuracy;
   GMEPRMS *prms=&PRMS;

   //ALOGE("hh:GMCGetOrientation\n");
   GMFS_Get_ORIENTATION(prms,&tmpx,&tmpy,&tmpz,&accuracy);

   //outputData->timeStamp=android::elapsedRealtimeNano() + IGNORE_EVENT_TIME;
   outputData->timeStamp=elapsedRealtimeNanoHh() + 0;	// IGNORE_EVENT_TIME=0

   outputData->x=tmpx;//yaw
   outputData->y=tmpy;//pitch
   outputData->z=tmpz;//roll
   outputData->status=accuracy;

   //ALOGE("hh:%s %8.3f %8.3f %8.3f\n", __func__, tmpx, tmpy, tmpz);

   return 1;
}

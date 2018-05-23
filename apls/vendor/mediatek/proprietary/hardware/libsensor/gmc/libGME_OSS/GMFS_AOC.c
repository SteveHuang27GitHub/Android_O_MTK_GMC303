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
 *
 ******************************************************************************/
#include "GMFS_AOC.h"
#include "GMFS_Math.h"
#include "ossf_magnetic.h"
/*
 * CalcR
 */
static GMFLOAT CalcR(
	const	GMFVEC	*x,
	const	GMFVEC	*y
){
	int16	i;
	GMFLOAT	r;

	r = 0.0;
	for (i = 0; i < 3; i++) {
		r += (x->v[i]-y->v[i]) * (x->v[i]-y->v[i]);
	}
	r = sqrt(r);

	return r;
}

/*
 * From4Points2Sphere()
 */
static int16 From4Points2Sphere(
	const	GMFVEC		points[],	/*!< (i/o)	: input vectors  */
			GMFVEC		*center,	/*!< (o)	: center of sphere   */
			GMFLOAT		*r			/*!< (i)	: add/subtract value */
){
	GMFLOAT	dif[3][3];
	GMFLOAT	r2[3];

	GMFLOAT	A;
	GMFLOAT	B;
	GMFLOAT	C;
	GMFLOAT	D;
	GMFLOAT	E;
	GMFLOAT	F;
	GMFLOAT	G;

	GMFLOAT	OU;
	GMFLOAT	OD;

	int16	i, j;

	for (i = 0; i < 3; i++) {
		r2[i] = 0.0;
		for (j = 0; j < 3; j++) {
			dif[i][j] = points[i].v[j] - points[3].v[j];
			r2[i] += (points[i].v[j]*points[i].v[j]
					- points[3].v[j]*points[3].v[j]);
		}
		r2[i] *= 0.5;
	}

	A = dif[0][0]*dif[2][2] - dif[0][2]*dif[2][0];
	B = dif[0][1]*dif[2][0] - dif[0][0]*dif[2][1];
	C = dif[0][0]*dif[2][1] - dif[0][1]*dif[2][0];
	D = dif[0][0]*r2[2]		- dif[2][0]*r2[0];
	E = dif[0][0]*dif[1][1] - dif[0][1]*dif[1][0];
	F = dif[1][0]*dif[0][2] - dif[0][0]*dif[1][2];
	G = dif[0][0]*r2[1]		- dif[1][0]*r2[0];

	OU = D*E + B*G;
	OD = C*F + A*E;

	if (fabs(OD) < GMFS_EPSILON) {
		return -1;
	}

	center->v[2] = OU / OD;

	OU = F*center->v[2] + G;
	OD = E;

	if (fabs(OD) < GMFS_EPSILON) {
		return -1;
	}

	center->v[1] = OU / OD;

	OU = r2[0] - dif[0][1]*center->v[1] - dif[0][2]*center->v[2];
	OD = dif[0][0];

	if (fabs(OD) < GMFS_EPSILON) {
		return -1;
	}

	center->v[0] = OU / OD;

	*r = CalcR(&points[0], center);

	return 0;

}

/*
 * MeanVar
 */
static void MeanVar(
	const	GMFVEC	v[],	/*!< (i)   : input vectors */
	const	int16	n,		/*!< (i)   : number of vectors */
			GMFVEC	*mean,	/*!< (o)   : (max+min)/2 */
			GMFVEC	*var	/*!< (o)   : variation in vectors */
){
	int16	i;
	int16	j;
	GMFVEC	max;
	GMFVEC	min;

	for (j = 0; j < 3; j++) {
		min.v[j] = v[0].v[j];
		max.v[j] = v[0].v[j];
		for (i = 1; i < n; i++) {
			if (v[i].v[j] < min.v[j]) {
				min.v[j] = v[i].v[j];
			}
			if (v[i].v[j] > max.v[j]) {
				max.v[j] = v[i].v[j];
			}
		}
		mean->v[j] = (max.v[j] + min.v[j]) / 2.0;	/*mean */
		var->v[j] = max.v[j] - min.v[j];			/*var  */
	}
}

/*
 * Get4points
 */
static void Get4points(
	const	GMFVEC	v[],	/*!< (i)   : input vectors */
	const	int16	n,		/*!< (i)   : number of vectors */
			GMFVEC	out[]	/*!< (o)   : */
){
	int16	i, j;
	GMFLOAT temp;
	GMFLOAT d;

	GMFVEC	dv[GMFS_HBUF_SIZE];
	GMFVEC	cross = {{0, 0, 0}};
	GMFVEC	tempv = {{0, 0, 0}};

	/* out 0 */
	out[0] = v[0];

	/* out 1 */
	d = 0.0;
	for (i = 1; i < n; i++) {
		temp = CalcR(&v[i], &out[0]);
		if (d < temp) {
			d = temp;
			out[1] = v[i];
		}
	}

	/* out 2 */
	d = 0.0;
	for (j = 0; j < 3; j++) {
		dv[0].v[j] = out[1].v[j] - out[0].v[j];
	}
	for (i = 1; i < n; i++) {
		for (j = 0; j < 3; j++) {
			dv[i].v[j] = v[i].v[j] - out[0].v[j];
		}
		tempv.v[0] = dv[0].v[1]*dv[i].v[2] - dv[0].v[2]*dv[i].v[1];
		tempv.v[1] = dv[0].v[2]*dv[i].v[0] - dv[0].v[0]*dv[i].v[2];
		tempv.v[2] = dv[0].v[0]*dv[i].v[1] - dv[0].v[1]*dv[i].v[0];
		temp =	tempv.u.x * tempv.u.x
			  +	tempv.u.y * tempv.u.y
			  +	tempv.u.z * tempv.u.z;
		if (d < temp) {
			d = temp;
			out[2] = v[i];
			cross = tempv;
		}
	}

	/* out 3 */
	d = 0.0;
	for (i = 1; i < n; i++) {
		temp =	  dv[i].u.x * cross.u.x
				+ dv[i].u.y * cross.u.y
				+ dv[i].u.z * cross.u.z;
		temp = fabs(temp);
		if (d < temp) {
			d = temp;
			out[3] = v[i];
		}
	}
}

/*
 * CheckInitFvec
 */
static int16 CheckInitFvec(
	const	GMFVEC	*v		/*!< [in]	vector */
){
	int16 i;

	for (i = 0; i < 3; i++) {
		if (GMFS_FMAX <= v->v[i]) {
			return 1;       /* initvalue */
		}
	}

	return 0;       /* not initvalue */
}

/*
 * GMFS_AOC
 */
int16 GMFS_AOC(				/*!< (o) : calibration success(GMFS_SUCCESS), failure(GMFS_ERROR) */
			GMFS_AOC_VAR	*haocv,	/*!< (i/o)	: a set of variables */
	const	GMFVEC			*hdata,	/*!< (i)	: vectors of data    */
			GMFVEC			*ho		/*!< (i/o)	: offset             */
){
	int16	i, j;
	int16	num;
	GMFLOAT	tempf;
	GMFVEC	tempho;

	GMFVEC	fourpoints[4];

	GMFVEC	var;
	GMFVEC	mean;
#ifdef OSSF_CALIBRATION
	
	  /* Get number of datas */
	  num = 0;
	  for (i = OSSF_HBUF_SIZE; 0 < i; i--) {
		if (CheckInitFvec(&haocv->hbuf[i - 1]) == 0) {
		  num = i;
		  break;
		}
	  }
	
	  // Shift the buffer  
	  for (i = 1; i < OSSF_HBUF_SIZE; i++) {
		haocv->hbuf[OSSF_HBUF_SIZE-i] = haocv->hbuf[OSSF_HBUF_SIZE-i-1];
	  }
#else
	  // Shift the buffer
	  for (i = 1; i < GMFS_HBUF_SIZE; i++) {
		haocv->hbuf[GMFS_HBUF_SIZE-i] = haocv->hbuf[GMFS_HBUF_SIZE-i-1];
	  } 
#endif //OSSF_CALIBRATION
	
	  /* buffer new data */
	  haocv->hbuf[0] = *hdata;
		
#ifdef OSSF_CALIBRATION
	  num = (num >= OSSF_HBUF_SIZE)? OSSF_HBUF_SIZE: (num + 1); //the data has been buffered
#else
	  /* Check Init */
	  num = 0;
	  for (i = GMFS_HBUF_SIZE; 3 < i; i--) {
		if (CheckInitFvec(&haocv->hbuf[i-1]) == 0) {
		  num = i;
		  break;
		}
	  }
#endif //OSSF_CALIBRATION
	
	  if (num < 4) {
		return GMFS_ERROR;
	  }
	
	  /* get 4 points */
#ifdef OSSF_CALIBRATION
	
	  Get4points(haocv->hbuf, (num > GMFS_HBUF_SIZE ? OSSF_HBUF_GET4PTS_SIZE: num), fourpoints);
	
#else
	
	  Get4points(haocv->hbuf, num, fourpoints);
	
#endif //OSSF_CALIBRATION
	
	  /* estimate offset */
	  if (0 != From4Points2Sphere(fourpoints, &tempho, &haocv->hraoc)) {
		return GMFS_ERROR;
	  }
	
	  /* check distance */
	  for (i = 0; i < 4; i++) {
		for (j = (i+1); j < 4; j++) {
		  tempf = CalcR(&fourpoints[i], &fourpoints[j]);
		  if ((tempf < haocv->hraoc) || (tempf < GMFS_HR_TH)) {
		return GMFS_ERROR;
		  }
		}
	  }
	
#ifdef OSSF_CALIBRATION
	
	  ossfCalibration(haocv->hbuf, num, &tempho, &haocv->hraoc, &tempf);
	
#endif //OSSF_CALIBRATION
	
	  /* update offset buffer */
	  for (i = 1; i < GMFS_HOBUF_SIZE; i++) {
		haocv->hobuf[GMFS_HOBUF_SIZE-i] = haocv->hobuf[GMFS_HOBUF_SIZE-i-1];
	  }
	  haocv->hobuf[0] = tempho;
	
	  /* clear hbuf */
#ifdef OSSF_CALIBRATION 
	
	  for (i = OSSF_HBUF_SIZE - OSSF_CLEAR_BUF_SIZE; i < OSSF_HBUF_SIZE; i++) {
	
#else
		for (i = (GMFS_HBUF_SIZE>>1); i < GMFS_HBUF_SIZE; i++) {
	
#endif //OSSF_CALIBRATION
		  for (j = 0; j < 3; j++) {
		haocv->hbuf[i].v[j] = GMFS_FMAX;
		  }
		}
	
		/* Check Init */
		if (CheckInitFvec(&haocv->hobuf[GMFS_HOBUF_SIZE-1]) == 1) {
		  return GMFS_ERROR;
		}
	
		/* Check ovar */
		tempf = haocv->hraoc * GMFS_HO_TH;
		MeanVar(haocv->hobuf, GMFS_HOBUF_SIZE, &mean, &var);
		if ((var.u.x >= tempf) || (var.u.y >= tempf) || (var.u.z >= tempf)) {
		  return GMFS_ERROR;
		}
	
		*ho = mean;
	
		return GMFS_SUCCESS;
	  }


/*
 * GMFS_InitAOC
 */
void GMFS_InitAOC(
			GMFS_AOC_VAR	*haocv
){
	int16 i, j;

		/* Initialize buffer */
#ifdef OSSF_CALIBRATION
	
	for (i = 0; i < OSSF_HBUF_SIZE; i++) 
	{
		for (j = 0; j < 3; j++) {
			haocv->hbuf[i].v[j] = GMFS_FMAX;
		}
	}	
#else
	
	for (i = 0; i < GMFS_HBUF_SIZE; i++) 
	{
		for (j = 0; j < 3; j++) {
			haocv->hbuf[i].v[j] = GMFS_FMAX;
		}
	}	
#endif //OSSF_CALIBRATION

	for (i = 0; i < GMFS_HOBUF_SIZE; i++) {
		for (j = 0; j < 3; j++) {
			haocv->hobuf[i].v[j] = GMFS_FMAX;
		}
	}

	haocv->hraoc = 0.0;
}


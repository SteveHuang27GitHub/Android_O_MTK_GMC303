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
#ifndef GMFS_INC_COMMON_H
#define GMFS_INC_COMMON_H

#ifdef WIN32
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0501
#endif    					
  					
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <stdarg.h>
#include <crtdbg.h>
#include <tchar.h>
#include "../Android/Android.h"

//henry mask
//#define ENABLE_GMEDEBUG	1
#define ENABLE_GMEDEBUG	0

#else
#include <stdio.h>     /* frpintf */
#include <stdlib.h>    /* atoi */
#include <string.h>    /* memset */
#include <unistd.h>
#include <stdarg.h>    /* va_list */
#include <errno.h>     /* errno */

#endif

#include "GMFS_Log.h"

/*** Constant definition ******************************************************/
#define GME_TRUE	1	/*!< Represents true */
#define GME_FALSE	0	/*!< Represents false */
#define GME_SUCCESS	0	/*!< Represents success */
#define GME_ERROR	-1	/*!< Represents error */

#define OPMODE_CONSOLE		(0x01)
#define OPMODE_FST			(0x02)

/*** Type declaration *********************************************************/

/*** Global variables *********************************************************/
extern int g_stopRequest;	/*!< 0:Not stop,  1:Stop */
extern int g_opmode;		/*!< 0:Daemon mode, 1:Console mode. */
extern int g_dbgzone;		/*!< Debug zone. */

/*** Prototype of function ****************************************************/

#endif /* GMED_INC_GMCOMMON_H */


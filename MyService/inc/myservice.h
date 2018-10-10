#ifndef __myservice_H__
#define __myservice_H__

#include <dlog.h>
#include <glib.h>
#include <system_settings.h>
#include <efl_extension.h>
#include <dlog.h>
#include <sensor.h>
#include <stdio.h>
#include <power.h>
#include <string.h>


#ifdef  LOG_TAG
#undef  LOG_TAG
#endif
#define LOG_TAG "myservice"


#define TAG "MyService"

void     initialize_sap();
gboolean find_peers();
void 	 mex_send_number(int n, int length, gboolean is_secured);

#endif /* __myservice_H__ */

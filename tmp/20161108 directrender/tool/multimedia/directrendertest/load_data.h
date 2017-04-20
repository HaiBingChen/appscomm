
#ifndef __ATC_DTDEMO_LOADDATA_H__
#define __ATC_DTDEMO_LOADDATA_H__

#include <linux/types.h>
#include <glib.h>
#include "atcsurface.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*ATCHWDATARECFUNC)(__u8 *data, __u32 len, void *pvarg);

void *OpenDataLoad(char *szfilename);
bool StartDataLoad(void *inst);
bool DataLoadRegDataReceive(void *inst, ATCHWDATARECFUNC cb, void *pvarg);
bool StopDataLoad(void *inst);
bool CloseDataLoad(void *inst);

#ifdef __cplusplus
}
#endif


#endif /* __ATC_DTDEMO_LOADDATA_H__ */


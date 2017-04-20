
#ifndef _ATC_3RDPARTY_VDEC_INST_H_
#define _ATC_3RDPARTY_VDEC_INST_H_

#include <OMX_Video.h>

typedef struct {
  OMX_VIDEO_CODINGTYPE eVType; /* video codec type */
  __s32 width;        /* video width */
  __s32 height;       /* video height */
  __s32 fps_n;        /* numerator of video framerate */
  __s32 fps_d;        /* denominator of video framerate */
  bool  interlaced;   /* designate whether video data is interaced  */
} ATC_VDEC_INPUT_FMT_INFO_T;

typedef struct _AtcVdecOutputDataInfo{
  void *buffer;       /* output buffer  */
  __u32 bufSz;        /* output buffer size */
  __u32 datasz;       /* valid data size in output buffer  */
  __s32 width;        /* video width */
  __s32 height;       /* video height */
  __s64 timestampus;  /* video frame timestamp, unit: us */
} ATC_VDEC_OUTPUT_DATA_INFO_T;

typedef enum {
  RET_ATCVDECINST_OK,            /* success */
  RET_ATCVDECINST_PARAM_ERR,     /* parameter error */
  RET_ATCVDECINST_NO_MEM,        /* no memory */
  RET_ATCVDECINST_WRONG_STATE,   /* state is error */
  RET_ATCVDECINST_COMPONENT_ERR, /* omx component encounter error */
  RET_ATCVDECINST_FLUSHING,      /* in flushing */
  RET_ATCVDECINST_FAIL,          /* other error */
} ATC_VDEC_ERRCODE;

#ifdef __cplusplus
extern "C" {
#endif

void *atc_vdec_open(void);

bool  atc_vdec_set_input_format(void *inst, ATC_VDEC_INPUT_FMT_INFO_T *format);

bool  atc_vdec_get_output_formats(void *inst,
  __u64 *psupport_fmts, __u32 *pformat);

bool  atc_vdec_decode(void *inst, __u8 *pdata, __u32 datasz, __s64 timestampus);

bool  atc_vdec_close(void *inst);

bool  atc_vdec_start(void *inst);

ATC_VDEC_ERRCODE atc_vdec_get_output_data(void * inst,
  ATC_VDEC_OUTPUT_DATA_INFO_T *prOutInfo);


bool  atc_vdec_stop(void *inst);

#ifdef __cplusplus
}
#endif


#endif /* _ATC_3RDPARTY_VDEC_INST_H_ */


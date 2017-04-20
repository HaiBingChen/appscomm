
#ifndef _ATC_3RDPARTY_VSINK_H_
#define _ATC_3RDPARTY_VSINK_H_

#include "atcsurface.h"

typedef struct {
  __s32 use_count; /* buffer count used, min_count <= use_count <= max_count */
  __s32 max_count; /* max buffer count can be used */
  __s32 min_count; /* min buffer count can be used */
  __s32 width;        /* video width */
  __s32 height;       /* video height */
  __u32 stride;       /* video stride */
  __u64 support_fmts; /* video sink support formats, it must >= ((__u64)1 << format) */
  __u32 format;       /* video sink current format, value refers to ATC_PIX_FMT_.. defined in atcsurface.h */
  __u32 fourcc;       /* video sink current format fourcc */
  bool  interlaced;   /* designate whether video data is interaced  */
} ATC_VSINK_CFG_T;

typedef struct {
  __s32 width;        /* video width */
  __s32 height;       /* video height */
  __u32 stride;       /* video stride */
  __u32 format;       /* video sink current format, value refers to ATC_PIX_FMT_.. defined in atcsurface.h */
  __u32 fourcc;       /* video sink current format fourcc */
  bool  interlaced;   /* designate whether video data is interaced  */
} ATC_VSINK_FMT_INFO_T;

#ifdef __cplusplus
extern "C" {
#endif

void *atc_video_sink_open(void);
bool atc_video_sink_set_surface(void *inst, void *surface);
bool atc_video_sink_get_config(void *inst, ATC_VSINK_CFG_T *prCfg);
bool atc_video_sink_set_format (void *inst, ATC_VSINK_FMT_INFO_T *prFormat);
bool atc_video_sink_set_buffer_count(void *inst, __s32 count);
bool atc_video_sink_start(void *inst);
bool atc_video_sink_dequeue_buffer(void *inst,
  void **ppBuffer, __u32 *pBufSz);
bool atc_video_sink_queue_buffer(void *inst, void *pbuffer, __u32 bufsz,
  __u32 datasz);
bool atc_video_sink_cancel_buffer(void *inst, void *pbuffer, __u32 bufsz);
bool atc_video_sink_stop(void *inst);
bool atc_video_sink_close(void *inst);

#ifdef __cplusplus
}
#endif

#endif /* _ATC_3RDPARTY_VSINK_H_ */



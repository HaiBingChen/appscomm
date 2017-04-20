#include <linux/types.h>
#include <QTimer>
#include <QMessageBox>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>
#include "ui_videoplay.h"
#include "dtdemo_videoplay.h"
#include "atcomxvdecinst.h"
#include "atcvideosink.h"
#include "atcsurface.h"
#include <syslog.h>

extern void exit_app(void);
extern "C" pid_t  gettid(void);

#define ATC_DTDEMO_MOD_NAME   "dtdemo"
#define ATC_DTDEMO_MAIN        16
#define ATC_DTDEMO_MINOR       10
#define ATC_DTDEMO_REV         26


//33ms
#define DEFAULT_WAIT_TIME (((int64_t)G_USEC_PER_SEC) / 30)
#define DTDEMO_MSECOND (((int64_t)G_USEC_PER_SEC) / 1000)
#define DTDEMO_WAIT_INFINITE ((int64_t)(-1))
#define DTDEMO_WAIT_REPLY 1

void DataReceived(__u8 *data, __u32 len, void *pvarg);
void *voutthread_proc(void *parg);

videoPlay::videoPlay(QWidget *parent) :
    QWidget(parent),
    m_parent(parent),
    ui(new Ui::videoPlay),
    video_surface(NULL),
    is_voutthread_created(FALSE)
{
    ui->setupUi(this);
    pthread_mutex_init(&video_surface_lock, NULL);

    manager = NULL;

    syslog(LOG_NOTICE, "[VER][%s] V%02d.%02d_%02d\r\n",
      ATC_DTDEMO_MOD_NAME, ATC_DTDEMO_MAIN,
      ATC_DTDEMO_MINOR, ATC_DTDEMO_REV);
}

videoPlay::~videoPlay()
{
  syslog(LOG_NOTICE, "[DTDEMO]++++++++++++++++~VideoPlay:Enter+++++++++++++\n");
  if (NULL != manager) {
    if (manager->loaddatainst) {
      CloseDataLoad(manager->loaddatainst);
      manager->loaddatainst = NULL;
    }
    if (manager->atcvdecinst) {
      atc_vdec_close(manager->atcvdecinst);
      manager->atcvdecinst = NULL;
    }
    if (manager->atcvsinkinst) {
      atc_video_sink_close(manager->atcvsinkinst);
      manager->atcvsinkinst = NULL;
    }

    if (NULL != manager->video_surface) {
      IAtcSurface_release(manager->video_surface);
      manager->video_surface = NULL;
    }

    if (NULL != manager->cmd_queue) {
      g_async_queue_unref(manager->cmd_queue);
      manager->cmd_queue = NULL;
    }
    
    syslog(LOG_NOTICE, "[DTDEMO] %s --> free manager\r\n",
      __FUNCTION__);
    g_free(manager);
    manager = NULL;
  }

  pthread_mutex_destroy(&video_surface_lock);
  syslog(LOG_NOTICE, "[DTDEMO]++++++++++++++++~VidepPlay:Leave+++++++++++++\n");
  delete ui;
}

void videoPlay::fromdtdemo(QString video_file)
{
  syslog(LOG_NOTICE, "[DTDEMO]fromdtdemo::Enter() -------------------\n");
  video_filePath = video_file;
  syslog(LOG_NOTICE, "[DTDEMO]fromdtdemo::Exit() -------------------\n");
}

void videoPlay::sendCmd(E_DTDEMO_CMD_T eCmd)
{
  DTDEMO_CMD_T *cmd = NULL;

  if (NULL == manager) {
    return;
  }

  if (manager->need_stop) {
    syslog(LOG_ERR, "[mmatewayland] sendCmd exit, now need stop\r\n");
    return;
  }

  cmd = (DTDEMO_CMD_T *)g_malloc0(sizeof(DTDEMO_CMD_T));
  if (NULL == cmd) {
    syslog(LOG_ERR, "[mmatewayland] sendCmd fail for no memory\r\n");
    return;
  }
  cmd->eCmd = eCmd;
  g_async_queue_push(manager->cmd_queue, (gpointer)cmd);
}

void videoPlay::do_exit_window(void)
{
  syslog(LOG_NOTICE, "[DTDEMO]****videoPlay %s()  enter *****\r\n",
    __FUNCTION__);

  if (NULL != manager) {
    if (NULL != manager->video_surface) {
      IAtcSurface_hide(manager->video_surface);
    }
    syslog(LOG_NOTICE, "[mmatewayland]****videoPlay %s() sendCmd(DTDEMO_CMD_EXIT) *****\r\n",
      __FUNCTION__);
    sendCmd(DTDEMO_CMD_EXIT);
    if (manager->loaddatainst) {
      StopDataLoad(manager->loaddatainst);
    }
    if (manager->atcvdecinst) {
      atc_vdec_stop(manager->atcvdecinst);
    }
    if (manager->atcvsinkinst) {
      atc_video_sink_stop(manager->atcvsinkinst);
    }
  }

  if (is_voutthread_created) {
    pthread_join(vout_thread, NULL);
    is_voutthread_created = NULL;
  }

  if (NULL != manager) {
    if (manager->loaddatainst) {
      CloseDataLoad(manager->loaddatainst);
      manager->loaddatainst = NULL;
    }
    if (manager->atcvdecinst) {
      atc_vdec_close(manager->atcvdecinst);
      manager->atcvdecinst = NULL;
    }
    if (manager->atcvsinkinst) {
      atc_video_sink_close(manager->atcvsinkinst);
      manager->atcvsinkinst = NULL;
    }

    if (NULL != manager->video_surface) {
      IAtcSurface_release(manager->video_surface);
      manager->video_surface = NULL;
    }

    if (NULL != manager->cmd_queue) {
      g_async_queue_unref(manager->cmd_queue);
      manager->cmd_queue = NULL;
    }
    
    syslog(LOG_NOTICE, "[DTDEMO] %s --> free manager\r\n",
      __FUNCTION__);
    g_free(manager);
  }
  manager = NULL;
  
  hide();
  
  syslog(LOG_NOTICE, "[DTDEMO]^^^^^^^^^^^^^^^^VideoSurface release:Leave^^^^^^^^^^^^^^\n");
  
  //delete this;
  syslog(LOG_NOTICE, "[DTDEMO]****videoPlay %s()  exit *****\r\n",
   __FUNCTION__);
}

void videoPlay::on_playBtn_clicked()
{
    int j = 0;
    int err = 0;
    syslog(LOG_NOTICE, "[DTDEMO] on_playBtn_clicked enter\r\n");
    qDebug() << "[DTDEMO] on_playBtn_clicked, videopath: " << video_filePath;
    
    QByteArray ch = video_filePath.toLatin1();
    char *szfilename = ch.data();
       
    if (NULL == manager) {
      syslog(LOG_NOTICE, "[mmatewayland] on_playBtn_clicked, allocate MMateThreadMan\r\n");
      manager = (DTDEMOMAN *)g_malloc0(sizeof(DTDEMOMAN));
      if (NULL == manager) {
        syslog(LOG_ERR, "[mmatewayland] %s line %d fail in allocate ate thread man\r\n",
          __FUNCTION__, __LINE__);
        return;
      }

      memset(manager, 0, sizeof(DTDEMOMAN));

      manager->cmd_queue = g_async_queue_new();
      if (NULL == manager->cmd_queue) {
        syslog(LOG_ERR, "[mmatewayland] %s line %d fail in create nfy queue\r\n",
          __FUNCTION__, __LINE__);
        return;
      }

      manager->loaddatainst = OpenDataLoad(szfilename);
      if (NULL == manager->loaddatainst) {
        syslog(LOG_NOTICE, "[DTDEMO] on_playBtn_clicked fail in OpenDataLoad\r\n");
        return;
      }
      
      manager->atcvdecinst = atc_vdec_open();
      if (NULL == manager->atcvdecinst) {
        syslog(LOG_NOTICE, "[DTDEMO] on_playBtn_clicked fail in atc_vdec_open\r\n");
        return;
      }
      manager->atcvsinkinst = atc_video_sink_open();
      if (NULL == manager->atcvsinkinst) {
        syslog(LOG_NOTICE, "[DTDEMO] on_playBtn_clicked fail in atc_video_sink_open\r\n");
        return;
      }

      if (NULL == manager->video_surface) {
        manager->video_surface = atc_createsurface(ATCSURF_TYPE_DEFAULT,
          1920, 1080,
          ATC_PIX_FMT_NV12M_PRIVATE1);
        if (NULL == manager->video_surface) {
          syslog(LOG_NOTICE, "[DTDEMO] %s fail in CreateVideoSurface****\r\n",
            __FUNCTION__);
          return;
        }
        
        IAtcSurface_setLayerZOrder(manager->video_surface, 3);
      
        QRect v_rc = ui->videoWidget->geometry();
        QPoint v_pos = ui->videoWidget->pos();
        QPoint v_parent_pos2 = ui->videoWidget->mapToGlobal(v_pos);
        IAtcSurface_setWindow(manager->video_surface,
          v_parent_pos2.x(), v_parent_pos2.y() + 26,
          v_rc.width(), v_rc.height());
      }
    }
    
    is_voutthread_created = TRUE;
    err = pthread_create(&vout_thread,
      NULL, voutthread_proc, manager);
    if (err != 0) {
      syslog(LOG_ERR, "[demo] %s line %d create thread fail because of %s\r\n",
        __func__, __LINE__, strerror(errno));
      is_voutthread_created = FALSE;
      return;
    }
    
    syslog(LOG_NOTICE, "[DTDEMO] on_playBtn_clicked exit\n");
    return;
}

void videoPlay::on_exitBtn_clicked()
{
  syslog(LOG_NOTICE, "[DTDEMO]*******videoPlay::on_exit_clicked begin *******\n");
  do_exit_window();
  syslog(LOG_NOTICE, "[DTDEMO]--------------------------------------close Leave--------------------------\n");
  //exit_app();
  syslog(LOG_NOTICE, "[DTDEMO]*******videoPlay::on_exit_clicked end *******\n");
}


void DataReceived(__u8 *data, __u32 len, void *pvarg)
{
  syslog(LOG_NOTICE, "[DTDEMO] %s atc_vdec_decode(%p, %p, %d)\r\n",
    __FUNCTION__, pvarg, data, len);
  if (!atc_vdec_decode(pvarg, data, len, (__s64)(-1))) {
    syslog(LOG_NOTICE, "[DTDEMO] %s fail in atc_vdec_decode\r\n",
      __FUNCTION__);
  }
}

void *voutthread_proc(void *parg)
{
  DTDEMOMAN *manager = (DTDEMOMAN *)parg;
  ATC_VSINK_FMT_INFO_T rSinkformat;
  ATC_VSINK_CFG_T rSinkCfg;
  __u64 support_out_formats = 0;
  __u32 outfmt = 0;

  memset(&rSinkformat, 0, sizeof(rSinkformat));
  memset(&rSinkCfg, 0, sizeof(rSinkCfg));

  syslog(LOG_NOTICE, "[DTDEMO] %s enter, manager: %p(loaddata: %p, vdec: %p, vsink: %p)\r\n",
    __FUNCTION__, manager, manager->loaddatainst, 
    manager->atcvdecinst, manager->atcvsinkinst);

  if (NULL != manager->loaddatainst) {
    syslog(LOG_NOTICE, "[DTDEMO] %s -- DataLoadRegDataReceive\r\n",
      __FUNCTION__);
    if (!DataLoadRegDataReceive(manager->loaddatainst, DataReceived,
      manager->atcvdecinst)){
      syslog(LOG_NOTICE, "[DTDEMO] %s fail in DataLoadRegDataReceive(%p)\r\n",
        __FUNCTION__, manager->loaddatainst);
      return NULL;
    }
  }
  
  syslog(LOG_NOTICE, "[DTDEMO] %s -- atc_video_sink_set_surface****\r\n",
    __FUNCTION__);
  if (NULL != manager->atcvsinkinst) {
    if (!atc_video_sink_set_surface(manager->atcvsinkinst,
      manager->video_surface)) {
      syslog(LOG_NOTICE, "[DTDEMO] %s fail in atc_video_sink_set_surface****\r\n",
        __FUNCTION__);
      return NULL;
    }
  }

  syslog(LOG_NOTICE, "[DTDEMO] %s -- atc_video_sink_get_config\r\n",
    __FUNCTION__);
  if (!atc_video_sink_get_config(manager->atcvsinkinst, &rSinkCfg)) {
    syslog(LOG_NOTICE, "[DTDEMO] %s fail in atc_vdec_start(%p)\r\n",
      __FUNCTION__, manager->atcvdecinst);
    return NULL;
  }
  if (!atc_vdec_get_output_formats(manager->atcvdecinst,
    &support_out_formats, &outfmt)) {
    syslog(LOG_NOTICE, "[DTDEMO] %s fail in atc_vdec_get_output_formats(%p)\r\n",
      __FUNCTION__, manager->atcvdecinst);
    return NULL;
  }

  if (0 == ((rSinkCfg.support_fmts) & support_out_formats)) {
    syslog(LOG_NOTICE, "[DTDEMO] %s fail for videosink's support formats(0x%llx) are not intersect with vdec support output formats(0x%llx)\r\n",
      __FUNCTION__, rSinkCfg.support_fmts, support_out_formats);
    return NULL;
  }
  
  if (0 == ((rSinkCfg.support_fmts) &(1 << outfmt))) {
    syslog(LOG_NOTICE, "[DTDEMO] %s don't support the format(%d), (support_fmts: 0x%llx), vdecinst: %p\r\n",
      __FUNCTION__, outfmt, rSinkCfg.support_fmts, manager->atcvdecinst);
    return NULL;
  }

  syslog(LOG_NOTICE, "[DTDEMO] %s vsink's buffers (min: %d, max: %d, use: %d)\r\n",
    __FUNCTION__, rSinkCfg.min_count, rSinkCfg.max_count, rSinkCfg.use_count);
  
  syslog(LOG_NOTICE, "[DTDEMO] %s -- atc_video_sink_set_buffer_count(%d)\r\n",
    __FUNCTION__, rSinkCfg.use_count);
  if (!atc_video_sink_set_buffer_count(manager->atcvsinkinst, rSinkCfg.use_count)) {
    syslog(LOG_NOTICE, "[DTDEMO] %s fail in atc_video_sink_set_buffer_count(%p, %d)\r\n",
      __FUNCTION__, manager->atcvdecinst, rSinkCfg.use_count);
    return NULL;
  }

  memset(&rSinkformat, 0, sizeof(rSinkformat));
  rSinkformat.format = ATC_PIX_FMT_NV12M_PRIVATE1;
  rSinkformat.width  = 1920;
  rSinkformat.height = 1080;
  rSinkformat.stride = (1920 + 15) / 16 * 16;
  rSinkformat.interlaced = FALSE;
  rSinkformat.fourcc = 0;

  syslog(LOG_NOTICE, "[DTDEMO] %s -- atc_video_sink_set_format\r\n",
    __FUNCTION__);
  if (!atc_video_sink_set_format(manager->atcvsinkinst, &rSinkformat)) {
    syslog(LOG_NOTICE, "[DTDEMO] %s fail in atc_vdec_start(%p)\r\n",
      __FUNCTION__, manager->atcvdecinst);
    return NULL;
  }

  ATC_VDEC_INPUT_FMT_INFO_T rVdecInputFmt;
  
  memset(&rVdecInputFmt, 0, sizeof(rVdecInputFmt));
  rVdecInputFmt.eVType = OMX_VIDEO_CodingAVC;
  rVdecInputFmt.width  = 1920;
  rVdecInputFmt.height = 1080;
  rVdecInputFmt.interlaced = FALSE;
  rVdecInputFmt.fps_d = 1;
  rVdecInputFmt.fps_n = 30;
  syslog(LOG_NOTICE, "[DTDEMO] %s -- atc_vdec_set_input_format\r\n",
    __FUNCTION__);
  if (!atc_vdec_set_input_format(manager->atcvdecinst, &rVdecInputFmt)) {
    syslog(LOG_NOTICE, "[DTDEMO] %s fail in atc_vdec_set_input_format(%p)\r\n",
      __FUNCTION__, manager->atcvdecinst);
    return NULL;
  }

  syslog(LOG_NOTICE, "[DTDEMO] %s -- atc_vdec_start\r\n",
    __FUNCTION__);
  if (!atc_vdec_start(manager->atcvdecinst)) {
    syslog(LOG_NOTICE, "[DTDEMO] %s fail in atc_vdec_start(%p)\r\n",
      __FUNCTION__, manager->atcvdecinst);
    return NULL;
  }
  
  syslog(LOG_NOTICE, "[DTDEMO] %s -- atc_video_sink_start\r\n",
    __FUNCTION__);
  if (!atc_video_sink_start(manager->atcvsinkinst)) {
    syslog(LOG_NOTICE, "[DTDEMO] %s fail in atc_video_sink_start(%p)\r\n",
      __FUNCTION__, manager->atcvsinkinst);
    return NULL;
  }
  
  syslog(LOG_NOTICE, "[DTDEMO] %s -- StartDataLoad\r\n",
    __FUNCTION__);
  if (!StartDataLoad(manager->loaddatainst)) {
    syslog(LOG_NOTICE, "[DTDEMO] %s fail in StartDataLoad(%p)\r\n",
      __FUNCTION__, manager->loaddatainst);
    return NULL;
  }

  void *pvOutbuf = NULL;
  __u32 OutBufLen = 0;
  ATC_VDEC_OUTPUT_DATA_INFO_T rVdecOutInfo;
  DTDEMO_CMD_T *cmd = NULL;
  
  while (TRUE) {
    cmd = (DTDEMO_CMD_T *)g_async_queue_timeout_pop(manager->cmd_queue, DEFAULT_WAIT_TIME);

    if (cmd != NULL) {
      if (cmd->eCmd = DTDEMO_CMD_EXIT) {
        syslog(LOG_NOTICE, "[DTDEMO] %s, DTDEMO_CMD_EXIT\r\n",
          __FUNCTION__);
        break;
      }
    }

    pvOutbuf = NULL;
    OutBufLen = 0;
    syslog(LOG_NOTICE, "[DTDEMO] %s -- atc_video_sink_dequeue_buffer\r\n",
      __FUNCTION__);
    if (!atc_video_sink_dequeue_buffer(manager->atcvsinkinst, &pvOutbuf, &OutBufLen)) {
      syslog(LOG_NOTICE, "[DTDEMO] %s fail in atc_video_sink_dequeue_buffer(%p)\r\n",
        __FUNCTION__, manager->atcvsinkinst);
      break;
    }

    memset(&rVdecOutInfo, 0, sizeof(rVdecOutInfo));
    rVdecOutInfo.buffer = pvOutbuf;
    rVdecOutInfo.bufSz = OutBufLen;
    rVdecOutInfo.datasz = 0;
    rVdecOutInfo.width = rVdecInputFmt.width;
    rVdecOutInfo.height = rVdecInputFmt.height;
    rVdecOutInfo.timestampus = 0;

    if (manager->need_stop) {
      syslog(LOG_NOTICE, "[DTDEMO] %s line %d -- atc_video_sink_cancel_buffer\r\n",
        __FUNCTION__, __LINE__);
      if (!atc_video_sink_cancel_buffer(manager->atcvsinkinst,
        pvOutbuf, OutBufLen)) {
        syslog(LOG_NOTICE, "[DTDEMO] %s fail in atc_video_sink_cancel_buffer(%p, dz: %d)\r\n",
          __FUNCTION__, pvOutbuf, OutBufLen);
      }
      break;
   }
    
    syslog(LOG_NOTICE, "[DTDEMO] %s -- atc_vdec_get_output_data(%p, bufsz: %d)\r\n",
      __FUNCTION__, pvOutbuf, OutBufLen);
    if (RET_ATCVDECINST_OK != atc_vdec_get_output_data(manager->atcvdecinst,
      &rVdecOutInfo)) {
      syslog(LOG_NOTICE, "[DTDEMO] %s line %d fail in atc_vdec_get_output_data\r\n",
        __FUNCTION__, __LINE__);
      if (!atc_video_sink_cancel_buffer(manager->atcvsinkinst,
        pvOutbuf, OutBufLen)) {
        syslog(LOG_NOTICE, "[DTDEMO] %s fail in atc_video_sink_cancel_buffer(%p, dz: %d)\r\n",
          __FUNCTION__, pvOutbuf, OutBufLen);
      }
      continue;
    }

    if (0 == rVdecOutInfo.datasz) {
      syslog(LOG_NOTICE, "[DTDEMO] %s line %d -- datasz = 0, cancel buffer\r\n",
        __FUNCTION__, __LINE__);
      if (!atc_video_sink_cancel_buffer(manager->atcvsinkinst,
        pvOutbuf, OutBufLen)) {
        syslog(LOG_NOTICE, "[DTDEMO] %s fail in atc_video_sink_cancel_buffer(%p, dz: %d)\r\n",
          __FUNCTION__, pvOutbuf, OutBufLen);
      }
      continue;
    }

    if ((rVdecOutInfo.width != rSinkformat.width) ||
      (rVdecOutInfo.height != rSinkformat.height)) {
      rSinkformat.format = ATC_PIX_FMT_NV12M_PRIVATE1;
      rSinkformat.width  = rVdecOutInfo.width;
      rSinkformat.height = rVdecOutInfo.height;
      rSinkformat.stride = (rVdecOutInfo.width + 15) / 16 * 16;
      rSinkformat.interlaced = FALSE;
      rSinkformat.fourcc = 0;
      
      syslog(LOG_NOTICE, "[DTDEMO] %s -- atc_video_sink_set_format\r\n",
        __FUNCTION__);
      if (!atc_video_sink_set_format(manager->atcvsinkinst, &rSinkformat)) {
        syslog(LOG_NOTICE, "[DTDEMO] %s fail in atc_vdec_start(%p)\r\n",
          __FUNCTION__, manager->atcvdecinst);
        return NULL;
      }
    }

    syslog(LOG_NOTICE, "[DTDEMO] %s -- atc_video_sink_queue_buffer(%p, sz: %d)\r\n",
      __FUNCTION__, pvOutbuf, rVdecOutInfo.datasz);
    if (!atc_video_sink_queue_buffer(manager->atcvsinkinst, pvOutbuf, OutBufLen,
      rVdecOutInfo.datasz)) {
      syslog(LOG_NOTICE, "[DTDEMO] %s fail in atc_video_sink_queue_buffer(%p, dz: %d)\r\n",
        __FUNCTION__, pvOutbuf, rVdecOutInfo.datasz);
      break;
    }
  }
  
  syslog(LOG_NOTICE, "[DTDEMO] %s exit\r\n",
    __FUNCTION__);
  return NULL;
}



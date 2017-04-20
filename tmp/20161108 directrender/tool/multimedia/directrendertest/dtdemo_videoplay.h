#ifndef VIDEOPLAY_H
#define VIDEOPLAY_H

#include <linux/types.h>
#include <QWidget>
#include <QMainWindow>
#include <QTextBrowser>
#include <QTime>
#include <QTimer>
#include <QString>
#include <QFile>
#include <QDebug>
#include <QtCore>
#include <QTextStream>
#include <QPainter>
#include <QFontDatabase>
#include <QThread>
#include <sys/types.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <locale.h>
#include <stdlib.h>
#include <glib.h>

#include <atcsurface.h>
#include "atcvideosink.h"
#include "atcomxvdecinst.h"
#include "load_data.h"

namespace Ui {

  
class videoPlay;
}

typedef enum {
  DTDEMO_CMD_UNKNOWN,
  DTDEMO_CMD_EXIT,
  DTDEMO_CMD_MAX,
} E_DTDEMO_CMD_T;

typedef struct {
  E_DTDEMO_CMD_T eCmd;
} DTDEMO_CMD_T;

typedef struct {
  void *loaddatainst;
  void *atcvdecinst;
  void *atcvsinkinst;
  bool  need_stop;
  GAsyncQueue *cmd_queue;
  IAtcSurface *video_surface;
} DTDEMOMAN;

class videoPlay : public QWidget
{
    Q_OBJECT

public:
    explicit videoPlay(QWidget *parent = 0);
    ~videoPlay();
    int setMediafile(const char *filename);
    void do_exit_window(void);
    
    static ATCHWDATARECFUNC DataReceived(__u8 *data, __u32 len);

    void sendCmd(E_DTDEMO_CMD_T eCmd);
    
private slots:

    void on_exitBtn_clicked();

    void on_playBtn_clicked();

    void fromdtdemo(QString video_file);

public :
  
    IAtcSurface     *video_surface;
    pthread_mutex_t video_surface_lock;

    QWidget *m_parent;

private:
    Ui::videoPlay   *ui;

    QString         video_filePath;

    DTDEMOMAN  *manager;
    bool      is_voutthread_created;
    pthread_t vout_thread;
};



#endif // VIDEOPLAY_H

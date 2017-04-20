#ifndef MMATE_H
#define MMATE_H

#include <QMainWindow>
#include <QString>
#include <QFile>
#include <QDebug>
#include <QtCore>
#include <QTextStream>
#include <QPainter>
#include <QFontDatabase>
#include <sys/types.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <locale.h>
#include <stdlib.h>
#include <glib.h>

#include "widget_directory.h"
#include "dtdemo_videoplay.h"

namespace Ui {
class DtDemo;
}

class DtDemo : public QMainWindow
{
    Q_OBJECT

public:
    explicit DtDemo(QWidget *parent = 0);
    ~DtDemo();



signals:
    void tovideoPlay(QString file_path);

private slots:
    void on_selectFile_txt_clicked();

    void receiveFile_txtPath(const QString &pathSelected);

    void on_exit_clicked();

    void on_start_clicked();
    
private:
    Ui::DtDemo          *ui;
    videoPlay           *VideoPlay;
   	DirectoryWidget 	  *directory;
    QString             test_video_file;
};

#endif // MMATE_H

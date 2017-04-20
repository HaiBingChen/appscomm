#include "dtdemo.h"
#include "ui_dtdemo.h"
#include <QMessageBox>

extern void exit_app(void);
extern "C" pid_t  gettid(void);

DtDemo::DtDemo(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::DtDemo),
    VideoPlay(NULL)
{
  ui->setupUi(this);
}

DtDemo::~DtDemo()
{
  qDebug()<<"~DtDemo(), delete ui";
  if (VideoPlay)
    delete VideoPlay;
  VideoPlay = NULL;
  delete ui;
}

void DtDemo::on_selectFile_txt_clicked()
{
	directory = new DirectoryWidget(this);
  connect(directory, SIGNAL(toMainWindow(QString)), this, SLOT(receiveFile_txtPath(QString)));
  directory->setGeometry(100, 100, 764, 390);
  directory->setAttribute(Qt::WA_ShowModal, true);
  directory->show();
}

void DtDemo::receiveFile_txtPath(const QString &pathSelected)
{
  test_video_file = pathSelected;
  ui->filePath->setText(test_video_file);
  qDebug()<< "&&&&&&&&&&&&&&&&&&&&&&&&"<<pathSelected;
  qDebug()<<"&&&&&&&&&&&&&&&&&&&&&&&"<< test_video_file;
}

void DtDemo::on_start_clicked()
{
  if (NULL == VideoPlay)
    VideoPlay = new videoPlay(this);

  connect(this, SIGNAL(tovideoPlay(QString)), VideoPlay, SLOT(fromdtdemo(QString)));
  emit tovideoPlay(test_video_file);
  VideoPlay->setGeometry(100, 100, 764, 390);
  VideoPlay->setAttribute(Qt::WA_ShowModal, true);
  VideoPlay->show();
}

void DtDemo::on_exit_clicked()
{
	qDebug("DtDemo::on_exit_clicked enter!");
#ifndef WITH_SOAPP
    qDebug("DtDemo::on_exit_clicked --> QApplication::exit()!");
    QApplication::exit();
#else
    qDebug("DtDemo::on_exit_clicked --> exit_app()!");
    exit_app();
    qDebug("DtDemo::on_exit_clicked --> delete this!");
    delete this;
#endif
}


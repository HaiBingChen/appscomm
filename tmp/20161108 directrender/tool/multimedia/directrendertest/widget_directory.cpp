#include "widget_directory.h"
#include "ui_directorywidget.h"
#include <QStorageInfo>
#include <QDebug>

DirectoryWidget::DirectoryWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DirectoryWidget)
{
    ui->setupUi(this);

    rootPath = QStorageInfo::root().rootPath();

    QTreeWidgetItem* root = new QTreeWidgetItem(QStringList(rootPath));
    ui->treeWidget->addTopLevelItem(root);

    QDir dir(rootPath);
    QFileInfoList fileInfoList = dir.entryInfoList();

    foreach(QFileInfo info, fileInfoList)
    {
        if(info.fileName() == "." || info.fileName() == "..")
        {
            continue;
        }
        else
        {
            QTreeWidgetItem * item=new QTreeWidgetItem(QStringList(info.fileName()));
            root->addChild(item);
        }
    }
    root->setExpanded(true);
}

DirectoryWidget::~DirectoryWidget()
{
    delete ui;
}

QString DirectoryWidget::getItemFullPath(QTreeWidgetItem *item)
{
    QString path = item->text(0);

    if(path == rootPath)
    {
        return rootPath;
    }
    else
    {
        QString parent = item->parent()->text(0);

        while(parent != rootPath)
        {
            path = parent + "/" + path;
            item = item->parent();
            parent = item->parent()->text(0);
        }

        return rootPath + path;
    }
}

void DirectoryWidget::on_treeWidget_itemExpanded(QTreeWidgetItem *item)
{
    for (int i=0; i<item->childCount(); i++)
    {
        QTreeWidgetItem* childItem = item->child(i);
        QString path = getItemFullPath(childItem);
        QFile file(path);
        QFileInfo fileInfo(file);
        if(fileInfo.isDir())
        {
            QDir dir(path);
            QFileInfoList fileInfoList = dir.entryInfoList();

            foreach(QFileInfo info, fileInfoList)
            {
                if(info.fileName() == "." || info.fileName() == "..")
                {
                    continue;
                }
                else
                {
                    QTreeWidgetItem * grandchild = new QTreeWidgetItem(QStringList(info.fileName()));
                    childItem->addChild(grandchild);
                }
            }
        }
    }
}

void DirectoryWidget::on_OKButton_clicked()
{
    const QString path = getItemFullPath(ui->treeWidget->currentItem());
    emit toMainWindow(path);
    this->close();

}

void DirectoryWidget::on_CancelButton_clicked()
{
    //const QString path = NULL;
    //emit toMainWindow(path);
    this->close();
}


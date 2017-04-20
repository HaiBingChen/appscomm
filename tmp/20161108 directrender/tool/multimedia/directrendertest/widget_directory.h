#ifndef DIRECTORYWIDGET_H
#define DIRECTORYWIDGET_H

#include <QWidget>
#include <QTreeWidgetItem>

namespace Ui {
class DirectoryWidget;
}

class DirectoryWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DirectoryWidget(QWidget *parent = 0);
    ~DirectoryWidget();

    QString getItemFullPath(QTreeWidgetItem *item);
signals:
    void toMainWindow(const QString &pathSelected);

private slots:

    void on_treeWidget_itemExpanded(QTreeWidgetItem *item);

    void on_OKButton_clicked();

    void on_CancelButton_clicked();

private:
    Ui::DirectoryWidget *ui;

    QString             rootPath;
};

#endif // DIRECTORYWIDGET_H

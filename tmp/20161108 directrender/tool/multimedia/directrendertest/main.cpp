#include <QTimer>
#include "dtdemo.h"
#include <QApplication>

#ifndef WITH_SOAPP
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    DtDemo w;
    w.show();

    return a.exec();
}

#else
typedef void(*soapp_exit_handler)(void *handle, void *arg);

static soapp_exit_handler   g_appExitHandler;
static void*                g_appHandle;

extern "C" int so_main(int argc, char *argv[], soapp_exit_handler exit_handler, void *handle, void *param)
{
    qDebug("multimedia directrender demo test app so_main entry, argc: %d\n", argc);

    DtDemo *w = new DtDemo();

    if (!w) {
        return (-1);
    }

    w->show();

    g_appExitHandler = exit_handler;
    g_appHandle = handle;

    return (0);
}
#endif
void exit_app(void) {
    qDebug("exit multimedia directrender demo test app, begin\n");
#ifdef WITH_SOAPP
    if (g_appExitHandler) {
        g_appExitHandler(g_appHandle, NULL);
    }
#endif

    qDebug("exit multimedia directrender demo test app, end\n");
}

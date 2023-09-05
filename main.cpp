#include <QCoreApplication>

#include "PjSipWorker.hpp"
#include <signal.h>

template <int ... SIGNAL>
void catchSignals(void(*HANDLER)(int)) {
    auto hs = {signal(SIGNAL, HANDLER) ...};
    static_cast<void>(hs);

    signal(SIGCHLD, SIG_IGN);
    signal(SIGPIPE, SIG_IGN);
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    PjSipWorker testWorker;
    //____инициализация после проверки на успешность инстанцирования pjsip
    if(testWorker.libGetState())
        testWorker.startCallTests();

    catchSignals<SIGINT, SIGTERM, SIGKILL>([](int){
        qDebug() << "App quit!";
        qApp->quit();
    });

    return a.exec();
}

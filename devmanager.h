#ifndef DEVMANAGER_H
#define DEVMANAGER_H
#include <QThread>
#include "k4adevice.h"

#define N_CAM 4

class devManager : public QThread
{
    Q_OBJECT
public:
    devManager();
    ~devManager();

    k4aDevice* k4aDevices[N_CAM];
    void begin();
    void stop();

private:
    void run() override;
    bool _is_running;

signals:


};

#endif // DEVMANAGER_H

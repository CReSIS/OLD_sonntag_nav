#include <QTextStream>
#include <QSerialPort>
#include <QTimer>
#include <QCoreApplication>

#include "gps.h"

QSerialPort * serial;
GPS gps;

void loop() {
  if (serial->bytesAvailable()) {
    uint8_t byte;
    serial->read((char*)&byte, 1);
    if (gps.ingest(byte)) {
      switch (gps.get_message_id()) {
        case 42:
          {
            BestPos pos = gps.parse_bestpos();
            QTextStream(stdout) << pos.latitude << "\t" << pos.longitude << "\t" << pos.sol_age << endl;
          }
          break;
        case 99:
          {
            BestVel vel = gps.parse_bestvel();
            QTextStream(stdout) << vel.hor_spd << "\t" << vel.trk_gnd << endl;
          }
          break;
        case 101:
          {
            Time time = gps.parse_time();
            QTextStream(stdout) << time.utc_year << "-" << time.utc_month << "-" << time.utc_day << "\t" << time.utc_hour << ":" << time.utc_min << ":" << time.utc_ms << endl;
          }
          break;
        default:
          break;
      }
    }
    //QTextStream(stdout) << bit << endl;
  }
}

int main(int argc, char *argv[]) {
  QCoreApplication app(argc, argv);

  serial = new QSerialPort("/dev/ttyUSB1");
  serial->open(QIODevice::ReadWrite);

  QTimer *timer = new QTimer();
  QObject::connect(timer, &QTimer::timeout, &loop);
  timer->start();

  return app.exec();
}

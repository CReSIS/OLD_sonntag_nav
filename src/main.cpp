//
// To avoid having to type password in:
// sudo chmod a+rwx ./sonntag_nav; sudo chown root:root ./sonntag_nav; sudo chmod u+s ./sonntag_nav;
// 

#include <QTextStream>
#include <QSerialPort>
#include <QTimer>
#include <QCoreApplication>

#include <fstream>

#include "gps.h"
#include "nav.h"

QSerialPort * serial;
GPS gps;
Nav * nav;

void loop() {
  if (serial->bytesAvailable()) {
    uint8_t byte;
    serial->read((char*)&byte, 1);

    std::ofstream fout;
    fout.open("gps_stream.bin", std::ios::binary | std::ios::app);
    fout.write((char*)&byte, sizeof(byte));
    fout.close();

    if (gps.ingest(byte)) {
      switch (gps.get_message_id()) {
        case 42:
          {
            BestPos pos = gps.parse_bestpos();
            QTextStream(stdout) << pos.latitude << "\t" << pos.longitude << "\t" << pos.sol_age << "\n";
            nav->update(pos);
          }
          break;
        case 99:
          {
            BestVel vel = gps.parse_bestvel();
            QTextStream(stdout) << vel.hor_spd << "\t" << vel.trk_gnd << "\n";
            nav->update(vel);
          }
          break;
        case 101:
          {
            Time time = gps.parse_time();
            QTextStream(stdout) << time.utc_year << "-" << time.utc_month << "-" << time.utc_day << "\t" << time.utc_hour << ":" << time.utc_min << ":" << time.utc_ms << "\n";
            nav->update(time);
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
  QCoreApplication::setSetuidAllowed(true);

  QCoreApplication app(argc, argv);

  nav = new Nav("localhost", 4040, &app); // Original
  //nav = new Nav("aq-field18", 20050, &app); //TEST

  serial = new QSerialPort("/dev/ttyUSB0");
  serial->open(QIODevice::ReadWrite);

  QTimer *parseTimer = new QTimer();
  QObject::connect(parseTimer, &QTimer::timeout, &loop);
  parseTimer->start();

  QTimer *navTimer = new QTimer();
  navTimer->setInterval(1000);
  QObject::connect(navTimer, &QTimer::timeout, nav, &Nav::send);
  navTimer->start();

  return app.exec();
}

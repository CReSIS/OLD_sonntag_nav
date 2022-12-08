//
// To avoid having to type password in:
// sudo chmod a+rwx ./sonntag_nav; sudo chown root:root ./sonntag_nav; sudo chmod u+s ./sonntag_nav;
// 

#include <QTextStream>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QTimer>
#include <QCoreApplication>

#include <QString>
#include <QDateTime>

#include <fstream>

#include <stdlib.h>

#include "gps.h"
#include "nav.h"

QSerialPort * serial;
GPS gps;
Nav * nav;

int time_valid;
QString filename;

//QString * timestamp;

void loop() {
  if (serial->bytesAvailable()) {
    uint8_t byte;
    serial->read((char*)&byte, 1);

    if (time_valid > 5) {
      std::ofstream fout;
      fout.open(filename.toLocal8Bit().constData(), std::ios::binary | std::ios::app);
      fout.write((char*)&byte, sizeof(byte));
      fout.close();
    }

    if (gps.ingest(byte)) {
        if (gps.get_message_id() != 0) {
            QString test;
            if (time_valid > 5) {
            //test = QString("Test %1%2%3").arg((int)2022,4,10,QLatin1Char('0')).arg(7,2,10,QLatin1Char('0')).arg(12,2,10,QLatin1Char('0'));
            //printf("FILENAME: [%s]\n", test.toLocal8Bit().constData());
            //printf("FILENAME: [%s]\n", filename.toLocal8Bit().constData());
            }
          //QTextStream(stdout) << __FILE__ << ":" << __LINE__ << ": " << gps.get_message_id() << " " << gps.get_header_len() << " " << gps.get_message_len() << "\n";
        }
      switch (gps.get_message_id()) {
        case 42:
          {
            BestPos pos = gps.parse_bestpos();
            //QTextStream(stdout) << __FILE__ << ":" << __LINE__ << ": \n";
            QTextStream(stdout) << "BESTPOS: Lat: " << pos.latitude << " Lon: " << pos.longitude  << " Height: " << pos.height << " Solution Age: " << pos.sol_age << "\n";
            //QTextStream(stdout) << __FILE__ << ":" << __LINE__ << ": \n";
            //nav->update(pos);
          }
          break;
        case 99:
          {
            //BestVel vel = gps.parse_bestvel();
            //QTextStream(stdout) << vel.hor_spd << "\t" << vel.trk_gnd << "\n";
            //nav->update(vel);
          }
          break;
        case 101:
          {
            Time time = gps.parse_time();
            QTextStream(stdout) << "TIME Status: " << time.utc_stat << " " << "Year: " << time.utc_year << " Month: " << time.utc_month << " Day:" << time.utc_day << " " << time.utc_hour << ":" << time.utc_min << ":" << ((double)time.utc_ms)/1000 << "\n";
            if (time.utc_stat == 1 && time_valid < 5) {
                time_valid++;
            }
            if (time.utc_stat == 1 && time_valid == 5) {
                time_valid = 6;
                QTextStream(stdout) << "**************** FOUND SUFFICIENT VALID TIMES TO ENSURE OLD BUFFER FLUSHED **************" << "\n";
                filename = QString("/data/GPS_Novatel_raw_%1%2%3_%4%5%6.bin")
                        .arg((int)time.utc_year,4,10,QLatin1Char('0'))
                        .arg((char)time.utc_month,2,10,QLatin1Char('0'))
                        .arg((char)time.utc_day,2,10,QLatin1Char('0'))
                        .arg((char)time.utc_hour,2,10,QLatin1Char('0'))
                        .arg((char)time.utc_min,2,10,QLatin1Char('0'))
                        .arg((int)time.utc_ms/1000,2,10,QLatin1Char('0'));
                //filename = QString("/data/OEM617D_Novatel_GPS_raw_%1%2%3_%4%5%6.bin").arg((int)40,4,'0').arg((char)39,2,'0').arg((char)38,2,'0').arg((char)time.utc_hour,2,'0').arg((char)time.utc_min,2,'0').arg((int)time.utc_ms/1000,2,'0');

                QTextStream(stdout) << "**************** RECORDING STARTING " << filename << " **************" << "\n";

                QTextStream(stdout) << "**************** SETTING SYSTEM DATE AND TIME TO GPS TIME **************" << "\n";
                QString set_date_cmd;
                set_date_cmd = QString("sudo date -u -s \"%1/%2/%3 %4:%5:%6\"")
                        .arg((char)time.utc_month,2,10,QLatin1Char('0'))
                        .arg((char)time.utc_day,2,10,QLatin1Char('0'))
                        .arg((int)time.utc_year,4,10,QLatin1Char('0'))
                        .arg((char)time.utc_hour,2,10,QLatin1Char('0'))
                        .arg((char)time.utc_min,2,10,QLatin1Char('0'))
                        .arg((int)time.utc_ms/1000,2,10,QLatin1Char('0'));
                QTextStream(stdout) << set_date_cmd << "\n";
                int ret_val;
                ret_val = system(set_date_cmd.toLocal8Bit().constData());
                QTextStream(stdout) << "  Return value: " << ret_val << "\n";


            }
            //nav->update(time);
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
  time_valid = 0;
  int port_valid = 0;

  QString port_system_location = "/dev/ttyUSB0";

  QTextStream out(stdout);
  out  << "**************** SYSTEM SERIAL PORT INFO **************" << "\n";
        const auto serialPortInfos = QSerialPortInfo::availablePorts();

        out << "Total number of serial ports available: " << serialPortInfos.count() << endl << endl;

        out << "Listing serial ports:" << endl << endl;

        const QString blankString = "N/A";
        QString description;
        QString manufacturer;
        QString serialNumber;

        for (const QSerialPortInfo &serialPortInfo : serialPortInfos) {
            description = serialPortInfo.description();
            manufacturer = serialPortInfo.manufacturer();
            serialNumber = serialPortInfo.serialNumber();


            if (description == "Novatel GPS Receiver" && port_valid == 0) {
                port_valid = 1;
              out << "******************** USING THIS PORT ************************" << endl;
              port_system_location = serialPortInfo.systemLocation();
            }

            out << "Port: " << serialPortInfo.portName() << endl
                << "Location: " << serialPortInfo.systemLocation() << endl
                << "Description: " << (!description.isEmpty() ? description : blankString) << endl
                << "Manufacturer: " << (!manufacturer.isEmpty() ? manufacturer : blankString) << endl
                << "Serial number: " << (!serialNumber.isEmpty() ? serialNumber : blankString) << endl
                << "Vendor Identifier: " << (serialPortInfo.hasVendorIdentifier()
                                             ? QByteArray::number(serialPortInfo.vendorIdentifier(), 16)
                                             : blankString) << endl
                << "Product Identifier: " << (serialPortInfo.hasProductIdentifier()
                                              ? QByteArray::number(serialPortInfo.productIdentifier(), 16)
                                              : blankString) << endl
                << "Busy: " << (serialPortInfo.isBusy() ? "Yes" : "No") << endl << endl;
        }

  //QDateTime now = QDateTime::currentDateTime();

  //timestamp = new QString(now.toString(QLatin1String("yyyyMMdd_hhmmss")));

  //nav = new Nav("localhost", 4040, &app); // Original
  //nav = new Nav("aq-field18", 20050, &app); //TEST

        if (port_valid == 0) {
            out << "WARNING: DID NOT FIND VALID NOVATEL PORT!" << endl;
        }

  out  << "**************** OPENING SERIAL PORT " << port_system_location << " **************" << endl;

  serial = new QSerialPort(port_system_location);
  serial->open(QIODevice::ReadWrite);
  serial->readAll();

  out  << "**************** STARTING TO READ SERIAL PORT **************" << endl;
  QTimer *parseTimer = new QTimer();
  QObject::connect(parseTimer, &QTimer::timeout, &loop);
  parseTimer->start();

  // QTimer *navTimer = new QTimer();
  // navTimer->setInterval(1000);
  // QObject::connect(navTimer, &QTimer::timeout, nav, &Nav::send);
  // QTextStream(stdout) << "Test\n";
  // navTimer->start();
  // QTextStream(stdout) << "Test\n";

  int ret_val = app.exec();

  out  << "**************** EXITING **************" << "\n";

  return ret_val;
}

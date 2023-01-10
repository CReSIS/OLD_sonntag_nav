//
// To avoid having to type password in:
// sudo chmod a+rwx /arena/sonntag_nav/sonntag_nav; sudo chown root:root /arena/sonntag_nav/sonntag_nav; sudo chmod u+s /arena/sonntag_nav/sonntag_nav;
// sudo chmod a+rwx /bin/date; sudo chown root:root /bin/date; sudo chmod u+s /bin/date;
// 

#include <QTextStream>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QTimer>
#include <QCoreApplication>
#include <QProcessEnvironment>
#include <QSysInfo>

#include <QString>
#include <QDateTime>

#include <QTcpSocket>
#include <QTcpServer>
#include <QtNetwork>
#include <QHostAddress>

#include <fstream>

#include <stdlib.h>
#include <time.h>
#include <math.h>

#include "gps.h"
#include "messages.h"

QTcpServer *nav_server;
QTcpSocket *nav_socket = NULL;
bool server_done = false;

QSerialPort * serial;
GPS gps;

int time_valid;
QString filename;

time_t last_good_time = 0;
time_t last_good_time_error = 0;

int TIME_VALID_THRESHOLD = 5;
double HEADING_OFFSET = 0.0;

BestPos pos;
BestVel vel;
BestTime gps_time;
Heading heading;
double best_vel_heading = 0.0;

void loop() {

    time_t cur_time;
    time(&cur_time);
    if (time_valid > TIME_VALID_THRESHOLD && ((double)cur_time) > ((double)last_good_time)+2.1 && cur_time > last_good_time_error + 1) {
        QTextStream(stdout) << "\n" << "ERROR!!!!\nBAD FOR " << cur_time - last_good_time << " SECONDS." << "\n\n";
        last_good_time_error = cur_time;
    }

    if (nav_socket != NULL && nav_socket->state() == QAbstractSocket::UnconnectedState) {
        QTextStream(stdout) << "  Disconnected\n";
        delete(nav_socket);
        nav_socket = NULL;
        server_done = false;
    }
    if (nav_server->hasPendingConnections()) {
        if (!server_done) {
            QTextStream(stdout) << "Pending connection\n";
            server_done = true;

            nav_socket = nav_server->nextPendingConnection();

            if (nav_socket->state() == QAbstractSocket::ConnectedState) {
                QTextStream(stdout) << "  Connected\n";
            } else {
                QTextStream(stdout) << "  Failed to connect\n";
            }
        } else {
            QTextStream(stdout) << "Pending connection, but already busy.\n";
        }

    }

    if (serial->bytesAvailable()) {
        uint8_t byte;
        serial->read((char*)&byte, 1);

        if (time_valid > TIME_VALID_THRESHOLD) {
            std::ofstream fout;
            fout.open(filename.toLocal8Bit().constData(), std::ios::binary | std::ios::app);
            fout.write((char*)&byte, sizeof(byte));
            fout.close();
        }

        if (gps.ingest(byte)) {
            if (gps.get_message_id() != 0) {
                QString test;
                if (time_valid > TIME_VALID_THRESHOLD) {
                }
            }
            switch (gps.get_message_id()) {
            case 42:
            {
                pos = gps.parse_bestpos();

                QString msg = QString("\nBESTPOS: Lat: %1 Lon: %2 Elev: %3 Age: %4 Sats: %5-%6-%7-%8 Status: %9 Error Lat: %10 Lon: %11 Elev: %12\n")
                        .arg((double)pos.latitude,8,'f',4,QLatin1Char(' '))
                        .arg((double)pos.longitude,9,'f',4,QLatin1Char(' '))
                        .arg((double)pos.height,5,'f',1,QLatin1Char(' '))
                        .arg((float)pos.sol_age,1,'g',0,QLatin1Char(' '))
                        .arg((uint8_t)pos.tracked_sats,2,10,QLatin1Char(' '))
                        .arg((uint8_t)pos.solution_sats,2,10,QLatin1Char(' '))
                        .arg((uint8_t)pos.sol_L1_sats,2,10,QLatin1Char(' '))
                        .arg((uint8_t)pos.sol_multi_sats,2,10,QLatin1Char(' '))
                        .arg((uint32_t)pos.sol_stat,1,10,QLatin1Char(' '))
                        .arg((double)pos.lat_stdev,4,'f',1,QLatin1Char(' '))
                        .arg((double)pos.long_stdev,4,'f',1,QLatin1Char(' '))
                        .arg((double)pos.hgt_stdev,4,'f',1,QLatin1Char(' '));

                QTextStream(stdout) << msg;
                time(&last_good_time);
                break;
            }
            case 99:
            {
                vel = gps.parse_bestvel();
                if (fabs(vel.hor_spd) < 1 && fabs(vel.vert_spd) < 1) {
                    vel.trk_gnd = best_vel_heading;
                } else {
                    best_vel_heading = vel.trk_gnd;
                }
                if (!isnan(HEADING_OFFSET) && heading.heading != 0) {
                    QString msg = QString("BESTVEL+HEADING: hor %1 m/s at %2 deg   vert %3 m/s at %4 deg\n")
                            .arg((double)vel.hor_spd,3,'f',0,QLatin1Char(' '))
                            .arg(fmodf(heading.heading+HEADING_OFFSET+180.0,360.0)-180.0,3,'f',0,QLatin1Char(' '))
                            .arg((double)vel.vert_spd,4,'f',1,QLatin1Char(' '))
                            .arg((double)heading.pitch,4,'f',1,QLatin1Char(' '));
                    QTextStream(stdout) << msg;
                    //QTextStream(stdout) << "BESTVEL+HEADING: " << vel.hor_spd << " m/s at " << fmodf(heading.heading+HEADING_OFFSET+180.0,360.0)-180.0 << " deg. Vert vel: " << vel.vert_spd << " m/s at " << heading.pitch << " deg.\n";
                } else {
                    QString msg = QString("BESTVEL: hor %1 m/s at %2 deg   vert %3 m/s\n")
                            .arg((double)vel.hor_spd,3,'f',0,QLatin1Char(' '))
                            .arg((double)vel.trk_gnd,3,'f',0,QLatin1Char(' '))
                            .arg((double)vel.vert_spd,4,'f',1,QLatin1Char(' '));
                    QTextStream(stdout) << msg;
                    //QTextStream(stdout) << "BESTVEL: " << vel.hor_spd << " m/s at " << vel.trk_gnd << " deg. Vert vel: " << vel.vert_spd << " m/s\n";
                }
                if (nav_socket != NULL && nav_socket->state() == QAbstractSocket::ConnectedState) {
                    if (!isnan(HEADING_OFFSET) && heading.heading != 0) {
                        QString output =
                                QString::asprintf("11,%04d%02d%02d,%02d%02d%06.3f,%f,%f,%f,%f,%f,%f\n",
                                                  (int)gps_time.utc_year, (char)gps_time.utc_month, (char)gps_time.utc_day,
                                                  (char)gps_time.utc_hour, (char)gps_time.utc_min, ((double)gps_time.utc_ms)/1000.0,
                                                  pos.latitude, pos.longitude, pos.height*3.28083989501, // 3.28... to convert metres->feet
                                                  fmodf(heading.heading+HEADING_OFFSET+180.0,360.0)-180.0, vel.hor_spd*1.94384449, vel.vert_spd*196.850393701 //1.94... to convert m/s->knots, 196.85... to convert m/s->fpm
                                                  );
                        QTextStream(stdout) << "  To nav: " << output << "\n";
                        nav_socket->write(output.toUtf8());
                    } else {
                        QString output =
                                QString::asprintf("11,%04d%02d%02d,%02d%02d%06.3f,%f,%f,%f,%f,%f,%f\n",
                                                  (int)gps_time.utc_year, (char)gps_time.utc_month, (char)gps_time.utc_day,
                                                  (char)gps_time.utc_hour, (char)gps_time.utc_min, ((double)gps_time.utc_ms)/1000.0,
                                                  pos.latitude, pos.longitude, pos.height*3.28083989501, // 3.28... to convert metres->feet
                                                  vel.trk_gnd, vel.hor_spd*1.94384449, vel.vert_spd*196.850393701 //1.94... to convert m/s->knots, 196.85... to convert m/s->fpm
                                                  );
                        QTextStream(stdout) << "  To nav: " << output << "\n";
                        nav_socket->write(output.toUtf8());
                    }
                } else {
                    QTextStream(stdout) << "  Nav not connected\n";
                }
            }
                break;
            case 101:
            {
                gps_time = gps.parse_time();

                QString msg = QString("TIME: Status: %1 %2/%3/%4 %5:%6:%7\n")
                        .arg(gps_time.utc_stat,2,10,QLatin1Char(' '))
                        .arg(gps_time.utc_day,2,10,QLatin1Char('0'))
                        .arg(gps_time.utc_month,2,10,QLatin1Char('0'))
                        .arg(gps_time.utc_year,4,10,QLatin1Char('0'))
                        .arg(gps_time.utc_hour,2,10,QLatin1Char('0'))
                        .arg(gps_time.utc_min,2,10,QLatin1Char('0'))
                        .arg(((double)gps_time.utc_ms)/1000.0,4,'f',1,QLatin1Char('0'));
                QTextStream(stdout) << msg;

                //QTextStream(stdout) << "TIME Status: " << gps_time.utc_stat << " " << "Year: " << gps_time.utc_year << " Month: " << gps_time.utc_month << " Day:" << gps_time.utc_day << " " << gps_time.utc_hour << ":" << gps_time.utc_min << ":" << ((double)gps_time.utc_ms)/1000 << "\n";
                if (gps_time.utc_stat == 1 && time_valid < TIME_VALID_THRESHOLD) {
                    time_valid++;
                }
                if (gps_time.utc_stat == 1 && time_valid == TIME_VALID_THRESHOLD) {
                    time_valid = TIME_VALID_THRESHOLD+1;

                    QString machine_hostname = QSysInfo::machineHostName();

                    QTextStream(stdout) << "**************** FOUND SUFFICIENT VALID TIMES TO ENSURE OLD BUFFER FLUSHED **************" << "\n";
                    filename = QString("/data/GPS_Novatel_raw_%7_%1%2%3_%4%5%6.gps")
                            .arg((int)gps_time.utc_year,4,10,QLatin1Char('0'))
                            .arg((char)gps_time.utc_month,2,10,QLatin1Char('0'))
                            .arg((char)gps_time.utc_day,2,10,QLatin1Char('0'))
                            .arg((char)gps_time.utc_hour,2,10,QLatin1Char('0'))
                            .arg((char)gps_time.utc_min,2,10,QLatin1Char('0'))
                            .arg((int)gps_time.utc_ms/1000,2,10,QLatin1Char('0'))
                            .arg(machine_hostname);

                    QTextStream(stdout) << "**************** RECORDING STARTING " << filename << " **************" << "\n";

                    QTextStream(stdout) << "**************** SETTING SYSTEM DATE AND TIME TO GPS TIME **************" << "\n";
                    QString set_date_cmd;
                    set_date_cmd = QString("date -u -s \"%1/%2/%3 %4:%5:%6\"")
                            .arg((char)gps_time.utc_month,2,10,QLatin1Char('0'))
                            .arg((char)gps_time.utc_day,2,10,QLatin1Char('0'))
                            .arg((int)gps_time.utc_year,4,10,QLatin1Char('0'))
                            .arg((char)gps_time.utc_hour,2,10,QLatin1Char('0'))
                            .arg((char)gps_time.utc_min,2,10,QLatin1Char('0'))
                            .arg((int)gps_time.utc_ms/1000,2,10,QLatin1Char('0'));
                    QTextStream(stdout) << set_date_cmd << "\n";
                    int ret_val;
                    ret_val = system(set_date_cmd.toLocal8Bit().constData());
                    QTextStream(stdout) << "  Return value: " << ret_val << "\n";


                }
                break;
            }
            case 971:
            case 2042:
            {
                heading = gps.parse_heading();
                // QTextStream(stdout) << "HEADING Heading: " << heading.heading << " deg Pitch: " << heading.pitch << " deg\n";
                break;
            }
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

    // Example for getting environment variables:
    //QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    //QString h1 = env.value("HOSTNAME", QString("default"));
    //QTextStream(stdout) << "h1 " << env.toStringList().value(0) << "\n";

    // Nav server: open listening port
    nav_server = new QTcpServer();
    QString host_name = "localhost";
    QHostAddress host;
    host.setAddress(host_name);
    int port = 4040;
    QTextStream(stdout) << "Trying to listen on " << host_name << ": " << port << "\n";
    nav_server->listen(host, port);
    QTextStream(stdout) << "Listening\n";

    QTextStream out(stdout);

    if (argc >= 3) {
        try {
            HEADING_OFFSET = QString(argv[2]).toDouble();
        } catch (void *exception) {
            out  << "Second argument interpretation failure. Setting HEADING_OFFSET to default value of 0.0." << "\n";
            HEADING_OFFSET = 0.0;
        }
        if (HEADING_OFFSET < -360 || HEADING_OFFSET > 360) {
            out  << "HEADING_OFFSET out of range. Setting HEADING_OFFSET to default value of 0.0." << "\n";
            HEADING_OFFSET = 0.0;
        }
    } else {
        HEADING_OFFSET = 0.0;
    }
    out  << "HEADING_OFFSET: " << HEADING_OFFSET << "\n\n";
    if (argc >= 2) {
        try {
            TIME_VALID_THRESHOLD = QString(argv[1]).toInt();
        } catch (void *exception) {
            out  << "First argument interpretation failure. Setting TIME_VALID_THRESHOLD to default value of 5." << "\n";
            TIME_VALID_THRESHOLD = 5;
        }
        if (TIME_VALID_THRESHOLD < 1 || TIME_VALID_THRESHOLD > 100) {
            out  << "TIME_VALID_THRESHOLD out of range. Setting TIME_VALID_THRESHOLD to default value of 5." << "\n";
            TIME_VALID_THRESHOLD = 5;
        }
    } else {
        TIME_VALID_THRESHOLD = 5;
    }
    out  << "TIME_VALID_THRESHOLD: " << TIME_VALID_THRESHOLD << "\n\n";

    time_valid = 0;
    int port_valid = 0;

    QString port_system_location = "/dev/ttyUSB0";

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


        if ((description == "Novatel GPS Receiver" || description == "NovAtel GPS Receiver") && port_valid == 0) {
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
    if (port_valid == 0) {
        out << "WARNING: DID NOT FIND VALID NOVATEL PORT!" << endl;
    }

    out  << "**************** OPENING SERIAL PORT " << port_system_location << " **************" << endl;
    serial = new QSerialPort(port_system_location);
    //serial->setBaudRate(230400); // Not needed?
    serial->open(QIODevice::ReadWrite);
    serial->readAll();
    out << "BAUD: " << serial->baudRate() << endl;

    out  << "**************** STARTING TO READ SERIAL PORT **************" << endl;
    QTimer *parseTimer = new QTimer();
    QObject::connect(parseTimer, &QTimer::timeout, &loop);
    parseTimer->start();

    int ret_val = app.exec();

    out  << "**************** EXITING **************" << "\n";

    return ret_val;
}

#include "nav.h"

Nav::Nav(QString host, int port, QObject *parent) : QObject(parent) {
  socket = new QTcpSocket(this);
  QTextStream(stdout) << "Trying to connect to " << host << ": " << port << "\n";
  socket->connectToHost(host, port);
  QTextStream(stdout) << "waitForConnected\n";
  socket->waitForConnected();
  QTextStream(stdout) << "Connected\n";
}

void Nav::update(BestPos pos) {
  this->pos = pos;
}

void Nav::update(BestVel vel) {
  this->vel = vel;
}

void Nav::update(Time time) {
  this->time = time;
}

void Nav::send() {
  if (socket->state() == QAbstractSocket::ConnectedState) {
    QString output = 
      QString::asprintf("11,%04d%02d%02d,%02d%02d%02d.%01d,%f,%f,%f,%f,%f,%f\n",
      time.utc_year, time.utc_month, time.utc_day,
      time.utc_hour, time.utc_min, time.utc_ms/1000, time.utc_ms/100%10,
      pos.latitude, pos.longitude, pos.height*3.28083989501, // 3.28... to convert metres->feet
      vel.trk_gnd, vel.hor_spd*1.94384449, vel.vert_spd*196.850393701 //1.94... to convert m/s->knots, 196.85... to convert m/s->fpm
      );
    socket->write(output.toUtf8());
  } else {
  QTextStream(stdout) << "Not connected yet\n";
    //throw std::range_error("TCP socket not connected");
  }
}

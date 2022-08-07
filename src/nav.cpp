#include "nav.h"

Nav::Nav(QString host, int port, QObject *parent) : QObject(parent) {
  socket = new QTcpSocket(this);
  socket->connectToHost(host, port);
  socket->waitForConnected();
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
      pos.latitude, pos.longitude, pos.height,
      vel.trk_gnd, vel.hor_spd, vel.vert_spd
      );
    socket->write(output.toUtf8());
  } else {
    throw "socket not connected";
  }
}
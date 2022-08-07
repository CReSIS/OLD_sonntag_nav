#pragma once

#include <QString>
#include <QtNetwork>
#include "messages.h"

class Nav : public QObject
{
private:
  BestPos pos;
  BestVel vel;
  Time time;

  QTcpSocket *socket;
public:
  Nav(QString host, int port, QObject *parent = nullptr);
  ~Nav() {};

  void update(BestPos pos);
  void update(BestVel vel);
  void update(Time time);
public slots:
  void send();
};

#ifndef MAPWIDGET_H
#define MAPWIDGET_H

/*
 OSMScout - a Qt backend for libosmscout and libosmscout-map
 Copyright (C) 2010  Tim Teulings

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <QQuickPaintedItem>

#include <osmscout/GeoCoord.h>
#include <osmscout/util/GeoBox.h>

#include "DBThread.h"
#include "SearchLocationModel.h"

class MapWidget : public QQuickPaintedItem
{
  Q_OBJECT
  Q_PROPERTY(double lat READ GetLat)
  Q_PROPERTY(double lon READ GetLon)

private:
  osmscout::GeoCoord           center;
  double                       angle;
  osmscout::Magnification      magnification;

  // Drag and drop
  int                          startX;
  int                          startY;
  osmscout::MercatorProjection startProjection;

  // Controlling rerendering...
  bool                          requestNewMap;
  bool                          quickZooming;
  double                        quickZoomFactor;
  double                        quickMoveX;
  double                        quickMoveY;

signals:
  void TriggerMapRenderingSignal();
  void latChanged();
  void lonChanged();

public slots:
  void initialisationFinished(const DatabaseLoadedResponse& response);
  void redraw();
  void zoomQuick(double zoomFactor);
  void zoom(double zoomFactor, double dx=0, double dy=0);
  void move(double x, double y);
  void moveQuick(double x, double y);
  void left();
  void right();
  void up();
  void down();
  void rotateLeft();
  void rotateRight();
  void setRotation(double degrees);
  void showCoordinates(double lat, double lon);
  void showLocation(Location* location);
  double geoToPixelX(double lon, double lat);
  double geoToPixelY(double lon, double lat);
  void pixelToGeo(double x, double y, double& lon, double& lat);
  double pixelToGeoLon(double x, double y);
  double pixelToGeoLat(double x, double y);
  double distanceToPixels(double distance);
  bool isValid();

private:
  void TriggerMapRendering();

  void HandleMouseMove(QMouseEvent* event);

public:
  MapWidget(QQuickItem* parent = 0);
  virtual ~MapWidget();

  Q_INVOKABLE void reopenMap(void);
  Q_INVOKABLE bool isRendering(void)
  {
      DBThread* dbThread = DBThread::GetInstance();
      return (dbThread->IsRendering());
  }

  inline double GetLat() const
  {
      return center.GetLat();
  }

  inline double GetLon() const
  {
      return center.GetLon();
  }

  /*void mousePressEvent(QMouseEvent* event);
  void mouseMoveEvent(QMouseEvent* event);
  void mouseReleaseEvent(QMouseEvent* event);
  void wheelEvent(QWheelEvent* event);
*/
  void paint(QPainter *painter);
};

#endif

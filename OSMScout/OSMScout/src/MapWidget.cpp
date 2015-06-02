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

#include "MapWidget.h"

#include <iostream>

//! We rotate in 16 steps
static double DELTA_ANGLE=2*M_PI/16.0;

MapWidget::MapWidget(QQuickItem* parent)
    : QQuickPaintedItem(parent),
      center(0.0,0.0),
      angle(0.0),
      magnification(64),
      requestNewMap(true)

{
    setOpaquePainting(true);
    //setAcceptedMouseButtons(Qt::LeftButton);

    DBThread *dbThread=DBThread::GetInstance();
    //setFocusPolicy(Qt::StrongFocus);

    connect(dbThread,SIGNAL(InitialisationFinished(DatabaseLoadedResponse)),
            this,SLOT(initialisationFinished(DatabaseLoadedResponse)));

    connect(this,SIGNAL(TriggerMapRenderingSignal()),
            dbThread,SLOT(TriggerMapRendering()));

    connect(dbThread,SIGNAL(HandleMapRenderingResult()),
            this,SLOT(redraw()));

    connect(dbThread,SIGNAL(Redraw()),
            this,SLOT(redraw()));
    quickZooming = false;
    quickMoveX = 0;
    quickMoveY = 0;
    quickZoomFactor = 1;
}

MapWidget::~MapWidget()
{
    // no code
}

void MapWidget::redraw()
{
    update();
}

void MapWidget::initialisationFinished(const DatabaseLoadedResponse& response)
{
    size_t zoom=1;
    double dlat=360;
    double dlon=180;

    center=response.boundingBox.GetCenter();

    while (dlat>response.boundingBox.GetHeight() &&
           dlon>response.boundingBox.GetWidth()) {
        zoom=zoom*2;
        dlat=dlat/2;
        dlon=dlon/2;
    }

    magnification=zoom;

    TriggerMapRendering();
}

void MapWidget::TriggerMapRendering()
{
    DBThread         *dbThread=DBThread::GetInstance();
    if(!dbThread->IsOpened())return;
    RenderMapRequest request;

    request.lat=center.GetLat();
    request.lon=center.GetLon();
    request.angle=angle;
    request.magnification=magnification;
    request.width=width();
    request.height=height();

    dbThread->UpdateRenderRequest(request);

    emit TriggerMapRenderingSignal();
}


void MapWidget::paint(QPainter *painter)
{
    if(quickZooming) //just zoom the already rendered Qimage and repaint
    {
        DBThread         *dbThread=DBThread::GetInstance();
        dbThread->RenderMapQuick(*painter, quickMoveX, quickMoveY, quickZoomFactor);
    }
    else
    {
        RenderMapRequest request;
        DBThread         *dbThread=DBThread::GetInstance();
        QRectF           boundingBox=contentsBoundingRect();

        request.lat=center.GetLat();
        request.lon=center.GetLon();
        request.angle=angle;
        request.magnification=magnification;
        request.width=boundingBox.width();
        request.height=boundingBox.height();

        if (!dbThread->RenderMap(*painter,request) &&
                requestNewMap) {
            TriggerMapRendering();
        }

        requestNewMap=true;
    }
}

void MapWidget::zoomQuick(double zoomFactor)
{
    quickZooming = true;
    quickZoomFactor = zoomFactor;
    redraw();
}

void MapWidget::zoomIn(double zoomFactor)
{
    osmscout::Magnification maxMag;
    maxMag.SetLevel(20);

    if (magnification.GetMagnification()*zoomFactor>maxMag.GetMagnification()) {
        magnification.SetMagnification(maxMag.GetMagnification());
    }
    else {
        magnification.SetMagnification(magnification.GetMagnification()*zoomFactor);
    }
    //TriggerMapRendering();
    quickZooming = false;
    quickMoveX = 0;
    quickMoveY = 0;
    quickZoomFactor = 1;


}

void MapWidget::zoomOut(double zoomFactor)
{
    if (magnification.GetMagnification()/zoomFactor<1) {
        magnification.SetMagnification(1);
    }
    else {
        magnification.SetMagnification(magnification.GetMagnification()/zoomFactor);
    }

    //TriggerMapRendering();
    quickZooming = false;
    quickMoveX = 0;
    quickMoveY = 0;
    quickZoomFactor = 1;

}

void MapWidget::move(int x, int y)
{
    quickMoveX = 0;
    quickMoveY = 0;
    quickZoomFactor = 1;
    quickZooming = false;
    DBThread                     *dbThread=DBThread::GetInstance();
    if(!dbThread->IsOpened()) return;
    osmscout::MercatorProjection projection;

    dbThread->GetProjection(projection);

    if(x<0) projection.MoveLeft(-1*x);
    else    projection.MoveRight(x);

    if(y<0) projection.MoveUp(-1*y);
    else    projection.MoveDown(y);

    center=projection.GetCenter();

    TriggerMapRendering();
}

void MapWidget::moveQuick(int x, int y)
{
    quickZooming = true;

    quickMoveX = x;
    quickMoveY = y;
    redraw();
}

void MapWidget::left()
{
    quickMoveX = 0;
    quickMoveY = 0;
    quickZooming = false;
    quickZoomFactor = 1;
    DBThread                     *dbThread=DBThread::GetInstance();
    osmscout::MercatorProjection projection;

    dbThread->GetProjection(projection);

    projection.MoveLeft(width()/3);

    center=projection.GetCenter();

    TriggerMapRendering();
}

void MapWidget::right()
{
    quickMoveX = 0;
    quickMoveY = 0;
    quickZooming = false;
    quickZoomFactor = 1;
    DBThread                     *dbThread=DBThread::GetInstance();
    osmscout::MercatorProjection projection;

    dbThread->GetProjection(projection);

    projection.MoveRight(width()/3);

    center=projection.GetCenter();

    TriggerMapRendering();
}

void MapWidget::up()
{
    quickMoveX = 0;
    quickMoveY = 0;
    quickZooming = false;
    quickZoomFactor = 1;

    DBThread                     *dbThread=DBThread::GetInstance();
    osmscout::MercatorProjection projection;

    dbThread->GetProjection(projection);

    projection.MoveUp(height()/3);

    center=projection.GetCenter();

    TriggerMapRendering();
}

void MapWidget::down()
{
    quickMoveX = 0;
    quickMoveY = 0;
    quickZooming = false;
    quickZoomFactor = 1;

    DBThread                     *dbThread=DBThread::GetInstance();
    osmscout::MercatorProjection projection;

    dbThread->GetProjection(projection);

    projection.MoveDown(height()/3);

    center=projection.GetCenter();

    TriggerMapRendering();
}

void MapWidget::rotateLeft()
{
    angle=round(angle/DELTA_ANGLE)*DELTA_ANGLE-DELTA_ANGLE;

    if (angle<0) {
        angle+=2*M_PI;
    }

    TriggerMapRendering();
}

void MapWidget::rotateRight()
{
    angle=round(angle/DELTA_ANGLE)*DELTA_ANGLE+DELTA_ANGLE;

    if (angle>=2*M_PI) {
        angle-=2*M_PI;
    }

    TriggerMapRendering();
}




void MapWidget::showCoordinates(double lat, double lon)
{
    center=osmscout::GeoCoord(lat,lon);
    this->magnification=osmscout::Magnification::magVeryClose;

    TriggerMapRendering();
}

void MapWidget::showLocation(Location* location)
{
    if (location==NULL) {
        std::cout << "MapWidget::showLocation(): no location passed!" << std::endl;

        return;
    }

    std::cout << "MapWidget::showLocation(\"" << location->getName().toLocal8Bit().constData() << "\")" << std::endl;

    if (location->getType()==Location::typeObject) {
        osmscout::ObjectFileRef reference=location->getReferences().front();

        DBThread* dbThread=DBThread::GetInstance();

        if (reference.GetType()==osmscout::refNode) {
            osmscout::NodeRef node;

            if (dbThread->GetNodeByOffset(reference.GetFileOffset(),node)) {
                center=node->GetCoords();
                this->magnification=osmscout::Magnification::magVeryClose;

                TriggerMapRendering();
            }
        }
        else if (reference.GetType()==osmscout::refArea) {
            osmscout::AreaRef area;

            if (dbThread->GetAreaByOffset(reference.GetFileOffset(),area)) {
                if (area->GetCenter(center)) {
                    this->magnification=osmscout::Magnification::magVeryClose;

                    TriggerMapRendering();
                }
            }
        }
        else if (reference.GetType()==osmscout::refWay) {
            osmscout::WayRef way;

            if (dbThread->GetWayByOffset(reference.GetFileOffset(),way)) {
                if (way->GetCenter(center)) {
                    this->magnification=osmscout::Magnification::magVeryClose;

                    TriggerMapRendering();
                }
            }
        }
        else {
            assert(false);
        }
    }
    else if (location->getType()==Location::typeCoordinate) {
        osmscout::GeoCoord coord=location->getCoord();

        std::cout << "MapWidget: " << coord.GetDisplayText() << std::endl;

        center=coord;
        this->magnification=osmscout::Magnification::magVeryClose;

        TriggerMapRendering();
    }
}

double MapWidget::geoToPixelX(double lon, double lat)
{
    DBThread                     *dbThread=DBThread::GetInstance();
    if(!dbThread->IsOpened()) return 0;
    double X,Y;
    osmscout::MercatorProjection projection;

    dbThread->GetProjection(projection);
    projection.GeoToPixel(lon, lat, X, Y);
    return X;
}

double MapWidget::geoToPixelY(double lon, double lat)
{
    DBThread                     *dbThread=DBThread::GetInstance();
    if(!dbThread->IsOpened()) return 0;
    double X,Y;
    osmscout::MercatorProjection projection;
    dbThread->GetProjection(projection);
    projection.GeoToPixel(lon, lat, X, Y);
    return Y;
}

bool MapWidget::isValid()
{
    DBThread                     *dbThread=DBThread::GetInstance();
    return dbThread->IsOpened();

}

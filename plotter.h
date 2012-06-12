#ifndef PLOTTER_H
#define PLOTTER_H

#include <QMap>
#include <QPixmap>
#include <QVector>
#include <QWidget>
#include "scale.h"
#include <QLabel>

class EcgSignal;
class Annotation;
class Filter;

class QToolButton;
class PlotSettings;
class Plotter : public QWidget
{
    Q_OBJECT

public:
    Plotter(QWidget *parent = 0);

	QSize minimumSizeHint() const;
	QSize sizeHint() const;

	void setSignalData();
	void setAnnotationData();
    void setFilterData();
	void setMinX(const double &minX);
	void setMaxX(const double &maxX);
	void setMinY(const double &minY);
	void setMaxY(const double &maxY);
	void setIncrementX(const double &increment);
	void setIncrementY(const double &increment);
	void setScaleX(const Scale &scaleX);
	void setScaleY(const Scale &scaleY);
	void setCurrentPointLabel(QLabel *label);

	QPixmap render(const QSize &s);

	Scale scaleX() const;
	Scale scaleY() const;
	EcgSignal signalData() const;

    bool readFile(const QString &fileName);
    int getSignalSampleRate() const;
    int getSignalSize() const;
	void setSignalSampleRate();


signals:
	void zoomChanged(int zoom, int maxZoom);

public slots:
    void zoomIn();
    void zoomOut();

	void updatePlot();

protected:
    void paintEvent(QPaintEvent *event);
    void resizeEvent(QResizeEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void keyPressEvent(QKeyEvent *event);
	void wheelEvent(QWheelEvent *event);

private:
	enum
	{
		LeftMargin = 10,
		TopMargin = 10,
		BottomMargin = 10,
		RightMargin = 20
	};

	QPixmap pixmap;
    EcgSignal *signal; //Заменить на композицию
	Annotation *annotation;
    Filter *filter;
	QVector<QPointF> plotData;
	QVector<Scale> zoomXAxis;
	QVector<Scale> zoomYAxis;

	QToolButton *zoomInButton;
    QToolButton *zoomOutButton;
	QLabel *_currentPointLabel;

    int curZoom;
	Scale _curScaleX;
	Scale _curScaleY;
    bool rubberBandIsShown;
    QRect rubberBandRect;
	QRect _plotRect;

	void updateRubberBandRegion();
	void refreshPixmap();
	void drawPlot(QPainter *painter);
	void drawEcgGrid(QPainter *painter);
	void drawAnnotation(QPainter *painter);
	QRect plotRect(const QSize &viewPortSize);

};

#endif // PLOTTER_H

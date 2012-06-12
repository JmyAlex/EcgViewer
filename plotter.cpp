#include <QtGui>   //////////!!!!!!!!!!!!!!!!111111111111111

#include "plotter.h"
#include "ecgsignal.h"
#include "annotation.h"
#include "filter.h"

#include <iostream>
#include <algorithm>

Plotter::Plotter(QWidget *parent) : QWidget(parent)
{
    setBackgroundRole(QPalette::Light);
    setAutoFillBackground(true);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setFocusPolicy(Qt::StrongFocus);
    rubberBandIsShown = false;

	zoomXAxis.append(Scale());
	zoomYAxis.append(Scale());
	curZoom = 0;

    zoomInButton = new QToolButton(this);  //Перенести на тулбар, добавить контекстное меню
    zoomInButton->setIcon(QIcon(":/images/zoomin.png"));
    zoomInButton->adjustSize();
    connect(zoomInButton, SIGNAL(clicked()), this, SLOT(zoomIn()));

    zoomOutButton = new QToolButton(this);
    zoomOutButton->setIcon(QIcon(":/images/zoomout.png"));
    zoomOutButton->adjustSize();
    connect(zoomOutButton, SIGNAL(clicked()), this, SLOT(zoomOut()));

	zoomInButton->hide();
	zoomOutButton->hide();

    signal = new EcgSignal;
	annotation = new Annotation;
    filter = new Filter;

}

void Plotter::zoomOut()
{
    if(curZoom > 0)
    {
        --curZoom;
        zoomOutButton->setEnabled(curZoom > 0);
        zoomInButton->setEnabled(true);
        zoomInButton->show();
		emit zoomChanged(curZoom, zoomXAxis.count());
        refreshPixmap();
    }
}

void Plotter::zoomIn()
{
	if(curZoom < zoomXAxis.count() - 1)
    {
        ++curZoom;
		zoomInButton->setEnabled(curZoom < zoomXAxis.count() - 1);
        zoomOutButton->setEnabled(true);
        zoomOutButton->show();
		emit zoomChanged(curZoom, zoomXAxis.count());
        refreshPixmap();
    }
}

//Set curve data
void Plotter::setSignalData()
{
	int size = signal->size();
	QVector<double> data = signal->data();

	QPointF point [size];
	for (int i = 0; i < size; i++)
	{
		point[i] = QPointF(i, data[i]);
		plotData.push_back(point[i]);
	}
	QVector<double>::iterator min_result = std::min_element(data.begin(), data.end());
	int minPos = std::distance(data.begin(), min_result);
	int miny = data[minPos];
	QVector<double>::iterator max_result = std::max_element(data.begin(), data.end());
	int maxPos = std::distance(data.begin(), max_result);
	int maxy = data[maxPos];

    setMinX(0.0);                              //Выбор развертки в зависимости от длины сигнала
    if (size <= 4000)
        setMaxX(size / 4);   //size/4
    else if (size <= 1000)
        setMaxX(size);
    else if (size >= 10000)
        setMaxX(size / 10);
    setMinY(miny - 200);
    setMaxY(maxy + 200);

}

void Plotter::setAnnotationData()
{
	annotation->setSignal(*signal);
	if (annotation->getAnnotation())
	{
        //QMessageBox::information(this, tr("Annotation"), tr("Get Annotation - done -_-"));
		refreshPixmap();
	}
	else
	{
		QMessageBox::warning(this, tr("Annotation"),
                            tr("Error: cant find waves"));
	}
}

void Plotter::setFilterData()
{
    if (signal->size() > 1)
    {
        filter->setSignal(*signal);
        if (filter->filterSignal())
        {
            QMessageBox::information(this, tr("Filter"), tr("Filter is filling good ^_^"));
            plotData.clear();
            QVector<double> data = filter->data();
            signal->setData(data);
            int size = signal->size();
            for (int i = 0; i < data.count(); i++)
            {
                plotData.append(QPointF(i, data[i]));
            }
            QVector<double>::iterator min_result = std::min_element(data.begin(), data.end());
            int minPos = std::distance(data.begin(), min_result);
            int miny = data[minPos];
            QVector<double>::iterator max_result = std::max_element(data.begin(), data.end());
            int maxPos = std::distance(data.begin(), max_result);
            int maxy = data[maxPos];

            setMinX(0.0);
            setMaxX(size/4);
            setMinY(miny - 200);
            setMaxY(maxy + 200);
//            for (int i = 0; i < plotData.count(); i++)
//                qDebug() << plotData[i].y();

            refreshPixmap();
        }
        else
        {
            QMessageBox::warning(this, tr("Filter"),
                            tr("Error"));
        }
    }
    else
        QMessageBox::warning(this, tr("Filter"),
                        tr("Error: no signal"));
}

void Plotter::setMinX(const double &minX)
{
	if (minX != _curScaleX.min())
	{
		_curScaleX.setMin(minX);
		zoomXAxis[curZoom] = _curScaleX;
		refreshPixmap();
	}
}

void Plotter::setMaxX(const double &maxX)
{
	if (maxX != _curScaleX.max())
	{
		_curScaleX.setMax(maxX);
		zoomXAxis[curZoom] = _curScaleX;
		refreshPixmap();
	}
}

void Plotter::setMinY(const double &minY)
{
	if (minY != _curScaleY.min())
	{
		_curScaleY.setMin(minY);
		zoomYAxis[curZoom] = _curScaleY;
		refreshPixmap();
	}
}

void Plotter::setMaxY(const double &maxY)
{
	if (maxY != _curScaleY.max())
	{
		_curScaleY.setMax(maxY);
		zoomYAxis[curZoom] = _curScaleY;
		refreshPixmap();
	}
}

void Plotter::setIncrementX(const double &increment)
{
	if (increment != _curScaleX.increment())
	{
		_curScaleX.setIncrement(increment);
		zoomXAxis[curZoom] = _curScaleX;
		refreshPixmap();
	}
}

void Plotter::setIncrementY(const double &increment)
{
	if (increment != _curScaleY.increment())
	{
		_curScaleY.setIncrement(increment);
		zoomYAxis[curZoom] = _curScaleY;
		refreshPixmap();
	}
}

void Plotter::setScaleX(const Scale &scaleX)
{
	if (_curScaleX != scaleX)
	{
		_curScaleX = scaleX;
		zoomXAxis[curZoom] = scaleX;
		refreshPixmap();
	}
}

void Plotter::setScaleY(const Scale &scaleY)
{
	if (scaleY != _curScaleY)
	{
		_curScaleY = scaleY;
		zoomYAxis[curZoom] = scaleY;
		refreshPixmap();
	}
}

Scale Plotter::scaleX() const
{
	return zoomXAxis[curZoom];
}

Scale Plotter::scaleY() const
{
	return zoomYAxis[curZoom];
}

EcgSignal Plotter::signalData() const
{
	return *signal;
}

void Plotter::updatePlot()
{
	refreshPixmap();
}

//void Plotter::removeCurve(int index)
//{
//    if (index >= 0 && index < _curvesList.size())
//	{
//        _curvesList.removeAt(index);
//        _showCurvesList.removeAt(index);
//        refreshPixmap();
//        emit removedCurve(index);
//    }
//}

//Size of widget
QSize Plotter::minimumSizeHint() const
{
	return QSize(300, 200);
}

QSize Plotter::sizeHint() const
{
    return QSize(750, 600);
}

void Plotter::paintEvent(QPaintEvent *event)
{
    QStylePainter painter(this);
    painter.drawPixmap(0, 0, pixmap);

    if(rubberBandIsShown)
    {
		painter.setPen(palette().dark().color().blue());
        painter.drawRect(rubberBandRect.normalized().adjusted(0, 0, -1, -1));
    }

	if(hasFocus())
	{
		QStyleOptionFocusRect option;
		option.initFrom(this);
		option.backgroundColor = palette().light().color();
		painter.drawPrimitive(QStyle::PE_FrameFocusRect, option);
	}
	QWidget::paintEvent(event);
}

//ZoomOut, ZoomIn buttons coordinats
void Plotter::resizeEvent(QResizeEvent * /* event */)
{
    int x = width() - (zoomInButton->width() + zoomOutButton->width() + 10);
    zoomInButton->move(x, 5);
    zoomOutButton->move(x + zoomInButton->width() + 5, 5);
    refreshPixmap();
}

//Mouse events, RubberBand
void Plotter::mousePressEvent(QMouseEvent *event)
{
	if(event->button() == Qt::LeftButton)
    {
		if(_plotRect.contains(event->pos()))
        {
            rubberBandIsShown = true;
            rubberBandRect.setTopLeft(event->pos());
            rubberBandRect.setBottomRight(event->pos());
            updateRubberBandRegion();
            setCursor(Qt::CrossCursor);
        }
    }
}

void Plotter::mouseMoveEvent(QMouseEvent *event)
{
    if(rubberBandIsShown)
    {
        updateRubberBandRegion();
        rubberBandRect.setBottomRight(event->pos());
        updateRubberBandRegion();
    }

//	if (_plotRect.contains(event->pos()))
//	{
//		setCursor(Qt::CrossCursor);

//		if (_currentPointLabel)
//		{
//			double a11 = _curScaleX.width()/_plotRect.width();
//			double a12 = _curScaleX.min() - a11 * _plotRect.x();
//			double a21 = -_curScaleY.width()/_plotRect.height();
//			double a22 = _curScaleY.max() - a21 * _plotRect.y();

//			double x = a11 * event->x() + a12;
//			double y = a21 * event->y() + a22;

//			_currentPointLabel->setText(QString("x = %1,  y = %2").arg(x).arg(y));
//		}
//	}
//	else
//	{
//		if (_currentPointLabel)
//			_currentPointLabel->setText("");
//		else
//			unsetCursor();
//	}
}

void Plotter::mouseReleaseEvent(QMouseEvent *event)
{
    if((event->button() == Qt::LeftButton) && rubberBandIsShown)
    {
        rubberBandIsShown = false;
        updateRubberBandRegion();

		unsetCursor();
        QRect rect = rubberBandRect.normalized();
        if(rect.width() < 4 || rect.height() < 4)
            return;

		rect.translate(-_plotRect.x(), -_plotRect.y());

		double dx = _curScaleX.width() / _plotRect.width();
		double dy = _curScaleY.width() / _plotRect.height();

		double minX = _curScaleX.min() + dx * rect.left();
		double maxX = _curScaleX.min() + dx * rect.right();
		double minY = _curScaleY.max() - dy * rect.bottom();
		double maxY = _curScaleY.max() - dy * rect.top();

		_curScaleX.setMin(minX);
		_curScaleX.setMax(maxX);
		_curScaleY.setMin(minY);
		_curScaleY.setMax(maxY);

		_curScaleX.adjust();
		_curScaleY.adjust();

		zoomXAxis.resize(curZoom + 1);
		zoomXAxis.append(_curScaleX);
		zoomYAxis.resize(curZoom + 1);
		zoomYAxis.append(_curScaleY);
        zoomIn();
    }
}

void Plotter::setCurrentPointLabel(QLabel *label)
{
	_currentPointLabel = label;
}

//Keyboard events
void Plotter::keyPressEvent(QKeyEvent *event)
{
    switch(event->key())
    {
    case Qt::Key_Plus:
        zoomIn();
        break;
    case Qt::Key_Minus:
        zoomOut();
        break;
    case Qt::Key_Left:
		scrollScale(zoomXAxis[curZoom], -1);
        refreshPixmap();
        break;
    case Qt::Key_Right:
		scrollScale(zoomXAxis[curZoom], 1);
        refreshPixmap();
        break;
    case Qt::Key_Down:
		scrollScale(zoomYAxis[curZoom], -1);
        refreshPixmap();
        break;
    case Qt::Key_Up:
		scrollScale(zoomYAxis[curZoom], 1);
        refreshPixmap();
        break;
    default:
        QWidget::keyPressEvent(event);
    }
}

void Plotter::wheelEvent(QWheelEvent *event)
{
	int numDegrees = event->delta() / 8;
	int numTicks = numDegrees / 15;

	if(event->orientation() == Qt::Horizontal)
	{
		scrollScale(zoomXAxis[curZoom], numTicks);
	}
	else
	{
		scrollScale(zoomYAxis[curZoom], numTicks);
	}
	refreshPixmap();
}

void Plotter::updateRubberBandRegion()
{
    QRect rect = rubberBandRect.normalized();
    update(rect.left(), rect.top(), rect.width(), 1);
    update(rect.left(), rect.top(), 1, rect.height());
    update(rect.left(), rect.bottom(), rect.width(), 1);
    update(rect.right(), rect.top(), 1, rect.height());
}

QRect Plotter::plotRect(const QSize &viewPortSize)
{
	return QRect(0, 0, viewPortSize.width(), viewPortSize.height());
}

void Plotter::drawPlot(QPainter *painter)
{
	painter->save();
	const double a11 = double(_plotRect.width())/_curScaleX.width();
	const double a12 = _plotRect.x() - _curScaleX.min() * a11;

	const double a21 = double(-_plotRect.height())/_curScaleY.width();
	const double a22 = _plotRect.y() - _curScaleY.max() * a21;

	//painter->setPen(tickPen);
	//painter->drawRect(_plotRect);

	painter->setClipRect(_plotRect);
	const QVector<QPointF> &data = plotData;
	QPolygonF polyline;
	foreach(QPointF p, data)
	{
		double x = a11 * p.x() + a12;
		double y = a21 * p.y() + a22;
        if ((p.x() >= _curScaleX.min() - 0.25*_curScaleX.width()) &&
            (p.x() <= _curScaleX.max() + 0.25*_curScaleX.width()))
			polyline << QPointF(x, y);
	}

	QPen curvePen;
	curvePen.setColor(Qt::black);	       //curvePen.setColor(cd.color());
    curvePen.setWidthF(1);					//curvePen.setWidthF(cd.penWidth());
	//curvePen.setStyle(cd.penStyle());      //curvePen.setStyle(cd.penStyle());

	painter->setPen(curvePen);
    painter->setRenderHint(QPainter::Antialiasing, false);
	painter->drawPolyline(polyline);
	painter->setRenderHint(QPainter::Antialiasing, false);

	painter->restore();
}

void Plotter::refreshPixmap()
{
    pixmap = QPixmap(size());
    pixmap.fill(this, 0, 0);

    QPainter painter(&pixmap);
	painter.fillRect(rect(), Qt::white);

	_curScaleX = zoomXAxis[curZoom];
	_curScaleY = zoomYAxis[curZoom];
	_plotRect = plotRect(size());
	drawEcgGrid(&painter);
	drawPlot(&painter);
	drawAnnotation(&painter);

    update();
}

QPixmap Plotter::render(const QSize &s)
{
	QPixmap pix(s);
	QPainter painter(&pix);
	painter.fillRect(0, 0, s.width(), s.height(), Qt::white);
	_curScaleX = zoomXAxis[curZoom];
	_curScaleY = zoomYAxis[curZoom];
	_plotRect = plotRect(s);
	drawPlot(&painter);

	return pix;
}

void Plotter::drawEcgGrid(QPainter *painter)       //Доработать !!!!!111
{
	painter->save();

	QPen quiteDark = palette().dark().color().light();
	for (int i = 0; i < width(); i += 4)
	{
		painter->setPen(quiteDark);
		painter->drawLine(i, _plotRect.top(), i, _plotRect.bottom());
		painter->drawLine(_plotRect.left(), i, _plotRect.right(), i);
	}
	//QPen quiteDark = palette().dark().color().light();
	for (int i = 0; i < width(); i += 20)
	{
		painter->setPen(Qt::lightGray);
		painter->drawLine(i, _plotRect.top(), i, _plotRect.bottom());
		painter->drawLine(_plotRect.left(), i, _plotRect.right(), i);
	}
	//QPen quiteDark = palette().dark().color().light();
	for (int i = 0; i < width(); i += 40)
	{
		painter->setPen(Qt::darkGray);
		painter->drawLine(i, _plotRect.top(), i, _plotRect.bottom());
		painter->drawLine(_plotRect.left(), i, _plotRect.right(), i);
	}

	painter->restore();
}

void Plotter::drawAnnotation(QPainter *painter)
{
	painter->save();

	const double a11 = double(_plotRect.width())/_curScaleX.width();
	const double a12 = _plotRect.x() - _curScaleX.min() * a11;

	const double a21 = double(-_plotRect.height())/_curScaleY.width();
	const double a22 = _plotRect.y() - _curScaleY.max() * a21;

	const QVector<Point> _points = annotation->points();
	const QVector<QString> _annotation = annotation->annotation();
	QVector<QRectF> rects;
	QVector<QPointF> pos(_points.count());
	for (int i = 0; i < _points.size(); i++)
	{
		double x = a11 * _points[i].x() + a12;
		double y = a21 * _points[i].y() + a22;
		if ((_points[i].x() >= _curScaleX.min()) &&
			(_points[i].x() <= _curScaleX.max()))
		{
			rects.append(QRectF(x - 3, y - 3, 6, 6));
			pos[i] = QPointF(x - 12, y - 6);
		}

	}
//	QPen annotationPen;
//	annotationPen.setBrush(Qt::red);
//	annotationPen.setColor(Qt::darkRed);
	painter->setPen(Qt::darkRed);
	painter->setBrush(Qt::red);
	painter->drawRects(rects);
	painter->setPen(QPen(Qt::darkBlue));
	painter->setFont(QFont("Courier", 12, QFont::Bold));
	for (int i = 0; i < _annotation.size(); i++)
	{
		painter->drawText(pos[i], _annotation[i]);
	}

	painter->restore();
}

bool Plotter::readFile(const QString &fileName)			//AAAAAAAAAAAAAA wtf переделать нафиг !!!
{
	plotData.clear();
	delete annotation;									//!!!!!!!1111
	annotation = new Annotation;
	signal->setSampleRate(1000);

	QFile InFile(fileName);
    if (!InFile.open(QIODevice::ReadOnly))              //Добавить проверку на расширение, magick number?
	{
		QMessageBox::warning(this, tr("Plotter"),
							 tr("Cannot read file "));
		std::cerr << "Cannot open file for reading: " << qPrintable(InFile.errorString()) << std::endl;
		return false;
	}
	int size = InFile.size() / sizeof(double);
	signal->setSize(size);

    QVector<double> data;
    data.resize(size);
	for (int i = 0; i < size; i++)
        InFile.read(reinterpret_cast<char*>(&data[i]), sizeof(double));
    InFile.close();

    signal->setData(data);
	setSignalData();

    return true;
}

int Plotter::getSignalSampleRate()const
{
	return signal->sampleRate();
}

int Plotter::getSignalSize()const
{
	return signal->size();
}

void Plotter::setSignalSampleRate()
{
	signal->setSampleRate(100);
}

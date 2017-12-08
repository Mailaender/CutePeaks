#include "traceview.h"

TraceView::TraceView(QWidget *parent)
    :QAbstractScrollArea(parent)
{
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    // enable touch screen
    viewport()->setAttribute(Qt::WA_AcceptTouchEvents, true);
    QScroller::grabGesture(viewport(), QScroller::LeftMouseButtonGesture);

    setDisabled(true);

    setCursor(Qt::CrossCursor);
}
//-------------------------------------------------------------------------------
void TraceView::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter painter(viewport());
    // Draw empty background
    if (!trace()->isValid()){
        painter.setPen(Qt::NoPen);
        painter.setBrush(QBrush(Qt::white));
        painter.drawRect(viewport()->rect());
        return;
    }

    // otherwise draw trace
    drawAll(painter);
}
//-------------------------------------------------------------------------------
void TraceView::scrollContentsBy(int dx, int dy)
{
    Q_UNUSED(dy)
    mXStart -= dx ;
    viewport()->update();
}
//-------------------------------------------------------------------------------
void TraceView::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event)
    updateScrollbar();
}
//-------------------------------------------------------------------------------
void TraceView::mouseMoveEvent(QMouseEvent *event)
{
    QAbstractScrollArea::mouseMoveEvent(event);
}

void TraceView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        qDebug()<<"click "<<mCurrentSelection.pos<<" "<<mCurrentSelection.length;
        mTrace->cut(mCurrentSelection.pos, mCurrentSelection.length);
        viewport()->update();
        updateScrollbar();

    }


    QAbstractScrollArea::mousePressEvent(event);
}
//-------------------------------------------------------------------------------
bool TraceView::viewportEvent(QEvent *event)
{
    return QAbstractScrollArea::viewportEvent(event);
}
//-------------------------------------------------------------------------------
void TraceView::setupViewport()
{
    qDebug()<<"after";
}
//-------------------------------------------------------------------------------
bool TraceView::inView(int pos, int margin) const
{
    // if base pos is in the viewport
    return (pos >= mXStart - margin && pos <= viewport()->rect().width()/mXFactor + mXStart + margin);
}
//-------------------------------------------------------------------------------
void TraceView::updateScrollbar()
{
    if (!trace()->isValid())
        return;

    int maxXSize = trace()->length();
    horizontalScrollBar()->setRange(0, maxXSize - viewport()->width()/mXFactor);
    horizontalScrollBar()->setPageStep(viewport()->width()/ mXFactor);
}
//-------------------------------------------------------------------------------
void TraceView::drawAll(QPainter &painter)
{
    // draw background
    painter.setPen(Qt::NoPen);
    painter.setBrush(QBrush(Qt::white));
    painter.drawRect(viewport()->rect());

    // set antialiasing
    painter.setRenderHint(QPainter::Antialiasing, true);

    // draw elements
    drawBases(painter);
    drawAminoAcid(painter);
    drawTraces(painter);
    drawConfident(painter);
    drawSelection(painter);
}
//-------------------------------------------------------------------------------
void TraceView::drawConfident(QPainter& painter)
{
    QPainterPath stepCurve;
    QColor color("#ced9eb");
    QPen pen;
    pen.setColor(color);
    pen.setWidthF(2);
    painter.setPen(pen);
    //    painter.setBrush(Qt::transparent);
    color.setAlphaF(0.4);
    painter.setBrush(color);

    QPainterPath path;

    QPointF oldPoint;

    path.moveTo(0,-100);

    for (int i = 0 ; i < trace()->baseLocations().length(); ++i)
    {
        int pos = trace()->baseLocations().at(i);
        int score  = trace()->baseScores().at(i);

        if (inView(pos, 10))
        {

            QPointF p ((pos - mXStart) * mXFactor, score * 5 );

            QPointF delta = QPointF(oldPoint.x() +(p.x() - oldPoint.x())/2, 100);

            // draw step

            //            path.moveTo(oldPoint);
            path.lineTo(QPoint(delta.x(), oldPoint.y()));
            path.lineTo(delta.x(), p.y());
            path.lineTo(p);
            oldPoint = p;
        }
    }

    path.lineTo(oldPoint.x(), -100);
    path.setFillRule(Qt::OddEvenFill);
    painter.drawPath(path);

}
//-------------------------------------------------------------------------------

void TraceView::drawBases(QPainter& painter)
{

    int yMargin = 27;
    QPen pen;
    pen.setColor(Qt::gray);
    painter.setPen(pen);
    painter.setBrush(QBrush("#F5F5F5"));
    //    painter.drawRect(0, 0, viewport()->width(), yMargin);
    int codonCounter = 0;
    QPointF oldPos;
    bool swapBgColor = false;

    QFont font;
    font.setPixelSize(15);
    font.setBold(true);
    QFontMetrics metrics(font);
    painter.setFont(font);

    int previousPos = 0;

    for (int i = 0 ; i < trace()->baseLocations().length(); ++i, ++codonCounter)
    {
        int pos = trace()->baseLocations().at(i);

        if (inView(pos))
        {
            QPointF p ((pos - mXStart) * mXFactor, 15);
            QChar base = trace()->sequence().at(i);

            // IKIT
            // draw point



            QPoint ptest ( p.x(), yMargin + 20);
            QPen penTest;
            penTest.setWidth(4);
            penTest.setColor(Qt::red);

            painter.setPen(penTest);
            painter.drawPoint(ptest);

            QFontMetrics metrics(font);
            QPointF textPos (p.x() - metrics.width(base)/2, p.y());
            textPos.setY(yMargin - 8);
            painter.setPen(QPen(TraceColor::color(base)));
            painter.setBrush(QBrush(TraceColor::color(base)));

            painter.drawText(textPos, QString(base));

            if ( !(i % 10)){
                painter.setPen(QPen(Qt::lightGray));
                painter.drawText(textPos + QPoint(0,50), QString::number(i));
            }

        }


    }



}
//-------------------------------------------------------------------------------

void TraceView::drawAminoAcid(QPainter &painter)
{
    QPen pen;
    pen.setColor(Qt::gray);
    painter.setPen(pen);

    for (int i = 0 ; i < trace()->baseLocations().length()-3; i+=3)
    {
        int pos = trace()->baseLocations().at(i);

        if (inView(pos))
        {
            QPointF p ((pos - mXStart) * mXFactor, 15);

            // Draw Base
            QByteArray codon = trace()->sequence().byteArray().mid(i,3);
            Sequence seq(codon);

            QString aa = seq.translate().toString();

            QFont font;
            font.setPixelSize(15);
            font.setBold(true);
            painter.setFont(font);

            QFontMetrics metrics(font);
            QPointF textPos (p.x() - metrics.width(aa)/2, p.y());
            textPos.setY(50);
            painter.drawText(textPos, QString(aa));


        }
    }
}
//-------------------------------------------------------------------------------
void TraceView::drawTraces(QPainter& painter)
{
    // inverse y axis
    painter.translate(viewport()->rect().bottomLeft());
    painter.scale(1.0, -1.0);

    for (QChar base : trace()->basesAvaible())
    {
        // load paths to draw
        QPainterPath path;
        path.moveTo(0,0);
        QVector<int> data = trace()->datas()[base];

        for ( int x = mXStart ; x < viewport()->rect().width()/mXFactor + mXStart; ++x)
        {
            if (x >= data.size())
                break;

            QPointF p ( x - mXStart, data[x]);
            path.lineTo((p.x()) * mXFactor , p.y() * mYFactor);

        }
        // draw path
        QPen pen;
        pen.setColor(TraceColor::color(base));
        pen.setWidthF(1);
        pen.setJoinStyle(Qt::RoundJoin);
        painter.setPen(pen);
        painter.setBrush(Qt::transparent);
        painter.drawPath(path);
    }
}
//-------------------------------------------------------------------------------
void TraceView::drawSelection(QPainter &painter)
{
    QPainterPath stepCurve;

    QPen pen;
    QColor highlight = palette().brush(QPalette::Highlight).color();
    pen.setColor(highlight);
    pen.setWidthF(2);
    painter.setPen(pen);
    highlight.setAlphaF(0.1);
    QBrush bgBrush = QBrush(highlight);
    painter.setBrush(bgBrush);

    if (mCurrentSelection.length >= trace()->baseLocations().length())
        return;


    int start = trace()->baseLocations().at(mCurrentSelection.pos);
    int end   = trace()->baseLocations().at(mCurrentSelection.pos + mCurrentSelection.length);

    QPointF up   ((start - mXStart) * mXFactor, 0);
    QPointF down ((end - mXStart) * mXFactor, viewport()->height());

    QRectF area;
    area.setTopLeft(up);
    area.setBottomRight(down);

    painter.drawRect(area);


}
//-------------------------------------------------------------------------------

void TraceView::setFilename(const QString &filename)
{
    mFilename = filename;
    setTrace(TraceFactory::createTrace(filename));


}
//-------------------------------------------------------------------------------
void TraceView::setTrace(Trace *trace)
{
    mTrace = trace;
    if (!mTrace->isValid()){
        qCritical()<<Q_FUNC_INFO<<tr("Cannot read the file");
        setDisabled(true);
        return ;
    }

    setDisabled(false);
    viewport()->update();
    updateScrollbar();
}
//-------------------------------------------------------------------------------

const Trace *TraceView::trace() const
{
    return mTrace;
}
//-------------------------------------------------------------------------------
bool TraceView::isValid() const
{
    return trace()->isValid();
}
//-------------------------------------------------------------------------------
void TraceView::setAmplitudeFactor(float factor)
{
    mYFactor = factor;
    viewport()->update();
    updateScrollbar();
}
//-------------------------------------------------------------------------------
void TraceView::setScaleFactor(float factor)
{
    mXFactor = factor;
    viewport()->update();
    updateScrollbar();
}
//-------------------------------------------------------------------------------
void TraceView::setSelection(int pos, int length)
{

    mCurrentSelection = {pos, length};
    viewport()->update();

    //int x = (pos - mXStart) * mXFactor;

    //horizontalScrollBar()->setValue(x);


}
//-------------------------------------------------------------------------------
bool TraceView::toSvg(const QString &filename)
{
    QSvgGenerator generator;
    generator.setFileName(filename);
    generator.setSize(viewport()->size());
    generator.setViewBox(viewport()->rect());
    generator.setTitle("Sanger trace file");
    generator.setDescription(tr("An SVG rendering from cutepeaks"));

    QPainter painter;
    painter.begin(&generator);
    drawAll(painter);
    painter.end();

    return true;
}
//-------------------------------------------------------------------------------
bool TraceView::toPng(const QString &filename)
{
    QPixmap image(viewport()->rect().size());
    image.fill(Qt::white);
    QPainter painter;
    painter.begin(&image);
    drawAll(painter);
    painter.end();
    return image.save(filename);
}



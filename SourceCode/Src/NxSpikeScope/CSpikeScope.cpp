#include "CSpikeScope.h"

#include "CSpikePlot.h"
#include "ui_CSpikeScope.h"
#include <QGraphicsDropShadowEffect>
#include <QMouseEvent>
#include <QDebug>

CSpikeScope::CSpikeScope(const QString& channelName_, const int sampleRate_, QWidget* parent)
    : FramelessWidget(parent)
    , ui(std::make_unique<Ui::CSpikeScope>())
{
    ui->setupUi(this);
    setMouseTracking(true);// 设置鼠标跟踪，不然只会在鼠标按下时才会触发鼠标移动事件
    // 设置无边框和背景透明
    setAttribute(Qt::WA_TranslucentBackground, true);
    Qt::WindowFlags flags = Qt::Dialog;
    flags |= Qt::FramelessWindowHint;
    setWindowFlags(flags);

    // 设置阴影效果
    const auto shadowEffect = new QGraphicsDropShadowEffect(this);
    shadowEffect->setOffset(0, 0);
    shadowEffect->setColor(Qt::black);
    shadowEffect->setBlurRadius(15);
    this->setGraphicsEffect(shadowEffect);

    spikePlot = new CSpikePlot(channelName_, sampleRate_,this);
    ui->spikePlotLayout->addWidget(spikePlot);
    ui->controlPannel->hide();

    connect(ui->timeScales,SIGNAL(currentIndexChanged(int)), this,SLOT(slotUpdateTimeScale(int)));
    connect(ui->clearBtn, SIGNAL(clicked()), spikePlot, SLOT(slotClearSpikes()));
    connect(ui->clearSnapshotBtn, SIGNAL(clicked()), spikePlot, SLOT(slotClearSnapshot()));
    connect(ui->takeSnapShot, SIGNAL(clicked()), spikePlot, SLOT(slotTakeSnapshot()));
    connect(ui->showCnts, SIGNAL(currentIndexChanged(int)), this, SLOT(slotSetMaxSpikeCount(int)));
    connect(ui->closeBtn, SIGNAL(clicked()), this, SIGNAL(signalCloseSpikeScope()));
    connect(ui->threshold, SIGNAL(valueChanged(int)), spikePlot, SLOT(slotUpdateThreshold(int)));
    connect(spikePlot, SIGNAL(signalThresholdChanged(int)), ui->threshold, SLOT(setValue(int)));
    connect(ui->threshold, SIGNAL(valueChanged(int)), spikePlot, SLOT(slotUpdateThreshold(int)));
    update();
}

CSpikeScope::~CSpikeScope() = default;

void CSpikeScope::slotUpdateTimeScale( int index_) 
{
    spikePlot->slotUpdateTimeScale((index_+1) * 2);
}

void CSpikeScope::slotSetMaxSpikeCount(const int index)
{
    const int counts[] = { 10, 20, 50,100, 500 };
    spikePlot->slotSetMaxSpikeCount(counts[index]);
}

void CSpikeScope::setWaveformName(const QString& channelName, const int sampleRate_) 
{
    spikePlot->slotSetWaveformName(channelName, sampleRate_);
}

void CSpikeScope::updateWaveforms(const QVector<float>& waveformSegment) const
{
    spikePlot->slotUpdateWaveforms(waveformSegment);
}

void CSpikeScope::clearSpikes() const
{
    spikePlot->slotClearSpikes();
}

bool CSpikeScope::isEnabledDrag(QMouseEvent* e)
{
    if (const auto w = childAt(e->pos()))
    {
        if (w == ui->toolbar)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }
}

#include "CSpectrogram.h"
#include "CSpectrogramPlot.h"
#include "ui_CSpectrogram.h"

BEGIN_NX_NAMESPACE
CSpectrogram::CSpectrogram(QWidget* parent)
    :QWidget(parent)
    , ui(new Ui::CSpectrogram)
{
    ui->setupUi(this);

    specPlot = new CSpectrogramPlot(this);
    ui->plotContainer->addWidget(specPlot);
    ui->left->hide();

    connect(ui->nFftSlider, SIGNAL(valueChanged(int)), this, SLOT(setNumFftPoints(int)));

    connect(ui->fMaxSpinBox, SIGNAL(valueChanged(int)), this, SLOT(setFMax(int)));

    connect(ui->fMinSpinBox, SIGNAL(valueChanged(int)), this, SLOT(setFMin(int)));

    connect(ui->fMarkerSpinBox, SIGNAL(valueChanged(int)), this, SLOT(setFMarker(int)));

    connect(ui->fMarkerShowCheckBox, SIGNAL(clicked(bool)), this, SLOT(toggleShowFMarker(bool)));

    connect(ui->fMarkerHarmonicsPinBox, SIGNAL(valueChanged(int)), this, SLOT(setNumHarmonics(int)));

    connect(ui->timeScaleComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(setTimeScale(int)));

    //connect(displayModeButtonGroup, SIGNAL(buttonClicked(int)), this, SLOT(changeDisplayMode(int)));

    updateDeltaTimeFreqLabels();

}

CSpectrogram::~CSpectrogram()
{
}

void CSpectrogram::changeCurrentChannel(const QString& customChannelName, const QString& comments, const double sampleRate)
{
    std::lock_guard lock(plotMutex);
    specPlot->setWaveform(customChannelName, comments, sampleRate);
}

void CSpectrogram::resetBuffer()
{
    std::lock_guard lock(plotMutex);
    specPlot->resetQueue();
}

void CSpectrogram::setBackground(const QColor color)
{
    std::lock_guard lock(plotMutex);
    specPlot->setBackground(color);
}

void CSpectrogram::updateWaveforms(int numSamples, const QVector<float>& data, const QVector<uint64_t>& timestamps)
{
    std::lock_guard lock(plotMutex);
    specPlot->updateWaveforms(numSamples, data, timestamps);
}

QString CSpectrogram::getCurentCustomWaveName()
{
    std::lock_guard lock(plotMutex);
    return specPlot->getCustomWaveName();
}

void CSpectrogram::activate()
{
    std::lock_guard lock(plotMutex);
    specPlot->resetSpectrogram();
    show();
}

void CSpectrogram::changeDisplayMode(int index)
{
    std::lock_guard lock(plotMutex);
    bool timeScaleOn = (index == 0);
    ui->timeScaleLabel->setEnabled(timeScaleOn);
    ui->timeScaleComboBox->setEnabled(timeScaleOn);
    if (index == 0)
    {
        specPlot->setDisplayMode(Spectrogram);
    }
    else
    {
        specPlot->setDisplayMode(Spectrum);
    }
    
}

void CSpectrogram::setNumFftPoints(int index)
{
    std::lock_guard lock(plotMutex);
    specPlot->setFftSize(128 * std::pow(2, index));
}

void CSpectrogram::setFMin(int fMin)
{
    std::lock_guard lock(plotMutex);
    specPlot->setFreqMin(fMin);
}

void CSpectrogram::setFMax(int fMax)
{
    std::lock_guard lock(plotMutex);
    specPlot->setFreqMax(fMax);
}

void CSpectrogram::setFMarker(int fMarker)
{
    std::lock_guard lock(plotMutex);
    specPlot->setFreqMarkerValue(fMarker);
}

void CSpectrogram::toggleShowFMarker(bool enabled)
{
    std::lock_guard lock(plotMutex);
    ui->fMarkerSpinBox->setEnabled(enabled);
    ui->fMarkerLabel->setEnabled(enabled);
    ui->fMarkerHarmonicsPinBox->setEnabled(enabled);
    ui->fMarkerHarmonicsLabel1->setEnabled(enabled);
    ui->fMarkerHarmonicsLabel2->setEnabled(enabled);
    specPlot->setShowMarker(enabled);
}

void CSpectrogram::setNumHarmonics(int num)
{
    std::lock_guard lock(plotMutex);
    specPlot->setNumHarmonics(num);
}

void CSpectrogram::setTimeScale(int index)
{
    std::lock_guard lock(plotMutex);
    int scaleList[] = {2, 5, 10}; // 2s 5s 10s
    specPlot->setTScale(scaleList[index]);

}

void CSpectrogram::updateDeltaTimeFreqLabels()
{
    double deltaTimeMsec = specPlot->getDeltaTimeMsec();
    double deltaFreqHz = specPlot->getDeltaFreqHz();

    int digitsAfterDecimalTime = deltaTimeMsec >= 100.0 ? 0 : 1;
    int digitsAfterDecimalFreq = deltaFreqHz >= 10.0 ? 1 : 2;
    if (deltaFreqHz >= 100.0) digitsAfterDecimalFreq = 0;

    //deltaTimeLabel->setText(QString::number(deltaTimeMsec, 'f', digitsAfterDecimalTime) + tr(" ms"));
    //deltaFreqLabel->setText(QString::number(deltaFreqHz, 'f', digitsAfterDecimalFreq) + tr(" Hz"));
}



END_NX_NAMESPACE



#include "COscillograph.h"
#include "../CProgressDialog.h"
#include "../CNxMessageBox.h"
#include "NxWaveformWidget/MultiColumnDisplay.h"
#include "NxSpectrogram/CSpectrogram.h"
#include "NxSpikeScope/CSpikeScope.h"
#include "NxAppService/COscillographService.h"
#include "NxEntity/ChannelPlan.h"
#include "NxEntity/ChannelName.h"
#include "NxSystemCfg/CSystemCfg.h"
#include "NxEntity/Patient.h"
#include <QDateTime>
#include <algorithm>
#include <QVector>

BEGIN_NX_NAMESPACE

const int WaveformPageIndex = 0;
const int SpectrogramPageIndex = 1;
const double SpectrogramFftSizeList[] = { 128.0, 256.0, 512.0, 1024.0, 2048.0, 4096.0, 8192.0 };
const int rmsTimeScaleList[] = { 1,2,4,6,8,10,20 }; // ms
const int snrTimeScaleList[] = { 1,2,4,6,8,10,20 }; // ms

//const int g_kSampleRateList[] = { 1000, 1250, 1500, 2000, 2500, 3000, 3333, 4000, 5000, 6250, 8000, 10000, 12500, 15000, 20000, 25000, 30000};
COscillograph::COscillograph(COscillographService* _service, const bool _isPlayback, QWidget* parent)
    :QWidget(parent)
    , ui(new Ui::Oscillograph())
    , spectrogram(nullptr)
    , spikeScope(nullptr)
    , service(_service)
    , progressDialog(nullptr)
    , currentCheckedBtn(ID_SIGNAL)
    , sampleRate(1000)
    , isInited(false)
    , isPlayback(_isPlayback)
{
    ui->setupUi(this);

    Patient patient;
    service->getCurrentPatient(patient);
    displayStatus.setYScale(patient.getSensitivityIndex());
    displayStatus.setPaperSpeed(patient.getPaperSpeedIndex());

    // 设置波形显示组件
    ui->timeBox->setVisible(false);
    ui->label_3->setVisible(false);
    ui->signalParamStack->setCurrentWidget(ui->signalParamPage);
    service->setVisibleWaveNamesHandle([=]() {  return displayStatus.getDisplayWaveformNames();  });
    service->changeOsMode(OS_SIGNAL);

    waveforms = new MultiColumnDisplay(this);
    ui->waveformContainer->addWidget(waveforms);
    connect(waveforms, SIGNAL(signalUpdateDisplayParam()), SLOT(slotUpdateDisplayParams()));
    connect(waveforms, SIGNAL(signalToggleSelectedWaveforms()), SLOT(slotUpdateDisabledChannels()));
    connect(waveforms, SIGNAL(signalUpdateSelectedWavename(QString)), SLOT(slotChangeSelctedWaveName(QString)));
    qRegisterMetaType<uint64_t>("uint64_t");
    qRegisterMetaType<QVector<QMap<std::string, std::string>>>("QVector<QMap<std::string, std::string>>");
    connect(service, SIGNAL(signalUpdateWaveform(bool, bool, int, uint64_t, uint64_t,bool, QVector<QMap<std::string, std::string>>)),
        SLOT(slotUpdateWaveform(bool, bool, int, uint64_t, uint64_t,bool, QVector<QMap<std::string, std::string>>)), Qt::QueuedConnection);

    // 设置频谱图组件
    service->setLoadSelectedAmpDataCallback([=](const QString& nativeName, int numSamples, const QVector<float>& data,
        const QVector<uint64_t>& timestamps) {
            slotUpdateSelectedAmpData(nativeName, numSamples, data, timestamps);
        });
    
    ui->sensitiveCombo->clear();
    for (int i = 0; i < displayStatus.getYScalerNumOfItems(); i++)
    {
        ui->sensitiveCombo->addItem(QString::fromStdString(displayStatus.getYScaleName(i)));
    }
    connect(ui->sensitiveCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(slotYScaleChanged()));

    ui->paperSpeedCombo->clear();
    for (int i = 0; i < displayStatus.getPaperSpeedNumberOfItems(); i++)
    {
        ui->paperSpeedCombo->addItem(QString::fromStdString(displayStatus.getPaperSpeedName(i)));
    }
    connect(ui->paperSpeedCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(slotPaperSpeedChanged()));

    connect(ui->signalBtn, SIGNAL(clicked()), SLOT(slotSignalBtnClicked()));
    connect(ui->snrBtn, SIGNAL(clicked()), SLOT(slotSNRBtnClicked()));
    connect(ui->rmsBtn, SIGNAL(clicked()), SLOT(slotRMSBtnClicked()));
    connect(ui->spectrumBtn, SIGNAL(clicked()), SLOT(slotSpectrumBtnClicked()));
    connect(ui->timeBox, &QComboBox::currentTextChanged,this, &COscillograph::changeTimeScale);
    qRegisterMetaType<QMap<QString, QString>>("QMap<QString, QString>");
    connect(service, SIGNAL(signalBoardInited(bool, QMap<QString, QString>)), this, SLOT(slotBoardInited(bool, QMap<QString, QString>)));
    if(isPlayback)
    {
        showProgressDialog(tr("Playback is initing, please wait!"));
    }
    else
    {
        showProgressDialog(tr("Device is connecting, please wait!"));
    }
    
}

COscillograph::~COscillograph() = default;

bool COscillograph::init()
{
    initWaveformChannels();
    if (isPlayback)
    {
        initPlayback();
        return  true;
    }
    else
    {
        return  initBoard();
    }
}

void COscillograph::initWaveformChannels()
{
    int order = 0;
    signalManager.reset();
    auto channelPlans = service->getChannelPlans();
    if (!channelPlans.empty())
    {
        sampleRate = channelPlans[0].SampleRate;
    }

    for (auto& plan : channelPlans)
    {
        auto channelNames = service->getChannelNamesByPlan(QString::fromStdString(plan.UID));
        auto disabledChannelNumbers = plan.getDisabledChannelNumbers().toList();
        for (int i = 0; i < plan.ChannelCounts; ++i)
        {
            UiChannel* channel = new UiChannel();
            QString groupName = QString::fromStdString(plan.Indexs);
            QString nativeName = groupName + QString("-%1").arg(i, 3, 10, QLatin1Char('0'));
            QString customName = groupName + QString("-%1").arg(i+1, 3, 10, QLatin1Char('0'));
            for (auto& name : channelNames)
            {
                if (nativeName == groupName + QString("-%1").arg(QString::fromStdString(name.ChannelNumber).toInt() - 1, 3, 10, QLatin1Char('0')))
                {
                    customName = QString::fromStdString(name.CustomName);
                }
            }
            if (disabledChannelNumbers.contains(QString::number(i + 1))) {
                channel->setEnabled(false);
            }

            channel->setColor(channelColor(i, 32).name());
            channel->setGroupName(groupName);
            channel->setNativeName(nativeName);
            channel->setCustomName(customName);
            channel->setUserOrder(order);
            channel->setSampleRate(sampleRate); 
            signalManager.addChannel(channel);
            //displayStatus.setRollMode(true);
            samplesMap.insert(channel->getNativeName(), new QVector<float>());
            order++;
        }
    }

    initChannelImpedance();

    waveforms->updateForRescan();
}

void COscillograph::initChannelImpedance() const
{
    const QDir impedanceDir(service->getImpedanceDataPath());
    auto list = impedanceDir.entryList(QStringList() << "*.csv", QDir::Files, QDir::Time);
    if (list.count() <= 0) { return; }

    const auto filePath = service->getImpedanceDataPath() + "/" + list[0];
    QFile file(filePath);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMap<QString, QColor> channelColors;
        QTextStream in(&file);
        QString line = in.readLine();// 首行表头
        while (!line.isNull())//字符串有内容
        {
            bool ok = true;
            line = in.readLine();//循环读取下行
            if (line.isEmpty())
            {
                continue;
            }
            auto nativeName = line.split(",")[0];
            const double magnitude = line.split(",")[4].toDouble(&ok);
            const double phase = line.split(",")[5].toDouble(&ok);

            QString magnitudeStr = "";
            if (magnitude >= 1000000) {
                magnitudeStr = QString::number(magnitude / 1000000.0, 'f', 2) + QString::fromLocal8Bit("MΩ");
            }
            else if (magnitude >= 1000) {
                magnitudeStr = QString::number(magnitude / 1000.0, 'f', 2) + QString::fromLocal8Bit("kΩ");
            }
            else {
                magnitudeStr = QString::number(magnitude) + QString::fromLocal8Bit("Ω");
            }

            const auto channel = signalManager.channelByName(nativeName);
            if (channel != nullptr)
            {
                channel->setImpednanceValided(true);
                channel->setImpedanceMagnitude(magnitude);
                channel->setImpedancePhase(phase);
            }
        }
    }
}

bool COscillograph::initBoard() const
{
    // Init signal sources
  return   service->initBoard(sampleRate);
}

void COscillograph::initPlayback() const
{
    // Init signal sources
    service->initPlayback(sampleRate);
}

void COscillograph::slotSignalBtnClicked() const
{
    const auto btns = { ui->snrBtn, ui->rmsBtn, ui->spectrumBtn };
    ui->timeBox->setVisible(false);
    ui->label_3->setVisible(false);
    ui->signalParamStack->setCurrentWidget(ui->signalParamPage);
    ui->label->setVisible(true);
    ui->paperSpeedCombo->setVisible(true);
    ui->label_2->setVisible(true);
    ui->sensitiveCombo->setVisible(true);
    for (const auto btn : btns)
    {
        btn->setChecked(false);
    }
    ui->signalBtn->setChecked(true);

    service->setSignalInfo(WaveFormType::WAVEFORM);

    ui->oscillographContainer->setCurrentIndex(WaveformPageIndex);

    service->changeOsMode(OS_SIGNAL);

    ui->sensitiveCombo->setEnabled(true);
    ui->paperSpeedCombo->setEnabled(true);
    displayStatus.setShowRuller(true);
    waveforms->reset();
}

void COscillograph::slotSNRBtnClicked()
{
    const auto btns = { ui->signalBtn, ui->rmsBtn, ui->spectrumBtn };
    ui->label->setVisible(false);
    ui->paperSpeedCombo->setVisible(false);
    ui->label_2->setVisible(false);
    ui->sensitiveCombo->setVisible(false);
    for (const auto btn : btns)
    {
        btn->setChecked(false);
    }    
    changeTimeScale();
    ui->snrBtn->setChecked(true);
    ui->timeBox->setVisible(true);
    ui->label_3->setVisible(true);
    ui->signalParamStack->setCurrentWidget(ui->signalParamPage);
    // 需要通过界面来拼出信号类型
    std::vector<int> signalType;
    signalType.reserve(256);
    for (int i = 0; i < 256; i++)
    {
        signalType.push_back(0);
    }
    service->setSignalType(signalType);

    service->setSignalInfo(WaveFormType::SNR);
    ui->oscillographContainer->setCurrentIndex(WaveformPageIndex);
    service->changeOsMode(OS_SIGNAL);

    ui->sensitiveCombo->setEnabled(false);
    ui->paperSpeedCombo->setEnabled(true);
    displayStatus.setShowRuller(false);
    waveforms->reset();
}

void COscillograph::slotRMSBtnClicked()
{
    const auto btns = { ui->signalBtn, ui->snrBtn,  ui->spectrumBtn };
    for (const auto btn : btns)
    {
        btn->setChecked(false);
    }
    ui->label->setVisible(false);
    ui->paperSpeedCombo->setVisible(false);
    ui->label_2->setVisible(false);
    ui->sensitiveCombo->setVisible(false);
    ui->rmsBtn->setChecked(true);
    changeTimeScale();
    service->setSignalInfo(WaveFormType::RMS);
    ui->timeBox->setVisible(true);
    ui->label_3->setVisible(true);
    ui->signalParamStack->setCurrentWidget(ui->signalParamPage);
    ui->oscillographContainer->setCurrentIndex(WaveformPageIndex);
    service->changeOsMode(OS_SIGNAL);

    ui->sensitiveCombo->setEnabled(true);
    ui->paperSpeedCombo->setEnabled(true);
    displayStatus.setShowRuller(true);
    waveforms->reset();
}

void COscillograph::slotSpectrumBtnClicked()
{
    if (nullptr == spectrogram)
    {
        spectrogram = new CSpectrogram(this);
        const auto cfg = systemConfig.GetSystemCfg().Spectrogram;
        spectrogram->changeDisplayMode(cfg.SpectrogramMode);
        spectrogram->setFMax(cfg.MaxFreq);
        spectrogram->setFMin(cfg.MinFreq);
        spectrogram->setTimeScale(cfg.TScale);
        spectrogram->setFMarker(cfg.FreqMarkerValue);
        spectrogram->setNumHarmonics(cfg.HarmonicsNum);
        spectrogram->toggleShowFMarker(cfg.ShowFreqMarker);
        ui->resolutionCombo->clear();
        for (const auto fftSize : SpectrogramFftSizeList) {
            auto itemText = QString::number(fftSize / sampleRate * 1000,'f',1) + "ms";
            ui->resolutionCombo->addItem(itemText);
        }
        ui->resolutionCombo->setCurrentIndex(1);
        connect(ui->resolutionCombo, SIGNAL(currentIndexChanged(int)), SLOT(slotSpectrogramResolutionChanged(int)));
        connect(ui->spectrogramTimeScale, SIGNAL(currentIndexChanged(int)), SLOT(slotSpectrogramTimeScaleChanged(int)));
        connect(ui->spectrogramMode, SIGNAL(currentIndexChanged(int)), SLOT(slotSpectrogramModeChanged(int)));
        connect(ui->spectrogramMinFreq, SIGNAL(valueChanged(int)), SLOT(slotSpectrogramMinFreqChanged(int)));
        connect(ui->spectrogramMaxFreq, SIGNAL(valueChanged(int)), SLOT(slotSpectrogramMaxFreqChanged(int)));
        ui->spectrumContainer->addWidget(spectrogram);

        const auto curChannel = signalManager.selectedChannel();
        if (curChannel) {
            QString comment = "";
            if (curChannel->isImpedanceValided()) {
                comment = curChannel->getImpedanceMagnitudeString();
            }
            spectrogram->changeCurrentChannel(curChannel->getCustomName(), comment,curChannel->getSampleRate());
        }
    }
    spectrogram->resetBuffer();

    const auto btns = { ui->signalBtn, ui->snrBtn, ui->rmsBtn };
    for (const auto btn : btns)
    {
        btn->setChecked(false);
    }

    ui->signalParamStack->setCurrentWidget(ui->spectrogramParamPage);

    ui->spectrumBtn->setChecked(true);
    service->setSignalInfo(SPECTRUM);

    ui->oscillographContainer->setCurrentIndex(SpectrogramPageIndex);
    service->changeOsMode(OS_SPECTROGRAM);

    ui->sensitiveCombo->setEnabled(false);
    ui->paperSpeedCombo->setEnabled(false);
}

void COscillograph::slotBoardInited(bool initRes, QMap<QString, QString> channelInfos)
{
    isInited = initRes;
    if (!isInited)
    {
        CNxMessageBox msgBox(tr("Connecting Device"), tr("Init device failed, please check connection. Click \"OK\" to return to the previous page."));
        msgBox.exec();
        const bool confirmed = msgBox.isConfirmed();
        closeProgressDialog();
        if (confirmed)
        {
            if (isPlayback)
            {
                initPlayback();
            }
            else
            {
                initBoard();
            }

        }
        else {
            emit signalExitOscillograph();
        }
        return;
    }

    waveforms->updateForRescan();
    const auto channelPlans = service->getChannelPlans();

    if (!channelPlans.empty())
    {
        const QString nativeName = signalManager.channelByIndex(0)->getNativeName();
        slotChangeSelctedWaveName(nativeName);
    }

    for (auto& plan : channelPlans)
    {
        auto portName = QString::fromStdString(plan.Indexs);
        auto notchType = QString::fromStdString(plan.Notch);

        service->updateFilterParam(plan);
    }

    if (isPlayback) {
        service->setPlaybackStopFunc([this] {
            emit signalServerIsStoped();
            });
    }

    const auto curChannel = signalManager.selectedChannel();
    if (spikeScope && curChannel)
    {
        spikeScope->setWaveformName(signalManager.selectedChannel()->getCustomName(),
            signalManager.selectedChannel()->getSampleRate());
    }

    if (spectrogram && curChannel)
    {
        
        QString comment = "";
        if (curChannel->isImpedanceValided()) {
            comment = curChannel->getImpedanceMagnitudeString();
        }
        spectrogram->changeCurrentChannel(curChannel->getCustomName(),
                       comment, curChannel->getSampleRate());
    }

    closeProgressDialog();
}

void COscillograph::slotUpdateDisplayParams()
{
    if (!isInited)
    {
        return;
    }

    const auto timespan = displayStatus.getTimespanPerScreen();
    const auto dispWidth = displayStatus.getDispWaveAreaLength();
    const auto refreshZoneCnt = displayStatus.getRefreshZoneNum();
    displayStatus.setSweepFirstTime(true);
    if (displayStatus.getPaperSpeedIndex() != ui->paperSpeedCombo->currentIndex())
    {
        ui->paperSpeedCombo->setCurrentIndex(displayStatus.getPaperSpeedIndex());
    }
    if (displayStatus.getYScaleIndex() != ui->sensitiveCombo->currentIndex())
    {
        ui->sensitiveCombo->setCurrentIndex(displayStatus.getYScaleIndex());
    }

    service->updateDisplayParam(timespan, dispWidth, refreshZoneCnt, displayStatus.isRollMode());
}

void COscillograph::slotUpdateWaveform(bool isSweepFirstTime,
    bool isUseVerticalLines,
    int validDataIndex, 
    uint64_t startTimestamp, uint64_t endTimestamp, bool isRunning, QVector<QMap<std::string, std::string>> waveformSegements)
{
    if (!isRunning)
    {
        return;
    }

    if (isUseVerticalLines)
    {
        for (auto& wavename : waveformSegements[0].keys())
        {
            SignalSegment* segment = dto.getSignalSegment(QString::fromStdString(wavename));
            if (segment == nullptr)
            {
                segment = new SignalSegment();
                segment->wavename = QString::fromStdString(wavename);
            }
            const auto minSrc = waveformSegements[0][wavename].data();
            const auto maxSrc = waveformSegements[1][wavename].data();
            const auto size = waveformSegements[0][wavename].size();
            segment->yMinData.resize(size / sizeof(float));
            segment->yMaxData.resize(size / sizeof(float));
            memcpy(segment->yMinData.data(), minSrc, size);
            memcpy(segment->yMaxData.data(), maxSrc, size);
            dto.addSignalSegment(segment);
        }
    }
    else
    {
        for (auto& wavename : waveformSegements[0].keys())
        {
            SignalSegment* segment = dto.getSignalSegment(QString::fromStdString(wavename));
            if (segment == nullptr)
            {
                segment = new SignalSegment();
                segment->wavename = QString::fromStdString(wavename);
            }
            const auto src = waveformSegements[0][wavename].data();
            const auto size = waveformSegements[0][wavename].size();
            segment->yData.resize(size / sizeof(float));
            memcpy(segment->yData.data(), src, size);
            dto.addSignalSegment(segment);
        }
    }
    displayStatus.setValidDataIndex(validDataIndex);
    displayStatus.setSweepFirstTime(isSweepFirstTime);
    updateTimeAxisInSweepMode(startTimestamp, endTimestamp);
    if(const auto channel = signalManager.channelByIndex(0))
    {
        updateRunTime(endTimestamp, channel->getSampleRate());
    }
    
    waveforms->loadWaveformData(dto);
    qApp->processEvents();
    //qDebug() << "slotUpdateWaveform done ***************";
}

// 更新时间戳
void COscillograph::updateTimeAxisInSweepMode(const uint64_t startTimestamp, const uint64_t endTimestamp)
{
    auto oldTimeAxisLabels = displayStatus.getTimeAxisLabels();
    
    displayStatus.clearTimeAxisLabels();
    if (signalManager.numEnabledChannels() == 0)
    {
        return;
    }

    const auto firstChannel = signalManager.channelByIndex(0);
    const auto sampleRate = firstChannel->getSampleRate();
    const auto wavename = firstChannel->getNativeName();
    auto segment = dto.getSignalSegment(wavename);
    const auto displayWidth = displayStatus.getDispWaveAreaLength();
    int samplesPerZone = displayStatus.getTimespanPerScreen() / displayStatus.getRefreshZoneNum() * sampleRate;

    const int leftSampleOffset = ((int)sampleRate - startTimestamp % (int)sampleRate) % (int)sampleRate;
    const int firstTime = (startTimestamp + leftSampleOffset) / (int)sampleRate;
    const int lastTime = endTimestamp / (int)sampleRate;

    const double pixelsPerSample = displayWidth / (displayStatus.getTimespanPerScreen() * sampleRate);
    int sampleIndex = 0;
    int timestampPosition = 0;
    for (int i = 0; i < lastTime - firstTime + 1; ++i)
    {
        const auto time = firstTime + i;
        const QString stamp = service->getRecordStartDateTime().addSecs(time).time().toString();
        sampleIndex = i * sampleRate + leftSampleOffset;
        timestampPosition = sampleIndex * pixelsPerSample;
        displayStatus.insertTimeAxisLabel(timestampPosition, stamp);
    }

    return;
    // 更新validIndex之后的时间戳
    if (!displayStatus.isSweepFirstTime() && oldTimeAxisLabels.count() > 0)
    {
        for (auto oldPosition : oldTimeAxisLabels.keys())
        {
            if (oldPosition > timestampPosition + sampleRate * pixelsPerSample / 2)
            {
                displayStatus.insertTimeAxisLabel(oldPosition, oldTimeAxisLabels[oldPosition]);
            }
        }
    }
}

// 更新运行时间
void COscillograph::updateRunTime(const uint64_t endTimestamp,const double sampleRate)
{
    const uint64_t runTimeInSecs = endTimestamp / sampleRate;
    const uint64_t runTimeInMSecs = runTimeInSecs*1000 + (double)(endTimestamp % (int)sampleRate) / sampleRate * 1000;
    service->setRuntime(runTimeInMSecs);
    emit signalUpdateTimestamp(runTimeInSecs+1);
}

void COscillograph::slotPaperSpeedChanged()
{
    displayStatus.setPaperSpeed(ui->paperSpeedCombo->currentIndex());
    waveforms->updateFromState();
    Patient patient;
    service->getCurrentPatient(patient);
    patient.setPaperSpeedIndex(ui->paperSpeedCombo->currentIndex());
    service->updatePatient(patient);
}

void COscillograph::slotYScaleChanged()
{
    displayStatus.setYScale(ui->sensitiveCombo->currentIndex());
    Patient patient;
    service->getCurrentPatient(patient);
    patient.setSensitivityIndex(ui->sensitiveCombo->currentIndex());
    service->updatePatient(patient);
}

void COscillograph::slotUpdateDisabledChannels()
{
    QStringList disabledChannels;
    bool hasEnableAndSelected = false;
    QStringList enabledChannels;
    for (int i = 0; i < signalManager.numChannels(); ++i)
    {
        const auto channel = signalManager.channelByIndex(i);
        if (!channel->isEnabled())
        {
            disabledChannels.append(channel->getCustomName());
            channel->setIsSelected(false);
        }
        else {
            if (channel->isSelected())
            {
                hasEnableAndSelected = true;
            }
            enabledChannels.append(channel->getNativeName());
        }
    }

    if (!hasEnableAndSelected && enabledChannels.count() > 0) {
        slotChangeSelctedWaveName(enabledChannels[0]);
    }
    
    emit signalUpdateDisabledWaveforms(disabledChannels);
}

void COscillograph::slotEnableChannel(const QString& customWavename)
{
    for (int i = 0; i < signalManager.numChannels(); ++i)
    {
        const auto channel = signalManager.channelByIndex(i);
        if (customWavename == channel->getCustomName())
        {
            channel->setEnabled(true);
            channel->setIsSelected(true);
            slotChangeSelctedWaveName(channel->getNativeName());
        }
        else {
            channel->setIsSelected(false);
        }
    }
}

void COscillograph::slotSpectrogramResolutionChanged(int index)
{
    if (spectrogram) {
        auto fftSize = SpectrogramFftSizeList[index];
        spectrogram->setNumFftPoints(index);
    }
}

void COscillograph::slotUpdateSelectedAmpData(const QString& nativeName, const int numSamples, const  QVector<float>& data, const  QVector<uint64_t>& timestamps)
{
    if(numSamples == 0 )
    {
        return;
    }

    if (spectrogram )
    {
        const auto channel = signalManager.channelByName(nativeName);
        updateRunTime(timestamps.last(), channel->getSampleRate());
        spectrogram->updateWaveforms(numSamples, data, timestamps);
    }

    if(spikeScope)
    {
        spikeScope->updateWaveforms(data);
    }
    
}

void COscillograph::slotChangeSelctedWaveName(QString nativeName)
{
    const auto channel = signalManager.channelByName(nativeName);
    channel->setIsSelected(true);
    service->setSelectedWaveName(channel->getNativeName());
    if (spectrogram)
    {
        QString comment = "";
        if (channel->isImpedanceValided()) {
            comment = channel->getImpedanceMagnitudeString();
        }
        spectrogram->changeCurrentChannel(channel->getCustomName(), comment, channel->getSampleRate());
    }

    if(spikeScope)
    {
        spikeScope->setWaveformName(channel->getCustomName(), channel->getSampleRate());
    }
}

QColor COscillograph::channelColor(const int colorIndex, const int numColors)
{
    const int index = colorIndex % numColors;
    constexpr int numCycles = 3;
    double hue = numCycles * (static_cast<double>(index) / static_cast<double>(numColors));
    while (hue > 1.0) {
        hue -= 1.0;
    }
    constexpr double saturation = 0.50;
    constexpr double value = 0.50;
    return QColor::fromHslF(hue, saturation, value);
}

void COscillograph::showProgressDialog(const QString& content)
{
    if (nullptr == progressDialog) {
        progressDialog = new CProgressDialog(Qt::ApplicationModal, this);
    }
    progressDialog->setContent(content);
    progressDialog->show();
    qApp->processEvents();
}

void COscillograph::updateProgressContent(const QString& content) const
{
    if (nullptr != progressDialog) {
        progressDialog->setContent(content);
    }
}

void COscillograph::closeProgressDialog() 
{
    if (nullptr != progressDialog) {
        progressDialog->close();
        delete progressDialog;
        progressDialog = nullptr;
    }
}

void COscillograph::resetWaveform() const
{
    waveforms->updateFromState();
    waveforms->reset();
}

void COscillograph::slotChannelPlanToggled(const QString& planUid, bool enabled)
{
    const auto plan = service->getChannelPlanByUid(planUid);
    for (int i = 0; i < plan.ChannelCounts; ++i)
    {
        auto nativeName = QString::fromStdString(plan.Indexs) + QString("-%1").arg(i, 3, 10, QLatin1Char('0'));
        signalManager.setChannelEnabled(nativeName, enabled);
    }

    slotUpdateDisabledChannels();
}
void COscillograph::changeTimeScale() const
{
    service->setTimeScale(snrTimeScaleList[ui->timeBox->currentIndex()]);
}

void COscillograph::slotSpectrogramTimeScaleChanged(const int index) const
{
    if (spectrogram) {
        spectrogram->setTimeScale(index);
    }
}

void COscillograph::slotSpectrogramModeChanged(const int index) const
{
    if (spectrogram) {
        spectrogram->changeDisplayMode(index);
    }
}

void COscillograph::slotSpectrogramMinFreqChanged(int val) const
{
    if (spectrogram) {
        spectrogram->setFMin(val);
        ui->spectrogramMaxFreq->setMinimum(val + 11);
    }
}

void COscillograph::slotSpectrogramMaxFreqChanged(int val) const
{
    if (spectrogram) {
        spectrogram->setFMax(val);
        ui->spectrogramMinFreq->setMaximum(val - 11);
    }
}

void COscillograph::slotToggleSpikeScope()
{
    if(spikeScope && spikeScope->isVisible())
    {
        slotSpikeScopeClosed();
    }
    else
    {
        if (!spikeScope)
        {
            const auto curChannel = signalManager.selectedChannel();
            spikeScope = new CSpikeScope(curChannel->getCustomName(), curChannel->getSampleRate(), this);
            spikeScope->setWaveformName(curChannel->getCustomName(), curChannel->getSampleRate());
            connect(spikeScope, SIGNAL(signalCloseSpikeScope()), this, SLOT(slotSpikeScopeClosed()));
        }

        spikeScope->show();
        spikeScope->raise();
    }
}

void COscillograph::slotSpikeScopeClosed()
{
    spikeScope->close();
    delete spikeScope;
    spikeScope = nullptr;
}

void COscillograph::slotResetSampleRate()
{
    if(service->getRunStatus()== RS_RUNNING)
    {
        service->stopRun();
    }

    if (service->getRunStatus() == RS_RECORDING)
    {
        service->stopRun();
    }

    showProgressDialog(tr("Changing sample rate, please wait!"));
    if(waveforms)
    {
        ui->waveformContainer->removeWidget(waveforms);
        disconnect(waveforms, SIGNAL(signalUpdateDisplayParam()),this, SLOT(slotUpdateDisplayParams()));
        disconnect(waveforms, SIGNAL(signalToggleSelectedWaveforms()), this, SLOT(slotUpdateDisabledChannels()));
        disconnect(waveforms, SIGNAL(signalUpdateSelectedWavename(QString)), this, SLOT(slotChangeSelctedWaveName(QString)));
        waveforms->deleteLater();

        waveforms = new MultiColumnDisplay(this);
        ui->waveformContainer->addWidget(waveforms);
        connect(waveforms, SIGNAL(signalUpdateDisplayParam()), this, SLOT(slotUpdateDisplayParams()));
        connect(waveforms, SIGNAL(signalToggleSelectedWaveforms()), this, SLOT(slotUpdateDisabledChannels()));
        connect(waveforms, SIGNAL(signalUpdateSelectedWavename(QString)), this, SLOT(slotChangeSelctedWaveName(QString)));
    }

    init();
}

END_NX_NAMESPACE


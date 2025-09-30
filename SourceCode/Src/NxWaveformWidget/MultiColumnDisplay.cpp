#include "MultiColumnDisplay.h"
#include "CommunicationDto.h"
#include <functional> 
BEGIN_NX_NAMESPACE
MultiColumnDisplay::MultiColumnDisplay(QWidget *parent) :
    QWidget(parent),
    minusYScaleShortcut(new QShortcut(this)),
    plusYScaleShortcut(new QShortcut(this))
{
    LOG("Beginning of MultiColumnDisplay ctor");

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
//    setFocusPolicy(Qt::StrongFocus);

    QHBoxLayout* mainLayout = new QHBoxLayout;
    mainLayout->setContentsMargins(0, 0, 0, 0);
    setLayout(mainLayout);

    LOG("About to create WaveformDisplayManager");
    waveformManager = new WaveformDisplayManager(100);
    LOG("Created WaveformDisplayManager");

    LOG("About to call addWaveforms()");
    addWaveforms();
    LOG("Completed addWaveforms()");

    LOG("About to call waveformManager->resetAll()");
    waveformManager->resetAll();
    LOG("Completed waveformManager->resetAll()");

    LOG("About to call addColumn(0)");
    addColumn();
    LOG("Completed addColumn(0)");

    tPaperSpeedIndex = displayStatus.getPaperSpeedIndex();
    rollModeFormerValue = displayStatus.isRollMode();

    minusYScaleShortcut->setKey(QKeySequence("-"));
    minusYScaleShortcut->setContext(Qt::ApplicationShortcut);
    connect(minusYScaleShortcut, &QShortcut::activated, [&]() {
        displayStatus.shiftYScale(1);
        emit signalUpdateDisplayParam();
        });

    plusYScaleShortcut->setKey(QKeySequence("="));
    plusYScaleShortcut->setContext(Qt::ApplicationShortcut);
    connect(plusYScaleShortcut, &QShortcut::activated, [&]() {
        displayStatus.shiftYScale(-1);
        emit signalUpdateDisplayParam();
        });

    LOG("End of MultiColumnDisplay ctor");
}

MultiColumnDisplay::~MultiColumnDisplay()
{
    delete waveformManager;
    delete displayColumn;

    delete minusYScaleShortcut;
    delete plusYScaleShortcut;
}

bool MultiColumnDisplay::addColumn()
{
    LOG("About to create WaveformDisplayColumn");
    displayColumn = new WaveformDisplayColumn(0, waveformManager, this);
    connect(displayColumn, SIGNAL(signalToggleSelectedWaveforms()), SIGNAL(signalToggleSelectedWaveforms()));
    connect(displayColumn, SIGNAL(signalUpdateSelectedWavename(QString)), SIGNAL(signalUpdateSelectedWavename(QString)));
    LOG("About to call updateColumnIndices() and updateLayout()");
    updateColumnIndices();
    updateLayout();
    LOG("Completed updateColumnIndices() and updateLayout()");
    updateRenderWaveName();
    return true;
}

void MultiColumnDisplay::updateColumnIndices()
{
    displayColumn->setIndex(0);
}

void MultiColumnDisplay::updateLayout()
{
    // Remove all items from layout.
    while (layout()->count() > 0) {
        layout()->takeAt(0);
    }

    // Add columns in correct order.
    layout()->addWidget(displayColumn);
}


void MultiColumnDisplay::updateForRescan()
{
    waveformManager->removeAllWaveforms();
    addWaveforms();
    waveformManager->resetAll();
    updateFromState();
}

void MultiColumnDisplay::addWaveforms()
{
    LOG("Beginning of addWaveforms()");
    // Add all waveforms to waveform manager.
    // Headstage port
    QStringList amplifierList = signalManager.getDisplayListAmplifiers();
    for (int j = 0; j < amplifierList.size(); ++j) {
        QString amplifierName = amplifierList.at(j);
        waveformManager->addWaveform(amplifierName); // TODO 需要删除 wide 标识符
    }
    LOG("End of addWaveforms()");
}

void MultiColumnDisplay::updateFromState()
{
    // Reset display if time scale or roll mode has changed.
    if (displayStatus.getPaperSpeedIndex() != tPaperSpeedIndex || displayStatus.isRollMode() != rollModeFormerValue) {
        tPaperSpeedIndex = displayStatus.getPaperSpeedIndex();
        rollModeFormerValue = displayStatus.isRollMode();
        reset();
    }
    displayColumn->updateFromState();
    emit signalUpdateDisplayParam();
}

void MultiColumnDisplay::reset()
{
    waveformManager->setTPaperSpeed();
    waveformManager->resetAll();
    displayColumn->updateNow();
    emit signalUpdateDisplayParam();
}

void MultiColumnDisplay::setShowPinned(const bool showPinned)
{
    displayColumn->setShowPinnedWaveforms(showPinned);
    //displayStatus.forceUpdateSystemState();
}

// 通知server当前显示的通道
void MultiColumnDisplay::updateRenderWaveName()
{
    displayStatus.updateDisplayWaveformNames(displayColumn->getPinnedWaveNames(), displayColumn->getRenderWaveNames());
}

void MultiColumnDisplay::setWaveformWidth(const int width)
{
    waveformManager->setMaxWidthInPixels(width);
    emit signalUpdateDisplayParam();
}

void MultiColumnDisplay::slotUpdateDisplayWaveNames()
{
    LOG("Visible waves changed!!!");
    displayStatus.updateDisplayWaveformNames(displayColumn->getPinnedWaveNames(), displayColumn->getRenderWaveNames());
}

void MultiColumnDisplay::loadWaveformData(UiDisplayDto& uiDisplayDto)
{
    displayColumn->loadWaveformData(uiDisplayDto);
}

void MultiColumnDisplay::enableSelectedChannels(bool enabled)
{
    displayColumn->enableSelectedWaveforms(enabled);
}

END_NX_NAMESPACE
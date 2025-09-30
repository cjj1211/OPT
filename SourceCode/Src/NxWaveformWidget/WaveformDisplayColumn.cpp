#include <iostream>
#include "multicolumndisplay.h"
#include "waveformdisplaycolumn.h"
#include "nxwaveformwidget_global.h"
BEGIN_NX_NAMESPACE

WaveformDisplayColumn::WaveformDisplayColumn(int columnIndex_, WaveformDisplayManager* waveformManager,
	                                             MultiColumnDisplay *parent_) :
    QWidget(parent_),
    parent(parent_),
    columnIndex(columnIndex_)
{
    LOG("Beginning of WaveformDisplayColumn ctor");
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setFocusPolicy(Qt::StrongFocus);

    visible = true;
    pinnedShown = true;

    LOG("About to create MultiWaveformPlot");
    waveformPlot = new MultiWaveformPlot(columnIndex, waveformManager,  this);
    connect(waveformPlot, SIGNAL(signalToggleSelectedWaveforms()), SIGNAL(signalToggleSelectedWaveforms()));
    connect(waveformPlot, SIGNAL(signalUpdateSelectedWavename(QString)), SIGNAL(signalUpdateSelectedWavename(QString)));
    LOG("Created MultiWaveformPlot");

    //lessSpacingButton = new QToolButton(this);
    //lessSpacingButton->setIcon(QIcon(":/Optimus/images/trainPage/lessspacingicon.png"));
    //lessSpacingButton->setToolTip(tr("Less Spacing"));
    //connect(lessSpacingButton, SIGNAL(clicked()), waveformPlot, SLOT(decreaseSpacing()));

    //moreSpacingButton = new QToolButton(this);
    //moreSpacingButton->setIcon(QIcon(":/Optimus/images/trainPage/morespacingicon.png"));
    //moreSpacingButton->setToolTip(tr("More Spacing"));
    //connect(moreSpacingButton, SIGNAL(clicked()), waveformPlot, SLOT(increaseSpacing()));

    //showPinnedCheckBox = new QCheckBox(tr("show pinned"), this);
    //showPinnedCheckBox->setChecked(true);
    //connect(showPinnedCheckBox, SIGNAL(clicked()), this, SLOT(showPinnedWaveformsSlot()));

    //LOG("Created QToolButtons");

    //const auto toolRow = new QHBoxLayout;
    //toolRow->addWidget(showPinnedCheckBox);
    //toolRow->addWidget(lessSpacingButton);
    //toolRow->addWidget(moreSpacingButton);

    LOG("About to create verticalSpacer");
    verticalSpacer = new QSpacerItem(1, 0, QSizePolicy::Fixed, QSizePolicy::Fixed);
    LOG("Created verticalSpacer. About to set main layout");

    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->addWidget(waveformPlot);
    mainLayout->addSpacerItem(verticalSpacer);
    //mainLayout->addLayout(toolRow); Òþ²Ø¹¤¾ßÐÐ
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(1);

    setLayout(mainLayout);

    connect(this, SIGNAL(signalUpdateDisplayParam()), parent, SIGNAL(signalUpdateDisplayParam()));
    connect(this, SIGNAL(signalDispalyWaveNamesChanged()), parent, SLOT(slotUpdateDisplayWaveNames()));
    LOG("Set main layout. End of ctor");
}

WaveformDisplayColumn::~WaveformDisplayColumn()
{
    disconnect(this, nullptr, waveformPlot, nullptr);
    disconnect(this, nullptr, nullptr, nullptr);
    delete waveformPlot;    // Probably not necessary since WaveformPlot is a QWidget, but just to be safe.
}

void WaveformDisplayColumn::setWaveformWidth(int width)
{
    parent->setWaveformWidth(width);
}

void WaveformDisplayColumn::showPinnedWaveformsSlot()
{
    bool show = showPinnedCheckBox->checkState() != Qt::Unchecked;
    waveformPlot->showPinnedWaveforms(show);
    pinnedShown = show;
}

void WaveformDisplayColumn::updateVisibleWaveName() const
{
    parent->updateRenderWaveName();
}

void WaveformDisplayColumn::setShowPinnedWaveforms(bool show)
{
    showPinnedCheckBox->setChecked(show);
    waveformPlot->showPinnedWaveforms(show);
    pinnedShown = show;
}

void WaveformDisplayColumn::updateFromState()
{
    waveformPlot->updateFromState();
}

END_NX_NAMESPACE
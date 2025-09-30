/******************************************************************************
 *  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。                   
 *  作者 : 宋志杰 zhijie.song@neuroxess.com
 *  创建日期:   September 2023
 *  修改记录: 
 *  <修改者姓名><邮件地址>   <修改内容>
******************************************************************************/
#ifndef __UI_CHANNEL_H
#define __UI_CHANNEL_H
#include "nxwaveformwidget_global.h"
#include "CDiscreteList.h"
#include <QString>
#include <mutex>

BEGIN_NX_NAMESPACE

enum UiSignalType {
	AmplifierSignal = 0
};

struct NX_WAVEFORM_WIDGET_EXPORT UiChannel
{
	UiChannel()
		: signalType(UiSignalType::AmplifierSignal)
		, customChannelNumber(0)
		, electrodeImpedance({ false, 0, 0 })
		, enabled(true)
		, nativeChannelNumber(0)
		, selected(false)
		, userOrder(0)
		, heatValue(0)
		, heatValid(false)
	{
		sampleRate = new CDiscreteList();
		sampleRate->addItem("500", "0.5k Hz", 500.0);
		sampleRate->addItem("800", "0.8kHz", 800.0);
		sampleRate->addItem("1000", "1kHz", 1000.0);
		sampleRate->addItem("1250", "1.25kHz", 1250.0);
		sampleRate->addItem("1500", "1.5kHz", 1500.0);
		sampleRate->addItem("2000", "2kHz", 2000.0);
		sampleRate->addItem("2500", "2.5kHz", 2500.0);
		sampleRate->addItem("3000", "3kHz", 3000.0);
		sampleRate->addItem("3333", "3.333kHz", 10000.0 / 3.0);
		sampleRate->addItem("4000", "4kHz", 4000.0);
		sampleRate->addItem("5000", "5kHz", 5000.0);
		sampleRate->addItem("6250", "6.25kHz", 6250.0);
		sampleRate->addItem("8000", "8kHz", 8000.0);
		sampleRate->addItem("10000", "10kHz", 10000.0);
		sampleRate->addItem("12500", "12.5kHz", 12500.0);
		sampleRate->addItem("15000", "15kHz", 15000.0);
		sampleRate->addItem("20000", "20kHz", 20000.0);
		sampleRate->addItem("25000", "2.5kHz", 25000.0);
		sampleRate->addItem("30000", "30kHz", 30000.0);
		sampleRate->setValue("30000");
	}

	void setGroupName(const QString& name) { groupName = name; }

	QString getGroupName() const { return groupName; }

	void setNativeName(const QString name) { nativeChannelName = name; }

	QString getNativeName() const {
		return nativeChannelName;
	}

	void setCustomName(QString name) { customChannelName = name; }

	QString getCustomName() const {
		return customChannelName;
	}

	/**
	 * 获取阻抗值.
	 *
	 * @return
	 */
	QString getImpedanceMagnitudeString() const
	{
		double zMag = electrodeImpedance.magnitude;
		double scale;
		QString unitPrefix;
		if (zMag >= 1.0e6) {
			scale = 1.0e6;
			unitPrefix = "M";
		}
		else {
			scale = 1.0e3;
			unitPrefix = "k";
		}
		int precision;
		if (zMag >= 100.0e6) precision = 0;
		else if (zMag >= 10.0e6) precision = 1;
		else if (zMag >= 1.0e6) precision = 2;
		else if (zMag >= 100.0e3) precision = 0;
		else if (zMag >= 10.0e3) precision = 1;
		else precision = 2;

		return QString::number(zMag / scale, 'f', precision) + unitPrefix + QString((QChar)0x03a9);
	}

	/**
	 * 获取阻抗相位.
	 *
	 * @return 阻抗相位
	 */
	QString getImpedancePhaseString() const
	{
		return (QString((QChar)0x2220) + QString::number(electrodeImpedance.phase, 'f', 0) + QString((QChar)0x00b0));
	}

	double getSampleRate() const { return sampleRate->getNumericValue(); }
	void setSampleRate(double value) const { 
		for (int i = 0; i < sampleRate->numberOfItems(); ++i)
		{
			if (abs(sampleRate->items[i].numericValue - value) < 1.0)
			{
				sampleRate->setIndex(i);
				return;
			}
		}

		LOG_ERR("Set sample rate fail.");
	}

	int getUserOrder() const { return userOrder; }
	void setUserOrder(const int order) { userOrder = order; }

	QString getColor() const { return color; }
	void setColor(const QString _color) { color = _color; }

	bool isSelected() const { return selected; }
	void setIsSelected(bool _selected) { selected = _selected; }

	bool isEnabled() const { return enabled; }
	void setEnabled(const bool _enabled) { enabled = _enabled; }

	UiSignalType getSignalType() const { return signalType; }
	void setSignalType(const UiSignalType type) { signalType = type; }

	bool isImpedanceValided() const { return electrodeImpedance.valid; }
	void setImpednanceValided(const bool valided) { electrodeImpedance.valid = valided; }

	double getImpedanceMagnitude() const { return electrodeImpedance.magnitude; }
	void setImpedanceMagnitude(const double magnitude) { electrodeImpedance.magnitude = magnitude; }

	double getImpedancePhase() const { return electrodeImpedance.phase; }
	void setImpedancePhase(const double phase) { electrodeImpedance.phase = phase; }

	double getHeatValue() {  
		std::lock_guard<std::mutex> lock(heatMutex);
		return heatValue;  
	}
	void setHeatValue(const double heat) { 
		std::lock_guard<std::mutex> lock(heatMutex);
		heatValue = heat; 
	}

	bool getHeatValid() { 
		std::lock_guard<std::mutex> lock(heatMutex);
		return heatValid; 
	}
	void setHeatValid(const bool valid) { 
		std::lock_guard<std::mutex> lock(heatMutex);
		heatValid = valid; 
	}

private:
	CDiscreteList* sampleRate;
	UiSignalType signalType;

	QString color;
	QString groupName;
	QString nativeChannelName;
	QString customChannelName;

	int userOrder;
	int nativeChannelNumber;
	int customChannelNumber;
	bool selected;
	bool enabled;

	struct {
		bool valid;
		double magnitude;
		double phase;
	} electrodeImpedance;

	double heatValue;
	bool heatValid;

	std::mutex heatMutex;
};

END_NX_NAMESPACE
#endif

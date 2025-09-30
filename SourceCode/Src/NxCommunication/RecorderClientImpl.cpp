#include "RecorderClientImpl.h"
#include "NxMessage.grpc.pb.h"
#include <CLogger.h>
#include <format>
#include <ranges>
#include <thread>
#include <QSharedMemory>
#include <QtConcurrent/QtConcurrent>

BEGIN_NX_NAMESPACE

RecorderClient::RecorderClientImpl::RecorderClientImpl(std::shared_ptr<grpc::Channel> channel)
	: stub(Recorder::NewStub(channel))
	, oscillographMode(OS_SIGNAL)
	, isLoading(false)
	, isSpectrogramDataLoading(false)
{
}

RecorderClient::RecorderClientImpl::~RecorderClientImpl()
{
}

void RecorderClient::RecorderClientImpl::setMasterId(const std::string processId) const
{
	StringMsg msg;
	msg.set_value(processId);

	EmptyMsg res;
	ClientContext ctx;

	if (const auto status = stub->setMasterId(&ctx, msg, &res); !status.ok())
	{
		LOG_ERR(std::format("Send msg fail. Error code: {}, \nError msg: {}, \nError details: {}",
			static_cast<int>(status.error_code()), status.error_message(), status.error_details()).c_str());
	}
}

bool RecorderClient::RecorderClientImpl::isControllerConnected() const
{
	const EmptyMsg msg;
	BoolMsg res;
	ClientContext ctx;
	if (const auto status = stub->isControllerConnected(&ctx, msg, &res); !status.ok())
	{
		LOG_ERR(std::format("Send msg fail. Error code: {}, \nError msg: {}, \nError details: {}",
			static_cast<int>(status.error_code()), status.error_message(), status.error_details()).c_str());
		return false;
	}
	return res.res();
}

bool RecorderClient::RecorderClientImpl::isServerAvailable() 
{
	const EmptyMsg msg;
	BoolMsg res;
	ClientContext ctx;
	QElapsedTimer timer;
	timer.start();
	if (const auto status = stub->connectTest(&ctx, msg, &res); !status.ok())
	{
		LOG_ERR(std::format("Send msg fail. Error code: {}, \nError msg: {}, \nError details: {}",
			static_cast<int>(status.error_code()), status.error_message(), status.error_details()).c_str());
		if (status.error_code() == grpc::StatusCode::UNAVAILABLE)
		{
			qDebug() << "isServerAvailable false, status: UNAVAILABLE" ;
			watcherTimeoutCount = 0;
			return false;
		}
		else if(status.error_code() == grpc::StatusCode::DEADLINE_EXCEEDED)
		{
			qDebug() << "isServerAvailable false, Timeout use time:" << timer.elapsed() << "ms";
			watcherTimeoutCount++;
			if(watcherTimeoutCount >= 5)
			{
				watcherTimeoutCount = 0;
				return false;
			}
			else
			{
				return true;
			}
		}
		else
		{
			qDebug() << std::format("IsServerAvailable:Send msg fail. Error code: {}, \nError msg: {}, \nError details: {}",
				static_cast<int>(status.error_code()), status.error_message(), status.error_details()).c_str();
		}

		watcherTimeoutCount = 0;
		return true;
	}
	return res.res();
}

bool RecorderClient::RecorderClientImpl::initBoard(const InitInfo& initInfo, QMap<QString, QString>& channelInfos) const
{
	InitBoardMsg msg;
	msg.set_samplerate(initInfo.SampleRate);
	msg.set_acquisitionmode(initInfo.AcquisitionMode);
	msg.set_savedatatype(initInfo.SaveFileType.toStdString());
	msg.set_rhdfilepath(initInfo.SaveFilePath.toStdString());

	msg.set_usegpu(initInfo.UseGpu);
	msg.set_compressdata(initInfo.CompressData);

	SignalsMsg res;
	ClientContext ctx;
	if (const auto status = stub->initBoard(&ctx, msg, &res); !status.ok())
	{
		LOG_ERR(std::format("Send msg fail. Error code: {}, \nError msg: {}, \nError details: {}",
			static_cast<int>(status.error_code()), status.error_message(), status.error_details()).c_str());
		return false;
	}

	channelInfos.clear();
	for (auto infoRes : res.channelmap())
	{
		channelInfos.insert(QString::fromStdString(infoRes.second.nativechannelname()), QString::fromStdString(infoRes.second.color()));
	}

	return true;
}

void RecorderClient::RecorderClientImpl::resetSampleRate(const int sampleRate) const
{
    IntMsg msg;
    msg.set_value(sampleRate);

    EmptyMsg res;
    ClientContext ctx;
    if (const auto status = stub->resetSampleRate(&ctx, msg, &res); !status.ok())
    {
        LOG_ERR(std::format("Reset sample rate fail. Error code: {}, \nError msg: {}, \nError details: {}",
				              static_cast<int>(status.error_code()), status.error_message(), status.error_details()).c_str());

    }
}

void RecorderClient::RecorderClientImpl::closeController() const
{
	const EmptyMsg msg;
	EmptyMsg res;
	ClientContext ctx;
	if(const auto status = stub->closeController(&ctx, msg, &res); !status.ok())
	{
	    LOG_ERR(std::format("Send msg fail. Error code: {0}, \nError msg: {1}, \nError details: {2}",
				              static_cast<int>(status.error_code()), status.error_message(), status.error_details()).c_str());
    }
}

void RecorderClient::RecorderClientImpl::setStoragePath( const QString& rhdFilePath) const
{
	StringMsg msg;
	msg.set_value(rhdFilePath.toStdString());

	EmptyMsg res;
	ClientContext ctx;
	if (const auto status = stub->setStoragePath(&ctx, msg, &res); !status.ok())
	{
		LOG_ERR(std::format("Send msg fail. Error code: {}, \nError msg: {}, \nError details: {}",
			static_cast<int>(status.error_code()), status.error_message(), status.error_details()).c_str());
	}
}

void RecorderClient::RecorderClientImpl::setImpedanceFilePath(const QString& impedanceFilePath) const
{
	StringMsg msg;
	msg.set_value(impedanceFilePath.toStdString());

	EmptyMsg res;
	ClientContext ctx;
	if (const auto status = stub->setImpedanceFileName(&ctx, msg, &res); !status.ok())
	{
		LOG_ERR(std::format("Send msg fail. Error code: {}, \nError msg: {}, \nError details: {}",
			static_cast<int>(status.error_code()), status.error_message(), status.error_details()).c_str());
	}
}

std::string RecorderClient::RecorderClientImpl::getPID() const
{
	EmptyMsg msg;
	StringMsg res;

	ClientContext ctx;
	if (const auto status = stub->getPID(&ctx, msg, &res); !status.ok())
	{
		LOG_ERR(std::format("Send msg fail. Error code: {}, \nError msg: {}, \nError details: {}",
			static_cast<int>(status.error_code()), status.error_message(), status.error_details()).c_str());
		return "";
	}

	return res.value();
}

std::string RecorderClient::RecorderClientImpl::getVID() const
{
	EmptyMsg msg;
	StringMsg res;

	ClientContext ctx;
	if (const auto status = stub->getVID(&ctx, msg, &res); !status.ok())
	{
		LOG_ERR(std::format("Send msg fail. Error code: {}, \nError msg: {}, \nError details: {}",
			static_cast<int>(status.error_code()), status.error_message(), status.error_details()).c_str());
		return "";
	}

	return res.value();
}

std::string RecorderClient::RecorderClientImpl::getDeviceID() const
{
	EmptyMsg msg;
	StringMsg res;

	ClientContext ctx;
	if (const auto status = stub->getDeviceID(&ctx, msg, &res); !status.ok())
	{
		LOG_ERR(std::format("Send msg fail. Error code: {}, \nError msg: {}, \nError details: {}",
			static_cast<int>(status.error_code()), status.error_message(), status.error_details()).c_str());
		return "";
	}

	return res.value();
}

void RecorderClient::RecorderClientImpl::stopLoadWaveformStream()
{
	LOG_INFO("stopLoadWaveformStream........................START");
	isLoading = false;
	isSpectrogramDataLoading = false;

	waveformDataReaderThread.waitForFinished();
	spectrogramDataReaderThread.waitForFinished();
	LOG_INFO("stopLoadWaveformStream........................FINISH");
}

void RecorderClient::RecorderClientImpl::changeOsMode(const OscillographMode& mode)
{
	oscillographMode = mode;
	OsModeMsg msg;
	msg.set_mode(mode);
	ClientContext ctx;
	EmptyMsg res;
	stub->setOscillographMode(&ctx, msg, &res);
}

void RecorderClient::RecorderClientImpl::setSelectedWaveName(const QString& nativeWaveName) const
{
	EmptyMsg res;
	StringMsg msg;
	msg.set_value(nativeWaveName.toStdString());
	ClientContext ctx;
	if (const auto status = stub->setSelectedWaveName(&ctx, msg, &res); !status.ok())
	{
		LOG_ERR(std::format("Send msg fail. Error code: {}, \nError msg: {}, \nError details: {}",
			static_cast<int>(status.error_code()), status.error_message(), status.error_details()));
	}
}

void RecorderClient::RecorderClientImpl::loadSelectedAmpData()
{
	LOG_INFO("loadWaveformData.........................start");
	isSpectrogramDataLoading = true;
	spectrogramDataReaderThread = QtConcurrent::run([&]() {
		ClientContext ctx;
        const EmptyMsg msg;
        const auto stream(stub->loadSpectrogramData(&ctx, msg));

		SpectrogramSegmentMsg recvData;
		while (isSpectrogramDataLoading)
		{
			//if (oscillographMode != OS_SPECTROGRAM)
			//{
			//	QThread::msleep(30);
			//	continue;
			//}

			while (stream->Read(&recvData))
			{
				//if (oscillographMode != OS_SPECTROGRAM)
				//{
				//	break;
				//}

				if (loadSelectedAmpDataCallback)
				{
					int sampleCnt = recvData.samplenum();

					QVector<float> samples;
					samples.resize(sampleCnt);
					memcpy(samples.data(), recvData.samplesdata().data(), sizeof(float) * sampleCnt);

					QVector<uint64_t> timestamps;
					timestamps.resize(sampleCnt);
					memcpy(timestamps.data(), recvData.timestamps().data(), sizeof(uint64_t) * sampleCnt);

					loadSelectedAmpDataCallback(QString::fromStdString(recvData.wavename()), sampleCnt, samples, timestamps);
				}
				else
				{
					QThread::msleep(30);
				}
			}
		}
		auto status = stream->Finish();
		LOG_INFO("spectrogramDataReaderThread DONE.");
		});
}

void RecorderClient::RecorderClientImpl::loadWaveformData()
{
	LOG_INFO("loadWaveformData.........................start");
	
	isLoading = true;
	auto thread_func = [&]() { 
		LOG("Writer thread in.");
		QThread::msleep(500);
		while (isLoading)
		{
			if (oscillographMode != OS_SIGNAL)
			{
				QThread::msleep(30);
				continue;
			}

			if (getVisibleWaveNamesFunc)
			{
				StringListMsg msg;
				ClientContext ctx;
				auto names = getVisibleWaveNamesFunc();
				for (auto& name : names)
				{
					msg.add_values(name.toStdString());
				}
				
				WaveformMsg recvData;
				Status status = stub->loadWaveformData(&ctx, msg, &recvData);
				if (!status.ok())
				{
					LOG_WARN("getData rpc failed");
					QThread::msleep(30);
				    continue;
				}

				if (0 == recvData.endtimestamp() && recvData.isrunning())
				{
					QThread::msleep(30);
					continue;
				}

				QVector<QMap<std::string, std::string>> data;
				if (recvData.isuseverticallines())
				{
					QMap<std::string, std::string> yMin, yMax;
					for (auto& item : recvData.ymin())
					{
						yMin.insert(item.first, item.second);
					}

					for (auto& item : recvData.ymax())
					{
						yMax.insert(item.first, item.second);
					}
					data.append(yMin);
					data.append(yMax);
				}
				else
				{
					QMap<std::string, std::string> yData;
					for (auto& item : recvData.ydata())
					{
						yData.insert(item.first, item.second);
					}
					data.append(yData);
				}
				loadwaveformCallback(recvData.issweepfirsttime(),
					recvData.isuseverticallines(),
					recvData.validzoneindex(),
					recvData.starttimestamp(),
					recvData.endtimestamp(), 
					recvData.isrunning(),
					data);
			}
			QThread::msleep(90); // 10fps
		}
	};

	waveformDataReaderThread = QtConcurrent::run(thread_func);
}

void RecorderClient::RecorderClientImpl::asyncSendSetCommand(const QString& parameter, const QString& value) const
{
	QtConcurrent::run([&](const QString& param, const QString& val)
		{
			SetCommandMsg msg;
			msg.set_parameter(param.toStdString());
			msg.set_value(val.toStdString());

			EmptyMsg res;
			ClientContext ctx;
			stub->sendSetCommand(&ctx, msg, &res);
		}, parameter, value);
}

void RecorderClient::RecorderClientImpl::syncSendSetCommand(const QString& parameter, const QString& value) const
{
	SetCommandMsg msg;
	msg.set_parameter(parameter.toStdString());
	msg.set_value(value.toStdString());

	EmptyMsg res;
	ClientContext ctx;
	stub->sendSetCommand(&ctx, msg, &res);
}

void RecorderClient::RecorderClientImpl::asyncSendExecuteCommand(const QString& action) const
{
	QtConcurrent::run([&](const QString act)
		{
			ExecuteCommandMsg msg;
			msg.set_action(act.toStdString());

			EmptyMsg res;
			ClientContext ctx;
			stub->sendExecuteCommand(&ctx, msg, &res);
		}, action);
}

void RecorderClient::RecorderClientImpl::syncSendExecuteCommand(const QString& action) const
{
	ExecuteCommandMsg msg;
	msg.set_action(action.toStdString());

	EmptyMsg res;
	ClientContext ctx;
	stub->sendExecuteCommand(&ctx, msg, &res);
}

std::vector<std::string> RecorderClient::RecorderClientImpl::getDisplayListAmplifiers(
	const std::string& groupName) const
{
	StringMsg msg;
	msg.set_value(groupName);
	ClientContext ctx;

	StringListMsg res;
	if(const auto status = stub->getDisplayListAmplifiers(&ctx, msg, &res); !status.ok())
	{
		LOG_ERR(std::format("Send msg fail. Error code: {0}, \nError msg: {1}, \nError details: {2}",
			static_cast<int>(status.error_code()), status.error_message(), status.error_details()).c_str());
	}

	std::vector<std::string> result;
	for(const auto& val: res.values())
	{
		result.emplace_back(val);
	}
	return result;
}

void RecorderClient::RecorderClientImpl::getImpedanceInfos(const QStringList& enabledPorts, QVector<ImpedanceInfo>& impedanceInfos) const
{
	StringListMsg msg;
	for (auto& portName : enabledPorts)
	{
		msg.add_values(portName.toStdString());
	}

	ImpedanceMsg res;
	ClientContext ctx;
	if (const auto status = stub->getImpedanceInfo(&ctx, msg, &res); !status.ok())
	{
		LOG_ERR(std::format("Send msg fail. Error code: {0}, \nError msg: {1}, \nError details: {2}",
			static_cast<int>(status.error_code()), status.error_message(), status.error_details()).c_str());
	}

	for(const auto& item : res.impedance())
	{
		qDebug() << "Measure Impedance: ChannelName " << QString::fromStdString(item.channnelname()) << " Magnitude: " << item.magnitude();
		impedanceInfos.append(ImpedanceInfo{ item.magnitude(), item.phase(), item.valid(), item.channnelname() });
	}

}

void RecorderClient::RecorderClientImpl::getImpedanceMeasureStatus(bool& isCompleted, int& progress)
{
	const EmptyMsg msg;
	ProgressStatus res;
	ClientContext ctx;
	if (const auto status = stub->getMeasureImpedanceStatus(&ctx, msg, &res); !status.ok())
	{
		LOG_ERR(std::format("Send msg fail. Error code: {0}, \nError msg: {1}, \nError details: {2}",
			static_cast<int>(status.error_code()), status.error_message(), status.error_details()).c_str());
		return;
	}

	isCompleted = res.iscompleted();
	progress = res.progress();
}

void RecorderClient::RecorderClientImpl::getHeatScales(QMap<QString, float>& heatScales) const
{
	const EmptyMsg msg;
	HeatScaleResponse res;
	ClientContext ctx;
	if (const auto status = stub->getHeatScales(&ctx, msg, &res); !status.ok())
	{
		LOG_ERR(std::format("Send msg fail. Error code: {0}, \nError msg: {1}, \nError details: {2}",
			static_cast<int>(status.error_code()), status.error_message(), status.error_details()).c_str());
		return;
	}

	for (auto& item : res.heatscalepair())
	{
		heatScales[QString::fromStdString(item.wavename())] = static_cast<float>(item.heatscale());
	}
}

void RecorderClient::RecorderClientImpl::enableMeasureHeatScale(const bool enable) const
{
	BoolMsg msg;
	msg.set_res(enable);
	EmptyMsg res;

	ClientContext ctx;
	if (const auto status = stub->enableMeasureHeatScale(&ctx, msg, &res); !status.ok())
	{
		LOG_ERR(std::format("Send msg fail. Error code: {0}, \nError msg: {1}, \nError details: {2}",
			static_cast<int>(status.error_code()), status.error_message(), status.error_details()).c_str());
	}
}

void RecorderClient::RecorderClientImpl::setHeatScaleChannels(const QStringList& channelNames) const
{
	HeatScaleChannelNamesMsg msg;
	for (const auto& item : channelNames)
	{
		msg.add_wavenames(item.toStdString());
	}

	EmptyMsg res;

	ClientContext ctx;
	if (const auto status = stub->setHeatScaleChannels(&ctx, msg, &res); !status.ok())
	{
		LOG_ERR(std::format("Send msg fail. Error code: {0}, \nError msg: {1}, \nError details: {2}",
			static_cast<int>(status.error_code()), status.error_message(), status.error_details()).c_str());
	}
}

void RecorderClient::RecorderClientImpl::getBufferStatus(double& hdBufferPercent, double& swBufferPercent,
                                                         double& cpuLoadPercent) const
{
	const EmptyMsg msg;
	BufferStatusResponse res;
	ClientContext ctx;

	if (const auto status = stub->getBufferStatus(&ctx, msg, &res); !status.ok())
	{
		LOG_ERR(std::format("Send msg fail. Error code: {0}, \nError msg: {1}, \nError details: {2}",
			static_cast<int>(status.error_code()), status.error_message(), status.error_details()).c_str());
		return;
	}
	hdBufferPercent = res.hardwarebufferpercent();
	swBufferPercent = res.softwarebufferpercent();
	cpuLoadPercent = res.cpuloadpercent();
}

void RecorderClient::RecorderClientImpl::updateFilterParams(const QString& portName, 
	const bool enableLff, 
	const bool enableHff, 
	const bool enableDSP, 
	const double dspCutoff, 
	const double hwLowCut, 
	const double hwHighCut,
	const double lowBandwidth, 
	const double highBandwidth, 
	const std::string& notchFilter, 
	const double Order, 
	const std::string& Filter, 
	const int32_t sampleRate, 
	const int multiNotchCount) const
{
	FilterParamsMsg msg;
	msg.set_portname(portName.toStdString());
	msg.set_lowerbandwidth(lowBandwidth * 0.992);
	msg.set_higherbandwidth(highBandwidth * 1.008);
	msg.set_hwlowcut(hwLowCut);
	msg.set_hwhighcut(hwHighCut);
	msg.set_enablelff(enableLff);
	msg.set_enablehff(enableHff);
	msg.set_enabledsp(enableDSP);
	msg.set_dspcutoff(dspCutoff);
	msg.set_notchfilter(notchFilter);
	msg.set_order(Order);
	msg.set_filter(Filter);
	msg.set_samplerate(sampleRate);
	msg.set_multinotchcnt(multiNotchCount);

	EmptyMsg res;
	ClientContext ctx;
	if (const auto status = stub->updateFilterParams(&ctx, msg, &res); !status.ok())
	{
		LOG_ERR(std::format("Send msg fail. Error code: {0}, \nError msg: {1}, \nError details: {2}",
			static_cast<int>(status.error_code()), status.error_message(), status.error_details()).c_str());
	}
	else
	{
		LOG_INFO(std::format("Update filter params: low {},high {},notch {}", lowBandwidth, highBandwidth, notchFilter).c_str());
	}
}

void RecorderClient::RecorderClientImpl::updateDisplayParam(const double timespan, const double dispWidth, const double refreshCount, const bool isRoll) const
{
	DiplayParamMsg msg;
	msg.set_timespanperfullrefresh(timespan);
	msg.set_displaywidthinpixel(dispWidth);
	msg.set_refreshzonecnt(refreshCount);
	msg.set_isrollmode(isRoll);

	EmptyMsg res;
	ClientContext ctx;

	if (const auto status = stub->updateDisplayParam(&ctx, msg, &res); !status.ok())
	{
		LOG_ERR(std::format("Send msg fail. Error code: {0}, \nError msg: {1}, \nError details: {2}",
			static_cast<int>(status.error_code()), status.error_message(), status.error_details()).c_str());
	}
}

void RecorderClient::RecorderClientImpl::jumpToPosition(const uint64_t targetTime) const
{
	TargetTimeMsg msg;
	msg.set_targettime(targetTime);

	EmptyMsg res;
	ClientContext ctx;
	if (const auto status = stub->jumpToPosition(&ctx, msg, &res); !status.ok())
	{
		LOG_ERR(std::format("Send msg fail. Error code: {0}, \nError msg: {1}, \nError details: {2}",
			static_cast<int>(status.error_code()), status.error_message(), status.error_details()).c_str());
	}
}

void RecorderClient::RecorderClientImpl::startRecord() const
{
	const EmptyMsg msg;
	EmptyMsg res;
	ClientContext ctx;
	if (const auto status = stub->startRecord(&ctx, msg, &res); !status.ok())
	{
		LOG_ERR(std::format("Send msg fail. Error code: {0}, \nError msg: {1}, \nError details: {2}",
			static_cast<int>(status.error_code()), status.error_message(), status.error_details()).c_str());
	}
}

void RecorderClient::RecorderClientImpl::restartRecord(const int timestampOffset) const
{
	IntMsg msg;
	msg.set_value(timestampOffset);
	qDebug() << "Timestamp offset:" << timestampOffset;
	EmptyMsg res;
	ClientContext ctx;
	if (const auto status = stub->restartRecord(&ctx, msg, &res); !status.ok())
	{
		LOG_ERR(std::format("Send msg fail. Error code: {0}, \nError msg: {1}, \nError details: {2}",
			static_cast<int>(status.error_code()), status.error_message(), status.error_details()).c_str());
	}
}

void RecorderClient::RecorderClientImpl::stopRecord() const
{
	const EmptyMsg msg;
	EmptyMsg res;
	ClientContext ctx;
	if (const auto status = stub->stopRecord(&ctx, msg, &res); !status.ok())
	{
		LOG_ERR(std::format("Send msg fail. Error code: {0}, \nError msg: {1}, \nError details: {2}",
			static_cast<int>(status.error_code()), status.error_message(), status.error_details()).c_str());
	}
}

void RecorderClient::RecorderClientImpl::setNotch(const QString& portName, const NotchType type)
{
	NotchMsg msg;
	msg.set_portname(portName.toStdString());
	switch (type)
	{
	case NT_NONE:
		msg.set_notchtype(NotchMsg_NotchType_NT_NONE);
		break;
	case NT_50Hz:
		msg.set_notchtype(NotchMsg_NotchType_NT_50HZ);
		break;
	case NT_60Hz:
		msg.set_notchtype(NotchMsg_NotchType_NT_60HZ);
		break;
	default:
		break;
	}
	EmptyMsg res;
	ClientContext ctx;
	if (const auto status = stub->setNotch(&ctx, msg, &res); !status.ok())
	{
		LOG_ERR(std::format("Send msg fail. Error code: {0}, \nError msg: {1}, \nError details: {2}",
			static_cast<int>(status.error_code()), status.error_message(), status.error_details()).c_str());
	}
}

void RecorderClient::RecorderClientImpl::setLowFilter(const QString& portName, const double filterValue, const int order, const FilterType type)
{
	FilterMsg msg;
	msg.set_portname(portName.toStdString());
	msg.set_value(filterValue);
	msg.set_order(order);
	switch (type)
	{
	case Bessel:
		msg.set_type("bessel");
		break;
	case Butterworth:
		msg.set_type("butterworth");
		break;
	default:
		msg.set_type("bessel");
		break;
	}
	EmptyMsg res;
	ClientContext ctx;

	if (const auto status = stub->setLowFilter(&ctx, msg, &res); !status.ok())
	{
		LOG_ERR(std::format("Send msg fail. Error code: {0}, \nError msg: {1}, \nError details: {2}",
			static_cast<int>(status.error_code()), status.error_message(), status.error_details()).c_str());
	}
}

void RecorderClient::RecorderClientImpl::setHighFilter(const QString& portName, const double filterValue, const int order, const FilterType type)
{
	FilterMsg msg;
	msg.set_portname(portName.toStdString());
	msg.set_value(filterValue);
	msg.set_order(order);
	switch (type)
	{
	case Bessel:
		msg.set_type("bessel");
		break;
	case Butterworth:
		msg.set_type("butterworth");
		break;
	default:
		msg.set_type("bessel");
		break;
	}
	EmptyMsg res;
	ClientContext ctx;

	if (const auto status = stub->setHighFilter(&ctx, msg, &res); !status.ok())
	{
		LOG_ERR(std::format("Send msg fail. Error code: {0}, \nError msg: {1}, \nError details: {2}",
			static_cast<int>(status.error_code()), status.error_message(), status.error_details()).c_str());
	}
}

void RecorderClient::RecorderClientImpl::setSignalInfo(WaveFormType info)
{
	IntMsg msg;
	msg.set_value(info);
	qDebug() << "Timestamp offset:" << QString::number(info);
	EmptyMsg res;
	ClientContext ctx;
	if (const auto status = stub->setSignalInfo(&ctx, msg, &res); !status.ok())
	{
		LOG_ERR(std::format("Send msg fail. Error code: {0}, \nError msg: {1}, \nError details: {2}",
			static_cast<int>(status.error_code()), status.error_message(), status.error_details()).c_str());
	}

}


void RecorderClient::RecorderClientImpl::setSignalType(std::vector<int>signalType)
{
	SignalsMsgRequest msg;
	for (int value : signalType) {
		msg.add_numbers(value);
	}
	EmptyMsg res;
	ClientContext ctx;
	if (const auto status = stub->setSignalType(&ctx, msg, &res); !status.ok())
	{
		LOG_ERR(std::format("Send msg fail. Error code: {0}, \nError msg: {1}, \nError details: {2}",
			static_cast<int>(status.error_code()), status.error_message(), status.error_details()).c_str());
	}


}

void RecorderClient::RecorderClientImpl::setTimeScale(int time)
{
	IntMsg msg;
	msg.set_value(time);
	EmptyMsg res;
	ClientContext ctx;
	if (const auto status = stub->setTimeScale(&ctx, msg, &res); !status.ok())
	{
		LOG_ERR(std::format("Send msg fail. Error code: {0}, \nError msg: {1}, \nError details: {2}",
			static_cast<int>(status.error_code()), status.error_message(), status.error_details()).c_str());
	}

}

void RecorderClient::RecorderClientImpl::setRestore(QVector<QString> restorePath)
{

	RestoreMsg request;
	for (int i = 0; i < restorePath.size(); i++)
	{
		request.add_restorepath(restorePath.at(i).toStdString());

	}
	EmptyMsg response;
	ClientContext context;
	if (const auto status = stub->startRestore(&context, request, &response); !status.ok())
	{
		LOG_INFO(std::format("Send msg fail. Error code: {}, \nError msg: {}, \nError details: {}",
			static_cast<int>(status.error_code()), status.error_message(), status.error_details()));
	}
}

bool RecorderClient::RecorderClientImpl::exportData(const QString& sourcePath_, const QString& destFilePath_,
    const bool isSrcCompressed_, const QString& exportFileType_) const
{
	ExportDataMsg request;
	request.set_sourcefilepath(sourcePath_.toStdString());
	request.set_destfilepath(destFilePath_.toStdString());
	request.set_issrccompressed(isSrcCompressed_);

	if(exportFileType_.toLower() == "rhd")
	{
		request.set_exportfiletype(Rpc_Nx_Rhd);
	}
	else if(exportFileType_.toLower() == "edf")
	{
		request.set_exportfiletype(Rpc_NX_Edf);
	}
	else 
	{
		return false;
	}

	BoolMsg response;
	ClientContext context;
	if (const auto status = stub->exportData(&context, request, &response); !status.ok())
	{
		LOG_INFO(std::format("Send msg fail. Error code: {}, \nError msg: {}, \nError details: {}",
			static_cast<int>(status.error_code()), status.error_message(), status.error_details()));
	}

	return response.res();
}

void RecorderClient::RecorderClientImpl::stopExport() const
{
    const EmptyMsg request;
	EmptyMsg response;
	ClientContext context;
	if (const auto status = stub->stopExport(&context, request, &response); !status.ok())
	{
		LOG_INFO(std::format("Send msg fail. Error code: {}, \nError msg: {}, \nError details: {}",
			static_cast<int>(status.error_code()), status.error_message(), status.error_details()));
	}
}

bool RecorderClient::RecorderClientImpl::isExportRunning() const
{
	const EmptyMsg request;
	BoolMsg response;
	ClientContext context;
	if (const auto status = stub->isExportRunning(&context, request, &response); !status.ok())
	{
		LOG_INFO(std::format("Send msg fail. Error code: {}, \nError msg: {}, \nError details: {}",
			static_cast<int>(status.error_code()), status.error_message(), status.error_details()));
		return false;
	}
	return response.res();
}

int RecorderClient::RecorderClientImpl::getExportProgress() const
{
	const EmptyMsg request;
	IntMsg response;
	ClientContext context;
	if (const auto status = stub->getExportProgress(&context, request, &response); !status.ok())
	{
		LOG_INFO(std::format("Send msg fail. Error code: {}, \nError msg: {}, \nError details: {}",
			static_cast<int>(status.error_code()), status.error_message(), status.error_details()));
		return 0;
	}
	return response.value();
}

END_NX_NAMESPACE

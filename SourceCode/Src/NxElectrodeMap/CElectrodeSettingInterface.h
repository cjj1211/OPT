#pragma once
#include <string>
#include <vector>
#include "nxelectrodemap_global.h"
#include "CElectrodeMapDataStructure.h"
class QXmlStreamReader;
class QXmlStreamWriter;
BEGIN_NX_NAMESPACE
class NXELECTRODEMAP_EXPORT CElectrodeSettingInterface
{
public:
	explicit CElectrodeSettingInterface(ElectrodeMapSettings* setting);
    bool loadFile(const QString& filename, QString& errorMessage);
    bool parseByteArray(const QByteArray& byteArray, QString& errorMessage);
private:
    bool parseDocumentStart(const QByteArray& byteArray, QString& errorMessage);
    bool parseProbeMapSettingsDom(const QByteArray& byteArray, QString& errorMessage) const;
    void clearProbeMapSettings() const;
    ElectrodeMapSettings* electrodeMapSetting;
};
END_NX_NAMESPACE

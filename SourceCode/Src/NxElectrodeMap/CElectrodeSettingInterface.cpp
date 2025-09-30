#include "CElectrodeSettingInterface.h"
#include "CElectrodeMapDataStructure.h"
#include <QDebug>
#include <QFile>
#include <QDomElement>
#include <qxmlstream.h>
BEGIN_NX_NAMESPACE
CElectrodeSettingInterface::CElectrodeSettingInterface(ElectrodeMapSettings* setting)
	:electrodeMapSetting(setting)
{
}
bool CElectrodeSettingInterface::loadFile(const QString& filename, QString& errorMessage)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        errorMessage = "Unable to open input file";
        return false;
    }
    QByteArray byteArray = file.readAll();
    file.close();
    if (!parseByteArray(byteArray, errorMessage)) {
        qDebug() << "Failure parsing XML data. Error message: " << errorMessage;
        return false;
    }
    return true;
}
bool CElectrodeSettingInterface::parseByteArray(const QByteArray& byteArray, QString& errorMessage)
{
    if (!parseDocumentStart(byteArray, errorMessage)) {
        return false;
    }
    if (!parseProbeMapSettingsDom(byteArray, errorMessage)) {
        return false;
    }
    return true;
}
bool CElectrodeSettingInterface::parseDocumentStart(const QByteArray& byteArray, QString& errorMessage)
{
    QXmlStreamReader stream(byteArray);
    // Read document start
    QXmlStreamReader::TokenType token = stream.readNext();
    if (token == QXmlStreamReader::StartDocument) {
        token = stream.readNext();
        if (token == QXmlStreamReader::StartElement) {
            if (stream.name() == "Optimus") {
                // Iterate through all XML attributes.
                QXmlStreamAttributes attributes = stream.attributes();
                for (auto attribute : attributes) {
                    // Get the attribute name and value from the XML
                    QString attributeName = attribute.name().toString();
                    QString attributeValue = attribute.value().toString();
                    auto a = 0;
                }
                return true;
            }
            else {
               /* if (systemConfig.getCommonConfig().language == "zh-CN")
                {
                    errorMessage.append(QString::fromLocal8Bit("错误:XML文档没有以MindExplorer元素开始"));
                }
                else {
                    errorMessage.append("Error: XML document doesn't start with an MindExplorer element");
                }*/
                return false;
            }
        }
        else {
            /*if (systemConfig.getCommonConfig().language == "zh-CN")
            {
                errorMessage.append(QString::fromLocal8Bit("错误:XML文档以无效元素开始"));
            }
            else {
                errorMessage.append("Error: XML document starts with an invalid element");
            }*/
            return false;
        }
    }
    else {
       /* if (systemConfig.getCommonConfig().language == "zh-CN")
        {
            errorMessage.append(QString::fromLocal8Bit("错误:XML文档以无效的标记开始"));
        }
        else {
            errorMessage.append("Error: XML document starts with an invalid token");
        }*/
        return false;
    }
}
bool CElectrodeSettingInterface::parseProbeMapSettingsDom(const QByteArray& byteArray, QString& errorMessage) const
{
    QDomDocument doc("XMLSettings");
    if (!doc.setContent(byteArray)) {
        return false;
    }
    QDomElement docElem = doc.documentElement().firstChildElement("ProbeMapSettings");
    if (docElem.isNull()) {
        return true;
    }
	clearProbeMapSettings();
    electrodeMapSetting->backgroundColor = docElem.attribute("backgroundColor", "Black");
    electrodeMapSetting->siteOutlineColor = docElem.attribute("siteOutlineColor", "Gray");
    electrodeMapSetting->lineColor = docElem.attribute("lineColor", "White");
    electrodeMapSetting->fontHeight = docElem.attribute("fontHeight", "10").toFloat();
    electrodeMapSetting->fontColor = docElem.attribute("fontColor", "White");
    electrodeMapSetting->textAlignment = docElem.attribute("textAlignment", "BottomLeft");
    electrodeMapSetting->siteShape = docElem.attribute("siteShape", "Rectangle");
    electrodeMapSetting->siteWidth = docElem.attribute("siteWidth", "5").toFloat();
    electrodeMapSetting->siteHeight = docElem.attribute("siteHeight", "5").toFloat();
    electrodeMapSetting->row = docElem.attribute("row").toInt();
    electrodeMapSetting->colum = docElem.attribute("colum").toInt();
    electrodeMapSetting->buttonsize = docElem.attribute("buttonsize").toInt();
    electrodeMapSetting->positionx = docElem.attribute("positionx").toInt();
    electrodeMapSetting->positiony = docElem.attribute("positiony").toInt();
    electrodeMapSetting->space = docElem.attribute("space").toInt();

    QDomNodeList pagesList = docElem.elementsByTagName("Page");
    for (int page = 0; page < pagesList.length(); ++page) {
        Page currentPage;
        currentPage.name = pagesList.at(page).toElement().attribute("name", QString::number(page + 1));
        currentPage.backgroundColor = pagesList.at(page).toElement().attribute("backgroundColor", "");
        currentPage.siteOutlineColor = pagesList.at(page).toElement().attribute("siteOutlineColor", "");
        currentPage.lineColor = pagesList.at(page).toElement().attribute("lineColor", "");
        currentPage.fontHeight = pagesList.at(page).toElement().attribute("fontHeight", "").toFloat();
        currentPage.fontColor = pagesList.at(page).toElement().attribute("fontColor", "");
        currentPage.textAlignment = pagesList.at(page).toElement().attribute("textAlignment", "");
        currentPage.siteShape = pagesList.at(page).toElement().attribute("siteShape", "");
        currentPage.siteWidth = pagesList.at(page).toElement().attribute("siteWidth", "").toFloat();
        currentPage.siteHeight = pagesList.at(page).toElement().attribute("siteHeight", "").toFloat();
        QDomNodeList portsList = pagesList.at(page).toElement().elementsByTagName("Port");
        for (int port = 0; port < portsList.length(); ++port) {
            Group currentPort;
            currentPort.name = portsList.at(port).toElement().attribute("name", "");
            currentPort.siteOutlineColor = portsList.at(port).toElement().attribute("siteOutlineColor", "");
            currentPort.siteShape = portsList.at(port).toElement().attribute("siteShape", "");
            currentPort.siteWidth = portsList.at(port).toElement().attribute("siteWidth", "").toFloat();
            currentPort.siteHeight = portsList.at(port).toElement().attribute("siteHeight", "").toFloat();
            QDomNodeList electrodeSiteList = portsList.at(port).toElement().elementsByTagName("ElectrodeSite");
            for (int electrodeSite = 0; electrodeSite < electrodeSiteList.length(); electrodeSite++) {
                ElectrodeSite currentElectrodeSite;
                currentElectrodeSite.channelNumber = electrodeSiteList.at(electrodeSite).toElement().attribute("channelNumber", "").toStdString();
                currentElectrodeSite.x = electrodeSiteList.at(electrodeSite).toElement().attribute("x", "0").toFloat();
                currentElectrodeSite.y = electrodeSiteList.at(electrodeSite).toElement().attribute("y", "0").toFloat();
                currentElectrodeSite.customName = currentPort.name + QString("-%1").arg(QString::fromStdString(currentElectrodeSite.channelNumber));
                currentElectrodeSite.siteOutlineColor = electrodeSiteList.at(electrodeSite).toElement().attribute("siteOutlineColor", "");
                currentElectrodeSite.siteShape = electrodeSiteList.at(electrodeSite).toElement().attribute("siteShape", "");
                currentElectrodeSite.siteWidth = electrodeSiteList.at(electrodeSite).toElement().attribute("siteWidth", "").toFloat();
                currentElectrodeSite.siteHeight = electrodeSiteList.at(electrodeSite).toElement().attribute("siteHeight", "").toFloat();
                currentElectrodeSite.color = "";
                currentElectrodeSite.highlighted = false;
                currentElectrodeSite.enabled = false;
                currentElectrodeSite.linked = false;
                currentElectrodeSite.portName = currentPort.name;
                currentPort.electrodeSites.append(currentElectrodeSite);
            }
            currentPage.ports.append(currentPort);
        }
        QDomNodeList linesList = pagesList.at(page).toElement().elementsByTagName("Line");
        for (int line = 0; line < linesList.length(); line++) {
            Line currentLine;
            currentLine.lineColor = linesList.at(line).toElement().attribute("lineColor", "");
            currentLine.x1 = linesList.at(line).toElement().attribute("x1", "").toFloat();
            currentLine.y1 = linesList.at(line).toElement().attribute("y1", "").toFloat();
            currentLine.x2 = linesList.at(line).toElement().attribute("x2", "").toFloat();
            currentLine.y2 = linesList.at(line).toElement().attribute("y2", "").toFloat();
            currentPage.lines.append(currentLine);
        }
        QDomNodeList textsList = pagesList.at(page).toElement().elementsByTagName("Text");
        for (int text = 0; text < textsList.length(); text++) {
            Text currentText;
            currentText.x = textsList.at(text).toElement().attribute("x", "").toFloat();
            currentText.y = textsList.at(text).toElement().attribute("y", "").toFloat();
            currentText.fontHeight = textsList.at(text).toElement().attribute("fontHeight", "").toFloat();
            currentText.fontColor = textsList.at(text).toElement().attribute("fontColor", "");
            currentText.textAlignment = textsList.at(text).toElement().attribute("textAlignment", "");
            currentText.rotation = textsList.at(text).toElement().attribute("rotation", "0").toFloat();
            currentText.text = textsList.at(text).toElement().attribute("text", "");
            currentPage.texts.append(currentText);
        }
        electrodeMapSetting->pages.append(currentPage);
    }
    return true;
}
void CElectrodeSettingInterface::clearProbeMapSettings() const
{
    electrodeMapSetting->backgroundColor = "";
    electrodeMapSetting->siteOutlineColor = "";
    electrodeMapSetting->lineColor = "";
    electrodeMapSetting->fontHeight = 0.0;
    electrodeMapSetting->fontColor = "";
    electrodeMapSetting->textAlignment = "";
    electrodeMapSetting->siteShape = "";
    electrodeMapSetting->siteWidth = 0.0;
    electrodeMapSetting->siteHeight = 0.0;
    electrodeMapSetting->pages.clear();
    electrodeMapSetting->row=0;
     electrodeMapSetting->colum =0;;
}
END_NX_NAMESPACE

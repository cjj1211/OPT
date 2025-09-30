#ifndef __C_PAGEVIEW
#define __C_PAGEVIEW
#include "CElectrodeMapDataStructure.h"
#include "nxelectrodemap_global.h"
#include <QWidget>
#include <QPushButton>
#include <QPair>
#include <QMap>
#include <QLabel>
BEGIN_NX_NAMESPACE
#define ZOOM_CONSTANT 0.7
#define BESTFIT_BUFFER 1.2
#define SCROLL_CONSTANT -0.05
enum SelectType {
	SelectAdd,
	SelectReplace,
	Null
};
enum MouseMode {
	Select,
	Scroll,
	Resize
};
enum ViewMode {
	DefaultView,
	ImpedanceView,
	LocationView
};
struct CustomData
{
	bool isCheck = false;
	int  posX;
	int posY;
	QString channelNumber;
	QPair<int, int>index;
	QPair<int, int>buttonIndex;
};
Q_DECLARE_METATYPE(CustomData);
class PageView final : public QWidget
{
	Q_OBJECT
public:
	explicit PageView(ElectrodeMapSettings* electrodeMapSetting, int pageIndex, ViewMode viewMode_, QWidget* parent = nullptr);
	void changeView(ViewMode viewMode_);
	bool searchChannelMap(QPair<int, int>position);
	bool searchChannelMap(QString customname);
	void setbuttonStyle(QMap<QString, QColor> buttonStyle);

	void buttonClick();
public:
	
	int num = 0;
	Page* page;
	QMap<QString, QColor> buttonStyle;
	QMap<QString, QString> impedanceValue;
	QString currentPageName;
	QString currentButtonCoustName;
signals:
	void signalShowCurrentChannelName(int currentButtonX, int currentButtonY, QString channelNumber);
	void signalUpdateCurrentChannelName(int currentButtonX, int currentButtonY, QString channelNumber);
public slots:
	void showChannelName();
protected:
	void paintEvent(QPaintEvent* event) override;
	void resizeEvent(QResizeEvent* event) override;
	void keyPressEvent(QKeyEvent* event) override;
	bool eventFilter(QObject* obj, QEvent* event) override;
private:
	// Display size/scaling
	QSize minimumSizeHint() const override;
	QSize sizeHint() const override;
	int xUnitsToPixelValue(float xUnit);
	int yUnitsToPixelValue(float yUnit);
	float xPixelsToUnitValue(int xPixel);
	float yPixelsToUnitValue(int yPixel);
	int xSizeToPixelSize(float xSize);
	int ySizeToPixelSize(float ySize);
	void getDesiredSize(float& newXMin, float& newXMax, float& newYMin, float& newYMax);
	void zoomToDesiredSize(float newXMin, float newXMax, float newYMin, float newYMax);
	void scrollOffset(float xOffset, float yOffset);
	bool siteAtCoordinates(float xCoord, float yCoord, QString& portName, int& customChannelNum);
	bool siteIncludesCoordinates(int port, int site, float xCoord, float yCoord);
	void initializeDisplay();
	void drawBackground();
	void drawLines();
	void drawTexts();
	void calculateCorners(const QString alignment, const float height, const float width, const float rotation,
		float& aX0, float& aY0, float& bX0, float& bY0, float& cX0, float& cY0, float& dX0, float& dY0);
	void drawSites();
	void drawRect();
	// Consulting XML for correct parameters
	QColor getBackgroundColor();
	QColor getLineColor(int line);
	QColor getSiteOutlineColor(int port, int site);
	QColor getFontColor(int text);
	float getFontHeight(int text);
	QString getTextAlignment(int text);
	float getSiteWidth(int port, int site);
	float getSiteHeight(int port, int site);
	QString getSiteShape(int port, int site);
	// Overwriting parameters
	void overwriteColor(QColor& color, const QString& colorName);
	void overwriteFloat(float& original, float newValue);
	void overwriteShape(QString& original, const QString& newString);
	void overwriteAlignment(QString& original, const QString& newString);
	// User interacting with PageView
	QVector<ElectrodeSite*> getSites(const QString& portName, const int channelNumber) const;
	QVector<ElectrodeSite*> getSitesWithinSelectRect();
	ViewMode viewMode;
	float unitsToPixel{};
	bool isHoverSite{ false };
	QString hoveredSitePortName;
	int hoveredSiteCustomNum;
	bool scrolling;
	float scrollStartX;
	float scrollStartY;
	bool resizing;
	int resizingStartX;
	int resizingStartY;
	int resizingEndX{};
	int resizingEndY{};
	QRect resizeRect;
	SelectType selecting;
	int selectingStartX;
	int selectingStartY;
	int selectingEndX{};
	int selectingEndY{};
	QRect selectRect;
	QString backgroundColor;
	QString lineColor;
	QString siteOutlineColor;
	float fontHeight;
	QString fontColor;
	QString textAlignment;
	QString siteShape;
	float siteWidth;
	float siteHeight;
	QPixmap pixmap;
	float bestfitXMin;
	float bestfitYMin;
	float bestfitWidth;
	float bestfitHeight;
	float visibleFrameXMin{};
	float visibleFrameYMin{};
	float visibleFrameWidth{};
	float visibleFrameHeight{};
	float visibleFrameCenterX{};
	float visibleFrameCenterY{};
	float currentZoomWidth{};
	float currentZoomHeight{};
	static const int PageViewXSize = 200;
	static const int PageViewYSize = 200;
	
	int cricalWith = 30;

	int startX = 183;
	int startY = 18;
	int pageViewX=18;
	int pageViewY=18;
	std::vector<std::vector<QPushButton*>> buttonArray;
	QPair<int, int>currentButtonIndex = QPair(0, 0);
	QPair<int, int>preButtonIndex = QPair(0, 0);
	ElectrodeMapSettings* electrodeMapSettings;
	int row;
	int colum;
	QLabel* impedanceLabel;
	QLabel* enterLabel;
	QString LabelStyel = "QLabel{ "
		"background-color: rgba(0, 0, 0, 0.50);"
		"border-radius: 4px;"
		"font-size: 10px;"
		"font-family: 'PingFangSC-Semibold', 'PingFang SC Semibold', 'PingFang SC', sans-serif, 'PingFangSC-Semibold', 'PingFang SC Semibold', 'PingFang SC', sans-serif-650;"
		"font-weight: bold;"
		"color: #ffffff;"
		"text-align: center;"
		"}";
};
END_NX_NAMESPACE
#endif
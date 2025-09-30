#include "PageView.h"
#include <QMessageBox>
#include <QPainter>
#include <QEvent>
#include <QCloseEvent>
#include <QDebug>
#include <QMetaType>
#include <QApplication>
#include <QDesktopWidget>
#include "RhxGlobals.h"
#include <QVBoxLayout>
BEGIN_NX_NAMESPACE
const double Pi = 3.14;
PageView::PageView(ElectrodeMapSettings* electrodeMapSetting, int pageIndex, const ViewMode viewMode_, QWidget* parent) :
	QWidget(parent),
	viewMode(viewMode_),
	page(new Page(electrodeMapSetting->pages[pageIndex])),
	backgroundColor("#283C82"),
	lineColor(electrodeMapSetting->lineColor),
	siteOutlineColor(electrodeMapSetting->siteOutlineColor),
	fontHeight(electrodeMapSetting->fontHeight),
	fontColor(electrodeMapSetting->fontColor),
	textAlignment(electrodeMapSetting->textAlignment),
	siteShape(electrodeMapSetting->siteShape),
	siteWidth(electrodeMapSetting->siteWidth),
	siteHeight(electrodeMapSetting->siteHeight),
	electrodeMapSettings(new ElectrodeMapSettings())
{
	scrolling = false;
	scrollStartX = 0;
	scrollStartY = 0;
	resizing = false;
	resizingStartX = 0;
	resizingStartY = 0;
	electrodeMapSettings = electrodeMapSetting;
	resizeRect.setCoords(resizingStartX, resizingStartY, width(), height());
	selecting = Null;
	selectingStartX = 0;
	selectingStartY = 0;
	selectRect = QRect();
	impedanceLabel = new QLabel(this);
	impedanceLabel->setVisible(false);
	impedanceLabel->setAlignment(Qt::AlignCenter); // 将文本水平和垂直居中显示
	enterLabel = new QLabel(this);
	enterLabel->setVisible(false);
	enterLabel->setAlignment(Qt::AlignCenter); // 将文本水平和垂直居中显示
	 cricalWith = electrodeMapSetting->buttonsize;

	 startX = electrodeMapSetting->positionx;
	 startY = electrodeMapSetting->positiony;
	 pageViewX = electrodeMapSetting->space;
	 pageViewY = electrodeMapSetting->space;
	 row= electrodeMapSetting->row;
	 colum= electrodeMapSetting->colum;
	// 在CustomData之前添加Q_DECLARE_METATYPE
		// 然后在主函数之前或其他需要使用CustomData的地方，例如在连接信号和槽之前，添加以下代码以注册CustomData类型：
	qRegisterMetaType<CustomData>("CustomData");
	// Initialize spike time to decay time (so that each site's initial state is fully decayed, i.e. not spiking)
	for (auto& port : page->ports)
	{
		for (auto& electrodeSite : port.electrodeSites)
		{
			electrodeSite.heatScale = 0;
		}
	}
	setBackgroundRole(QPalette::Window);
	setAutoFillBackground(true);
	setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	setFocusPolicy(Qt::StrongFocus);
	qreal dpr = devicePixelRatioF();
	pixmap = QPixmap(width() * dpr, height() * dpr);
	pixmap.setDevicePixelRatio(dpr);
	pixmap.fill(Qt::transparent);
	if (page->ports.size() < 1) {
		QMessageBox::critical(this, "Error", "No ports are present on this page-> At least one port with an electrode site must be present to be displayed");
		return;
	}
	// If there are no electrode sites present, warn user and exit
	int numSites = 0;
	for (int port = 0; port < page->ports.size(); ++port) {
		numSites += page->ports[port].electrodeSites.size();
	}
	if (numSites < 1) {
		return;
	}
	// Populate each electrode site's boundary rectangle attributes.
	for (int port = 0; port < page->ports.size(); ++port) {
		for (int site = 0; site < page->ports[port].electrodeSites.size(); ++site) {
			ElectrodeSite* thisSite = &page->ports[port].electrodeSites[site];
			thisSite->boundXMin = thisSite->x - getSiteWidth(port, site) / 2;
			thisSite->boundXMax = thisSite->x + getSiteWidth(port, site) / 2;
			thisSite->boundYMin = thisSite->y - getSiteHeight(port, site) / 2;
			thisSite->boundYMax = thisSite->y + getSiteHeight(port, site) / 2;
		}
	}
	float boundXMin = 0.0f, boundXMax = 0.0f, boundYMin = 0.0f, boundYMax = 0.0f;
	// Initialize bounds around the first electrode site.
	for (int port = 0; port < page->ports.size(); ++port) {
		if (page->ports[port].electrodeSites.size() > 0) {
			boundXMin = page->ports[port].electrodeSites[0].boundXMin;
			boundXMax = page->ports[port].electrodeSites[0].boundXMax;
			boundYMin = page->ports[port].electrodeSites[0].boundYMin;
			boundYMax = page->ports[port].electrodeSites[0].boundYMax;
		}
	}
	// Cycle through all electrode sites, getting bounds.
	for (int port = 0; port < page->ports.size(); ++port) {
		for (int electrodeSite = 0; electrodeSite < page->ports[port].electrodeSites.size(); ++electrodeSite) {
			boundXMin = std::min(page->ports[port].electrodeSites[electrodeSite].boundXMin, boundXMin);
			boundXMax = std::max(page->ports[port].electrodeSites[electrodeSite].boundXMax, boundXMax);
			boundYMin = std::min(page->ports[port].electrodeSites[electrodeSite].boundYMin, boundYMin);
			boundYMax = std::max(page->ports[port].electrodeSites[electrodeSite].boundYMax, boundYMax);
		}
	}
	// Cycle through all lines, getting bounds.
	for (int line = 0; line < page->lines.size(); ++line) {
		boundXMin = std::min({ page->lines[line].x1, page->lines[line].x2, boundXMin });
		boundXMax = std::max({ page->lines[line].x1, page->lines[line].x2, boundXMax });
		boundYMin = std::min({ page->lines[line].y1, page->lines[line].y2, boundYMin });
		boundYMax = std::max({ page->lines[line].y1, page->lines[line].y2, boundYMax });
	}
	// Cycle through all texts, getting bounds.
	for (int text = 0; text < page->texts.size(); ++text) {
		// Create dummy QFont (that won't get used) just to get the ratio of width-to-height for this machine's 'TypeWriter' style font.
		QFont thisText("Courier");
		int dummyHeight = 100; // Arbitrary height for dummy text
		thisText.setPixelSize(dummyHeight);
		QFontMetrics metrics(thisText);
		float dummyWidth = metrics.horizontalAdvance('A'); // Arbitrary character. Assuming this is a monospace font, any one character should have the same width.
		float widthToHeightRatio = dummyWidth / dummyHeight;
		/* A ------------ B
		 * |              |
		 * |   text ....  |
		 * |              |
		 * D ------------ C */
		 // Calculate the coordinates of four points defining the (possibly rotated) rectangle containing the text -
		 // their coordinates where origin defined by alignment is (0, 0)
		float aX0 = 0.0f, aY0 = 0.0f, bX0 = 0.0f, bY0 = 0.0f, cX0 = 0.0f, cY0 = 0.0f, dX0 = 0.0f, dY0 = 0.0f;
		QString alignment = getTextAlignment(text);
		float textHeight = getFontHeight(text);
		float textWidth = widthToHeightRatio * textHeight * page->texts[text].text.length();
		//float rotation = page->texts[text].rotation * DegreesToRadiansF;
		//calculateCorners(alignment, textHeight, textWidth, rotation, aX0, aY0, bX0, bY0, cX0, cY0, dX0, dY0);
		// Calculate the actual coordinates defining the (possibly rotated) rectangle containing the text.
		float aX = page->texts[text].x + aX0;
		float aY = page->texts[text].y + aY0;
		float bX = page->texts[text].x + bX0;
		float bY = page->texts[text].y + bY0;
		float cX = page->texts[text].x + cX0;
		float cY = page->texts[text].y + cY0;
		float dX = page->texts[text].x + dX0;
		float dY = page->texts[text].y + dY0;
		// Store the coordinates of the bottom-left corner of the rectangle (from the perspective of the text) for later drawing of the text.
		page->texts[text].bottomLeftX = dX;
		page->texts[text].bottomLeftX = dY;
		page->texts[text].topLeftX = aX;
		page->texts[text].topLeftY = aY;
		page->texts[text].bottomRightX = cX;
		page->texts[text].bottomRightY = cY;
		// Determine minimum and maximum x and y values of this rectangle.
		boundXMin = std::min({ aX, bX, cX, dX, boundXMin });
		boundXMax = std::max({ aX, bX, cX, dX, boundXMax });
		boundYMin = std::min({ aY, bY, cY, dY, boundYMin });
		boundYMax = std::max({ aY, bY, cY, dY, boundYMax });
	}
	// Make the visible rectangle BESTFIT_BUFFER (120%) larger than the bounding rectangle.
	float boundingRectWidth = boundXMax - boundXMin;
	float boundingRectHeight = boundYMax - boundYMin;
	float visibleRectWidth = BESTFIT_BUFFER * boundingRectWidth;
	float visibleRectHeight = BESTFIT_BUFFER * boundingRectHeight;
	float diffX = visibleRectWidth - boundingRectWidth;
	float diffY = visibleRectHeight - boundingRectHeight;
	// Store minimum coordinates and dimensions of visible rectangle.
	bestfitXMin = boundXMin - diffX / 2;
	float bestfitXmax = boundXMax + diffX / 2;
	bestfitYMin = boundYMin - diffY / 2;
	float bestfitYmax = boundYMax + diffY / 2;
	bestfitWidth = (float)(bestfitXmax - bestfitXMin);
	bestfitHeight = (float)(bestfitYmax - bestfitYMin);
	// Initialize widget to be a reasonable size.
	//initializeDisplay();
	colum = electrodeMapSettings->colum;
	row = electrodeMapSettings->row;

	startX += this->rect().x();
	startY += this->rect().y();
}
// Change if this page should color sites based on impedance, spike activity, or default color.
void PageView::changeView(ViewMode viewMode_)
{
	viewMode = viewMode_;
}
void PageView::paintEvent(QPaintEvent* /* event */)
{
	initializeDisplay();
	QPainter sPainter(this);
	sPainter.setRenderHint(QPainter::Antialiasing);
	sPainter.drawPixmap(0, 0, pixmap);
}
void PageView::resizeEvent(QResizeEvent* /* event */)
{
	// Pixel map used for double buffering - why is this needed?
	pixmap = QPixmap(size());
	pixmap.fill();
	int gridSize = qMin(this->rect().x() + this->rect().width(), this->rect().y() + this->rect().height()); // 每个网格的大小
	//float scal;
	//if (this->rect().width() > this->rect().height())
	//{
	//	scal = (width()) / (height() * 1.00f);
	//	pageViewX = scal * (gridSize) / (electrodeMapSettings->row+2);
	//	pageViewY = (gridSize) / (electrodeMapSettings->colum+2);
	//}
	//else
	//{
	//	scal = (height()) / (width() * 1.00f);
	//	pageViewX = (gridSize) / (electrodeMapSettings->row + 2);
	//	pageViewY = scal * (gridSize) / (electrodeMapSettings->colum + 2);
	//}
	impedanceLabel->setStyleSheet(LabelStyel);
	enterLabel->setStyleSheet(LabelStyel);
	impedanceLabel->setContentsMargins(10, 10, 10, 10);
	enterLabel->setContentsMargins(10, 10, 10, 10);
	//impedanceLabel->setMinimumWidth(pageViewX/3);
	//impedanceLabel->setMaximumHeight(pageViewY);
	
	
	clearMask();
	update();
}
QSize PageView::minimumSizeHint() const
{
	return { PageViewXSize / 10, PageViewYSize / 10 };
}
QSize PageView::sizeHint() const
{
	return { PageViewXSize, PageViewYSize };
}
void PageView::initializeDisplay()
{
	drawBackground();
	drawSites();
}
int PageView::xUnitsToPixelValue(float xUnit)
{
	// Equation for ratio: P = (U - visibleFrameXMin) / visibleFrameWidth
	float ratio = ((xUnit - visibleFrameXMin) / visibleFrameWidth);
	return (int)(ratio * this->width());
}
int PageView::yUnitsToPixelValue(float yUnit)
{
	// Equation for ratio: P = (U - visibleFrameYMin) / visibleFrameHeight
	float ratio = ((yUnit - visibleFrameYMin) / visibleFrameHeight);
	// Flip, to be consistent with user coordinate system in which y increases north (in software, y increases south).
	ratio = 1 - ratio;
	return (int)(ratio * this->height());
}
float PageView::xPixelsToUnitValue(int xPixel)
{
	const float ratio = static_cast<float>(xPixel) / static_cast<float>(this->width());
	return (ratio * visibleFrameWidth) + visibleFrameXMin;
}
float PageView::yPixelsToUnitValue(int yPixel)
{
	float ratio = static_cast<float>(yPixel) / static_cast<float>(this->height());
	ratio = 1 - ratio;
	return (ratio * visibleFrameHeight) + visibleFrameYMin;
}
int PageView::xSizeToPixelSize(float xSize)
{
	// Equation for ratio: P = S / visibleFrameWidth
	const float ratio = xSize / visibleFrameWidth;
	return static_cast<int>(round(ratio * this->width()));
}
int PageView::ySizeToPixelSize(float ySize)
{
	// Equation for ratio: P = S / visibleFrameHeight
	float ratio = ySize / visibleFrameHeight;
	return (int)round(ratio * this->height());
}
void PageView::getDesiredSize(float& newXMin, float& newXMax, float& newYMin, float& newYMax)
{
	float x1 = xPixelsToUnitValue(resizeRect.topLeft().x());
	float y1 = yPixelsToUnitValue(resizeRect.topLeft().y());
	float x2 = xPixelsToUnitValue(resizeRect.bottomRight().x());
	float y2 = yPixelsToUnitValue(resizeRect.bottomRight().y());
	newXMin = std::min(x1, x2);
	newYMin = std::min(y1, y2);
	newXMax = std::max(x1, x2);
	newYMax = std::max(y1, y2);
}
void PageView::zoomToDesiredSize(float newXMin, float newXMax, float newYMin, float newYMax)
{
	// Compare areas of visible rectangle and best-fit rectangle.
	// If visible rectangle's area is too small, no more zooming in.
	const float visibleArea = visibleFrameWidth * visibleFrameHeight;
	const float bestfitArea = bestfitWidth * bestfitHeight;
	if (visibleArea < bestfitArea / 400.0) {
		update();
		return;
	}
	// Determine what boundaries of widget are in USER UNITS.
	const float widgetAspectRatio = static_cast<float>(this->height()) / static_cast<float>(this->width());
	const float desiredAspectRatio = (newYMax - newYMin) / (newXMax - newXMin);
	if (widgetAspectRatio < desiredAspectRatio) {
		// Height of desired rectangle is the limit.
		visibleFrameHeight = newYMax - newYMin;
		visibleFrameYMin = newYMin;
		unitsToPixel = (newYMax - newYMin) / this->height();
		visibleFrameWidth = unitsToPixel * this->width();
		visibleFrameCenterX = (newXMin + newXMax) / 2;
		visibleFrameXMin = visibleFrameCenterX - visibleFrameWidth / 2;
		visibleFrameCenterY = visibleFrameYMin + visibleFrameHeight / 2;
	}
	else {
		// Width of desired rectangle is the limit.
		visibleFrameWidth = newXMax - newXMin;
		visibleFrameXMin = newXMin;
		unitsToPixel = (newXMax - newXMin) / this->width();
		visibleFrameHeight = unitsToPixel * this->height();
		visibleFrameCenterY = (newYMin + newYMax) / 2;
		visibleFrameYMin = visibleFrameCenterY - visibleFrameHeight / 2;
		visibleFrameCenterX = visibleFrameXMin + visibleFrameWidth / 2;
	}
	currentZoomWidth = visibleFrameWidth;
	currentZoomHeight = visibleFrameHeight;
	update();
	initializeDisplay();
}
void PageView::scrollOffset(float xOffset, float yOffset)
{
	visibleFrameCenterX += xOffset;
	visibleFrameCenterY += yOffset;
	update();
}
bool PageView::siteAtCoordinates(float xCoord, float yCoord, QString& portName, int& customChannelNum)
{
	// Cycle through all ports.
	for (int port = 0; port < page->ports.size(); ++port) {
		// Cycle through all sites in this port.
		for (int site = 0; site < page->ports[port].electrodeSites.size(); ++site) {
			const bool match = siteIncludesCoordinates(port, site, xCoord, yCoord);
			if (match)
			{
				portName = page->ports[port].name;
				/*customChannelNum = page->ports[port].electrodeSites[site].channelNumber;*/
				return true;
			}
		}
	}
	return false;
}
bool PageView::siteIncludesCoordinates(int port, int site, float xCoord, float yCoord)
{
	// If given coordinates lie outside of bounding rectangle, just return false.
	if (!(xCoord >= page->ports[port].electrodeSites[site].boundXMin && xCoord <= page->ports[port].electrodeSites[site].boundXMax &&
		yCoord >= page->ports[port].electrodeSites[site].boundYMin && yCoord <= page->ports[port].electrodeSites[site].boundYMax)) {
		return false;
	}
	if (getSiteShape(port, site) == "Rectangle") {  // If site shape is rectangle, then return true.
		return true;
	}
	else if (getSiteShape(port, site) == "Ellipse") {  // If site shape is ellipse, then do ellipse calculation.
		// Ellipse equation: (x^2)/(with/2)^2 + (y^2)/(height/2)^2 = 1
		float x = xCoord - page->ports[port].electrodeSites[site].x;
		float y = yCoord - page->ports[port].electrodeSites[site].y;
		// If left side of equation > 1, given point is outside ellipse. If left side of equation <= 1, given point is inside ellipse.
		float leftSide = pow(x, 2) / pow((getSiteWidth(port, site) / 2), 2);
		leftSide += pow(y, 2) / pow((getSiteHeight(port, site) / 2), 2);
		return leftSide <= 1;
	}
	// Shouldn't happen - just return false.
	return false;
}
void PageView::drawBackground()
{
	QPainter painter;
	painter.begin(&pixmap);
	// Clear entire Widget display area.
	painter.eraseRect(rect());
	// Draw border around Widget display area.
	painter.setPen(Qt::darkGray);
	QRect rect(0, 0, width() - 1, height() - 1);
	painter.fillRect(rect, getBackgroundColor());
	painter.drawRect(rect);
}
void PageView::drawLines()
{
	QPainter painter;
	painter.begin(&pixmap);
	// Draw lines where they are specified.
	for (int line = 0; line < page->lines.size(); ++line) {
		int x1 = xUnitsToPixelValue(page->lines[line].x1);
		int y1 = yUnitsToPixelValue(page->lines[line].y1);
		int x2 = xUnitsToPixelValue(page->lines[line].x2);
		int y2 = yUnitsToPixelValue(page->lines[line].y2);
		// Set pen to correct color.
		painter.setPen(getLineColor(line));
		// Draw line.
		painter.drawLine(x1, y1, x2, y2);
	}
}
void PageView::drawTexts()
{
	QPainter painter;
	painter.begin(&pixmap);
	// Draw texts where they are specified.
	for (int text = 0; text < page->texts.size(); ++text) {
		int topLeftX = xUnitsToPixelValue(page->texts[text].topLeftX);
		int topLeftY = yUnitsToPixelValue(page->texts[text].topLeftY);
		float rotation = -1 * page->texts[text].rotation;
		int fontSizePixels = ySizeToPixelSize(getFontHeight(text));
		QString textContent = page->texts[text].text;
		// Set pen to correct color.
		painter.setPen(getFontColor(text));
		// Draw text.
		QFont thisText("Courier New");
		thisText.setPixelSize(fontSizePixels);
		QFontMetrics metrics(thisText);
		painter.setFont(thisText);
		painter.translate(topLeftX, topLeftY);
		painter.rotate(rotation);
		QPointF topLeft(0, 0);
		QPointF bottomRight(metrics.horizontalAdvance(textContent), metrics.height());
		QRectF thisRect(topLeft, bottomRight);
		painter.drawText(thisRect, textContent);
		painter.rotate(-1 * rotation);
		painter.translate(-1 * topLeftX, -1 * topLeftY);
	}
}
void PageView::calculateCorners(const QString alignment, const float height, const float width, const float rotation, float& aX0, float& aY0, float& bX0, float& bY0, float& cX0, float& cY0, float& dX0, float& dY0)
{
	/* A ---------- B
	 * |            |
	 * |  text ...  |
	 * |            |
	 * D -----------C */
	if (alignment == "BottomLeft") {
		dX0 = 0;
		dY0 = 0;
		aX0 = height * std::cos((Pi / 2) + rotation);
		aY0 = height * std::sin((Pi / 2) + rotation);
		cX0 = width * std::cos(rotation);
		cY0 = width * std::sin(rotation);
		bX0 = cX0 + aX0;
		bY0 = cY0 + aY0;
	}
	else if (alignment == "TopLeft") {
		aX0 = 0;
		aY0 = 0;
		dX0 = height * std::cos((3 * Pi / 2) + rotation);
		dY0 = height * std::sin((3 * Pi / 2) + rotation);
		bX0 = width * std::cos(rotation);
		bY0 = width * std::sin(rotation);
		cX0 = bX0 + dX0;
		cY0 = bY0 + dY0;
	}
	else if (alignment == "TopRight") {
		bX0 = 0;
		bY0 = 0;
		cX0 = height * std::cos((3 * Pi / 2) + rotation);
		cY0 = height * std::sin((3 * Pi / 2) + rotation);
		aX0 = width * std::cos((Pi)+rotation);
		aY0 = width * std::sin((Pi)+rotation);
		dX0 = aX0 + cX0;
		dY0 = aY0 + cY0;
	}
	else if (alignment == "BottomRight") {
		cX0 = 0;
		cY0 = 0;
		bX0 = height * std::cos((Pi / 2) + rotation);
		bY0 = height * std::sin((Pi / 2) + rotation);
		dX0 = width * std::cos((Pi)+rotation);
		dY0 = width * std::sin((Pi)+rotation);
		aX0 = dX0 + bX0;
		aY0 = dY0 + bY0;
	}
	else if (alignment == "CenterLeft") {
		aX0 = 0.5 * height * std::cos((Pi / 2) + rotation);
		aY0 = 0.5 * height * std::sin((Pi / 2) + rotation);
		dX0 = 0.5 * height * std::cos((3 * Pi / 2) + rotation);
		dY0 = 0.5 * height * std::sin((3 * Pi / 2) + rotation);
		bX0 = width * std::cos(rotation) + aX0;
		bY0 = width * std::sin(rotation) + aY0;
		cX0 = width * std::cos(rotation) + dX0;
		cY0 = width * std::sin(rotation) + dY0;
	}
	else if (alignment == "CenterTop") {
		aX0 = 0.5 * width * std::cos((Pi)+rotation);
		aY0 = 0.5 * width * std::sin((Pi)+rotation);
		bX0 = 0.5 * width * std::cos(rotation);
		bY0 = 0.5 * width * std::sin(rotation);
		cX0 = height * std::cos((3 * Pi / 2) + rotation) + bX0;
		cY0 = height * std::sin((3 * Pi / 2) + rotation) + bY0;
		dX0 = height * std::cos((3 * Pi / 2) + rotation) + aX0;
		dY0 = height * std::sin((3 * Pi / 2) + rotation) + aY0;
	}
	else if (alignment == "CenterRight") {
		bX0 = 0.5 * height * std::cos((Pi / 2) + rotation);
		bY0 = 0.5 * height * std::sin((Pi / 2) + rotation);
		cX0 = 0.5 * height * std::cos((3 * Pi / 2) + rotation);
		cY0 = 0.5 * height * std::sin((3 * Pi / 2) + rotation);
		aX0 = width * std::cos((Pi)+rotation) + bX0;
		aY0 = width * std::sin((Pi)+rotation) + bY0;
		dX0 = width * std::cos((Pi)+rotation) + cX0;
		dY0 = width * std::sin((Pi)+rotation) + cY0;
	}
	else if (alignment == "CenterBottom") {
		cX0 = 0.5 * width * std::cos(rotation);
		cY0 = 0.5 * width * std::sin(rotation);
		dX0 = 0.5 * width * std::cos((Pi)+rotation);
		dY0 = 0.5 * width * std::sin((Pi)+rotation);
		bX0 = height * std::cos((Pi / 2) + rotation) + cX0;
		bY0 = height * std::sin((Pi / 2) + rotation) + cY0;
		aX0 = height * std::cos((Pi / 2) + rotation) + dX0;
		aY0 = height * std::sin((Pi / 2) + rotation) + dY0;
	}
	else if (alignment == "Center") {
		float centerLeftX = 0.5 * width * std::cos((Pi)+rotation);
		float centerLeftY = 0.5 * width * std::sin((Pi)+rotation);
		float centerTopX = 0.5 * height * std::cos((Pi / 2) + rotation);
		float centerTopY = 0.5 * height * std::sin((Pi / 2) + rotation);
		float centerRightX = 0.5 * width * std::cos(rotation);
		float centerRightY = 0.5 * width * std::sin(rotation);
		float centerBottomX = 0.5 * height * std::cos((3 * Pi / 2) + rotation);
		float centerBottomY = 0.5 * height * std::sin((3 * Pi / 2) + rotation);
		aX0 = centerLeftX + centerTopX;
		aY0 = centerLeftY + centerTopY;
		bX0 = centerRightX + centerTopX;
		bY0 = centerRightY + centerTopY;
		cX0 = centerRightX + centerBottomX;
		cY0 = centerRightY + centerBottomY;
		dX0 = centerLeftX + centerBottomX;
		dY0 = centerLeftY + centerBottomY;
	}
	else {
		qDebug() << "This alignment not supported yet...";
	}
}
void PageView::drawSites()
{
	QPainter painter;
	painter.begin(&pixmap);
	painter.setRenderHint(QPainter::Antialiasing);
	// 创建二维数组来存储 QPushButton 对象
	if (num == 0)
	{
		buttonArray.resize(page->ports.size());
		for (int port = 0; port < page->ports.size(); ++port) {
			for (int site = 0; site < page->ports[port].electrodeSites.size(); ++site) {
				const int cols = row*colum;
				for (int i = 0; i < page->ports.size(); ++i) {
					buttonArray[i].resize(cols);
				}
				// Set pen to correct color.
				painter.setPen(getSiteOutlineColor(port, site));
				// Get correct width and height of site, according to hierarchy from xml, and convert them to pixel size.
				const auto siteWidth = xSizeToPixelSize(getSiteWidth(port, site));
				const auto siteHeight = ySizeToPixelSize(getSiteHeight(port, site));
				// Get correct shape.
				QString shape = getSiteShape(port, site);
				// Get x and y coordinates of site in pixels.
				QPointF siteOrigin(xUnitsToPixelValue((page->ports[port].electrodeSites[site].x) * pageViewX), yUnitsToPixelValue(page->ports[port].electrodeSites[site].y) * pageViewY);
				if (shape != "Ellipse" && shape != "Rectangle") {
					qDebug() << "This electrode site didn't have a valid shape... not plotting";
					return;
				}
				/* const bool ellipse = shape == "Ellipse";*/
				const double radiusW = (static_cast<double>(pageViewX * 0.8) / 2.0); // 椭圆宽度方向半径
				const double radiusH = (static_cast<double>(pageViewX * 0.8) / 2.0); // 椭圆高度方向半径
				// If this site is disabled, draw X at this location, and continue to the next site.
				if (!page->ports[port].electrodeSites[site].enabled) {
					painter.setBrush(Qt::NoBrush);
					QPushButton* ellipseButton = new QPushButton(this);
					ellipseButton->setFocusPolicy(Qt::NoFocus);
					ellipseButton->setFixedSize(static_cast<int>(radiusW), static_cast<int>(radiusW));
					// 设置按钮的样式，将其形状设置为椭圆形状
					QString styel = "QPushButton {"
						"    border-radius: %1px;"
						"    background-color: transparent;"
						"    border: 2px solid white;  "
						"}";
					ellipseButton->setStyleSheet(styel.arg(static_cast<int>(radiusW) / 2));
					/*			ellipseButton->show();*/
					CustomData customData;
					customData.channelNumber = QString::fromStdString(page->ports[port].electrodeSites[site].channelNumber);
					customData.posX = page->ports[port].electrodeSites[site].x;
					customData.posY = page->ports[port].electrodeSites[site].y;
					customData.index = QPair(port, site);
					ellipseButton->setProperty("customData", QVariant::fromValue(customData));
					buttonArray[port][site] = ellipseButton;
					buttonArray[port][site]->installEventFilter(this);
					connect(ellipseButton, &QPushButton::clicked, this, &PageView::showChannelName);
					continue;
				}
			}
		}
	}
	else
	{


		int orgsize = (cricalWith + pageViewX) * row + 2 * startY;
		if (this->height() < orgsize)
		{
			double scal = this->rect().height() * 1.00 / orgsize * 1.00;
			cricalWith = cricalWith * scal;
			pageViewX = pageViewX * scal;
			pageViewY = pageViewY * scal;
		}
		for (int port = 0; port < page->ports.size(); ++port) {
			int col = 0;
			int row = 0;
			for (int site = 0; site < page->ports[port].electrodeSites.size(); ++site) {
				// Set pen to correct color.
				painter.setPen(getSiteOutlineColor(port, site));
				// Get correct width and height of site, according to hierarchy from xml, and convert them to pixel size.
				const auto siteWidth = xSizeToPixelSize(getSiteWidth(port, site));
				const auto siteHeight = ySizeToPixelSize(getSiteHeight(port, site));
				// Get correct shape.
				QString shape = getSiteShape(port, site);
				// Get x and y coordinates of site in pixels.
				QPointF siteOrigin(xUnitsToPixelValue((page->ports[port].electrodeSites[site].x) * pageViewX), yUnitsToPixelValue(page->ports[port].electrodeSites[site].y) * pageViewY);
				if (shape != "Ellipse" && shape != "Rectangle") {
					qDebug() << "This electrode site didn't have a valid shape... not plotting";
					return;
				}
				const double radiusW = (static_cast<int>(cricalWith)); // 椭圆宽度方向半径
				const double radiusH = (static_cast<double>(cricalWith)); // 椭圆高度方向半径
				// If this site is disabled, draw X at this location, and continue to the next site.
				if (!page->ports[port].electrodeSites[site].enabled) {
					painter.setBrush(Qt::NoBrush);
					buttonArray[port][site]->setFixedSize(static_cast<int>(radiusW), static_cast<int>(radiusW));
					// 设置按钮的样式，将其形状设置为椭圆形状
					QString styel = "QPushButton {"
						"    border-radius: %1px;"
						"    background-color: transparent;"
						"    border: 2px solid white;  "
						"}";
					// 设置按钮的样式，将其形状设置为椭圆形状
					buttonArray[port][site]->setStyleSheet(styel.arg(static_cast<int>(radiusW) / 2));

					// 设置按钮的位置
					buttonArray[port][site]->move(static_cast<int>((page->ports[port].electrodeSites[site].y+1) * (pageViewX+ cricalWith)+ startX), static_cast<int>(static_cast<int>((page->ports[port].electrodeSites[site].x+1) * (pageViewY + cricalWith))));
					buttonArray[port][site]->show();
					if (buttonArray[port][site]->property("customData").value<CustomData>().isCheck)
					{
						// 获取按钮的位置
						int buttonX = buttonArray[port][site]->x();
						int buttonY = buttonArray[port][site]->y();
						// 获取按钮的宽度和高度
						int buttonWidth = buttonArray[port][site]->width();
						int buttonHeight = buttonArray[port][site]->height();
						// 计算圆圈的位置
						// 计算圆的中心坐标和半径
						double centerX = buttonX + buttonWidth / 2.0;
						double centerY = buttonY + buttonHeight / 2.0;
						double radius =( buttonWidth/2+6);

						// 设置圆的边界矩形
						QRectF circleRect(centerX-radius, centerY - radius, radius*2, radius*2);

						// 绘制圆形
						painter.setRenderHint(QPainter::Antialiasing);
						QColor color("#1FE9F4");
						QPen pen(color);
						pen.setWidth(2);
						painter.setPen(pen);
						painter.setBrush(Qt::NoBrush);
						painter.drawArc(circleRect, 0, 5760); // 使用drawArc()方法绘制整个圆形（0到5760是表示整个圆的角度范围）
						if (viewMode == ViewMode::LocationView)
						{
							for (auto button : impedanceValue.keys())
							{
								if (currentPageName.left(1) + QString("-%1").arg((buttonArray[port][site]->property("customData").value<CustomData>().channelNumber).toInt(), 3, 10, QLatin1Char('0')) == button)
								{
									impedanceLabel->setText(impedanceValue.value(button));
									impedanceLabel->move(buttonX- impedanceLabel->width()/2, buttonY - impedanceLabel->height());
									impedanceLabel->raise();
									impedanceLabel->show();
									break;
								}
							}
						}
					}
				}

				update();
			}
			continue;
		}
	}
	if (num == 0)
	{
		buttonArray[0][0]->clicked();
	}
	if (viewMode == LocationView)
	{
		setbuttonStyle(buttonStyle);
	}
	num++;
	update();
}
void PageView::drawRect()
{
	QPainter painter;
	painter.begin(&pixmap);
	painter.setRenderHint(QPainter::Antialiasing);
	painter.setPen(Qt::white);
	painter.setBrush(QColor(200, 200, 200, 100));
	if (resizing) {
		painter.drawRect(resizeRect);
	}
	else if (selecting != Null) {
		painter.drawRect(resizeRect);
	}
}
QColor PageView::getBackgroundColor()
{
	// Start with default color (black).
	QColor backgroundcolor("Black");
	// If global color exists and is valid, overwrite previous color.
	overwriteColor(backgroundcolor, backgroundColor);
	// If page-level color exists and is valid, overwrite previous color.
	overwriteColor(backgroundcolor, page->backgroundColor);
	return backgroundcolor;
}
QColor PageView::getLineColor(int line)
{
	// Start with default color (white).
	QColor line_color("White");
	// If global color exists and is valid, overwrite previous color.
	overwriteColor(line_color, lineColor);
	// If page-level color exists and is valid, overwrite previous color.
	overwriteColor(line_color, page->lineColor);
	// If line-level color exists and is valid, overwrite previous color.
	overwriteColor(line_color, page->lines[line].lineColor);
	return line_color;
}
QColor PageView::getSiteOutlineColor(int port, int site)
{
	// Start with default color (gray).
	QColor site_outline_color("Gray");
	// If global color exists and is valid, overwrite previous color.
	overwriteColor(site_outline_color, siteOutlineColor);
	// If page-level color exists and is valid, overwrite previous color.
	overwriteColor(site_outline_color, page->siteOutlineColor);
	// If port-level color exists and is valid, overwrite previous color.
	overwriteColor(site_outline_color, page->ports[port].siteOutlineColor);
	// If site-level color exists and is valid, overwrite previous color.
	overwriteColor(site_outline_color, page->ports[port].electrodeSites[site].siteOutlineColor);
	return site_outline_color;
}
QColor PageView::getFontColor(int text)
{
	// Start with default color (white).
	QColor font_color("White");
	// If global color exists and is valid, overwrite previous color.
	overwriteColor(font_color, fontColor);
	// If page-level color exists and is valid, overwrite previous color.
	overwriteColor(font_color, page->fontColor);
	// If text-level color exists and is valid, overwrite previous color.
	overwriteColor(font_color, page->texts[text].fontColor);
	return font_color;
}
float PageView::getFontHeight(int text)
{
	// Start with default height (0).
	float font_height = 0;
	// If global font height exists and is valid, overwrite previous font height.
	overwriteFloat(font_height, fontHeight);
	// If page-level font height exists and is valid, overwrite previous font height.
	overwriteFloat(font_height, page->fontHeight);
	// If text-level font height exists and is valid, overwrite previous font height.
	overwriteFloat(font_height, page->texts[text].fontHeight);
	return font_height;
}
QString PageView::getTextAlignment(int text)
{
	// Start with default text alignment (BottomLeft).
	QString text_alignment("BottomLeft");
	// If global text alignment exists and is valid, overwrite previous text alignment.
	overwriteAlignment(text_alignment, textAlignment);
	// If page-level text alignment exists and is valid, overwrite previous text alignment.
	overwriteAlignment(text_alignment, page->textAlignment);
	// If text-level text alignment exists and is valid, overwrite previous text alignment.
	overwriteAlignment(text_alignment, page->texts[text].textAlignment);
	return text_alignment;
}
float PageView::getSiteWidth(int port, int site)
{
	// Start with default width (5).
	float site_width = 5;
	// If global width is specified and valid (not 0.0), overwrite previous width.
	overwriteFloat(site_width, siteWidth);
	// If page-level width is specified and valid (not 0.0), overwrite previous width.
	overwriteFloat(site_width, page->siteWidth);
	// If port-level width is specified and valid (not 0.0), overwrite previous width.
	overwriteFloat(site_width, page->ports[port].siteWidth);
	// If site-level width is specified and valid (not 0.0), overwrite previous width.
	overwriteFloat(site_width, page->ports[port].electrodeSites[site].siteWidth);
	return site_width;
}
float PageView::getSiteHeight(int port, int site)
{
	// Start with default height (5).
	float site_height = 5;
	// If global height is specified and valid (not 0.0), overwrite previous height.
	overwriteFloat(site_height, siteHeight);
	// If page-level height is specified and valid (not 0.0), overwrite previous height.
	overwriteFloat(site_height, page->siteHeight);
	// If port-level height is specified and valid (not 0.0), overwrite previous height.
	overwriteFloat(site_height, page->ports[port].siteHeight);
	// If site-level height is specified and valid (not 0.0), overwrite previous height.
	overwriteFloat(site_height, page->ports[port].electrodeSites[site].siteHeight);
	return site_height;
}
QString PageView::getSiteShape(int port, int site)
{
	// Start with default shape (Rectangle).
	QString site_shape = "Rectangle";
	// If global shape is specified and valid (not ""), overwrite previous shape.
	overwriteShape(site_shape, siteShape);
	// If page-level shape is specified and valid (not ""), overwrite previous shape.
	overwriteShape(site_shape, page->siteShape);
	// If port-level shape is specified and valid (not ""), overwrite previous shape.
	overwriteShape(site_shape, page->ports[port].siteShape);
	// If site-level shape is specified and valid (not ""), overwrite previous shape.
	overwriteShape(site_shape, page->ports[port].electrodeSites[site].siteShape);
	return site_shape;
}
void PageView::overwriteColor(QColor& color, const QString& colorName)
{
	if (color.isValidColor(colorName))
		color.setNamedColor(colorName);
}
void PageView::overwriteFloat(float& original, float newValue) {
	if (newValue > 0.0)
		original = newValue;
}
void PageView::overwriteShape(QString& original, const QString& newString)
{
	if ((newString == "Rectangle") || (newString == "Ellipse")) {
		original = newString;
	}
}
void PageView::overwriteAlignment(QString& original, const QString& newString)
{
	if ((newString == "BottomLeft") || (newString == "TopLeft") || (newString == "TopRight") || (newString == "BottomRight") ||
		(newString == "CenterLeft") || (newString == "CenterTop") || (newString == "CenterRight") || (newString == "CenterBottom") || (newString == "Center")) {
		original = newString;
	}
}
QVector<ElectrodeSite*> PageView::getSites(const QString& portName, const int channelNumber) const
{
	QVector<ElectrodeSite*> sites;
	for (int portIndex = 0; portIndex < page->ports.size(); ++portIndex) {
		if (page->ports[portIndex].name != portName) {
			continue;
		}
		for (int siteIndex = 0; siteIndex < page->ports[portIndex].electrodeSites.size(); ++siteIndex) {
			/*if (page->ports[portIndex].electrodeSites[siteIndex].channelNumber == channelNumber) {
				sites.append(&page->ports[portIndex].electrodeSites[siteIndex]);
			}*/
		}
	}
	return sites;
}
QVector<ElectrodeSite*> PageView::getSitesWithinSelectRect()
{
	QVector<ElectrodeSite*> sites;
	// Find the sites within the select rect.
	for (int portIndex = 0; portIndex < page->ports.size(); ++portIndex) {
		for (int siteIndex = 0; siteIndex < page->ports[portIndex].electrodeSites.size(); ++siteIndex) {
			QPoint thisSiteOrigin(xUnitsToPixelValue(page->ports[portIndex].electrodeSites[siteIndex].x), yUnitsToPixelValue(page->ports[portIndex].electrodeSites[siteIndex].y));
			// If this site is within the select rect, append it.
			if (selectRect.contains(thisSiteOrigin)) {
				sites.append(&page->ports[portIndex].electrodeSites[siteIndex]);
			}
		}
	}
	return sites;
}
void  PageView::showChannelName()
{
	QPushButton* clickedButton = qobject_cast<QPushButton*>(sender());
	if (clickedButton) {
		auto pre = buttonArray[currentButtonIndex.first][currentButtonIndex.second]->property("customData").value<CustomData>();
		pre.isCheck = false;
		buttonArray[currentButtonIndex.first][currentButtonIndex.second]->setProperty("customData", QVariant::fromValue(pre));
		auto current = clickedButton->property("customData").value<CustomData>();
		current.isCheck = true;
		preButtonIndex = QPair(currentButtonIndex.first, currentButtonIndex.second);
		currentButtonIndex = QPair(current.index.first, current.index.second);
		buttonArray[currentButtonIndex.first][currentButtonIndex.second]->setProperty("customData", QVariant::fromValue(current));
		emit signalShowCurrentChannelName(current.posX, current.posY, current.channelNumber);
	}
}
bool PageView::searchChannelMap(QPair<int, int>position)
{
	for (auto row : buttonArray) {
		for (auto element : row) {
			auto data = element->property("customData").value<CustomData>();
			if (QPair(data.posX, data.posY) == position)
			{
				element->clicked();
				return true;
			}
		}
	}
	return false;
}
bool  PageView::searchChannelMap(QString customname)
{
	for (auto row : buttonArray) {
		for (auto element : row) {
			auto data = element->property("customData").value<CustomData>();
			if (data.channelNumber == customname)
			{
				element->clicked();
				return true;
			}
		}
	}
	return false;
}
void  PageView::keyPressEvent(QKeyEvent* event)
{
	// 处理按键按下事件
	if (event->key() == Qt::Key_Up) {
		if (currentButtonIndex.second - 1 >= 0)
		{
			emit buttonArray[currentButtonIndex.first][(currentButtonIndex.second - 1)]->clicked();
		}
	}
	else if (event->key() == Qt::Key_Down) {
		if ((currentButtonIndex.second + 1) < page->ports[0].electrodeSites.size())
		{
			emit buttonArray[currentButtonIndex.first][currentButtonIndex.second + 1]->clicked();
		}
	}
	else if (event->key() == Qt::Key_Left) {
		if (currentButtonIndex.second - 1 >= 0)
		{
			emit buttonArray[currentButtonIndex.first][currentButtonIndex.second - 1]->clicked();
		}
	}
	else if (event->key() == Qt::Key_Right) {
		if (currentButtonIndex.second + 1 < page->ports[0].electrodeSites.size())
		{
			emit buttonArray[currentButtonIndex.first][currentButtonIndex.second + 1]->clicked();
		}
	}
	// 调用父类的方法以确保事件继续传播
	QWidget::keyPressEvent(event);
}
bool PageView::eventFilter(QObject* obj, QEvent* event)
{
	if (event->type() == QEvent::Enter) {
		if (QPushButton* thisbutton = qobject_cast<QPushButton*>(obj)) {

			if (viewMode == ViewMode::LocationView)
			{
				for (auto button : impedanceValue.keys())
				{
					if (currentPageName.left(1) + QString("-%1").arg((thisbutton->property("customData").value<CustomData>().channelNumber).toInt(), 3, 10, QLatin1Char('0')) == button)
					{
						enterLabel->setText(impedanceValue.value(button));
						enterLabel->move(thisbutton->x()- enterLabel->width()/2, thisbutton->y() - enterLabel->height());
						enterLabel->raise();
						enterLabel->show();
						break;
					
					}
				}
			}
			else
			{
				emit signalUpdateCurrentChannelName(thisbutton->property("customData").value<CustomData>().posX, thisbutton->property("customData").value<CustomData>().posY, thisbutton->property("customData").value<CustomData>().channelNumber);
				impedanceLabel->setText(currentButtonCoustName);
				impedanceLabel->move(thisbutton->x(), thisbutton->y() - impedanceLabel->height());
				impedanceLabel->show();
			
			}

		}
	}
	else if (event->type() == QEvent::Leave) {
		if (QPushButton* button = qobject_cast<QPushButton*>(obj)) {
			impedanceLabel->hide();
			enterLabel->hide();
		}
	
	}
	return QObject::eventFilter(obj, event);
}
void PageView::setbuttonStyle(QMap<QString, QColor> buttonStyle)
{
	QString styel = "QPushButton {"
		"    border-radius: %1px;"
		"    background-color: %2;"
		"    border: 2px solid white;  "
		"}";
	for (auto button : buttonStyle.keys())
	{
		for (auto row : buttonArray) {
			for (auto element : row) {
				auto data = element->property("customData").value<CustomData>();
				if (currentPageName.left(1) + QString("-%1").arg((data.channelNumber).toInt(), 3, 10, QLatin1Char('0')) == button)
				{
					element->setStyleSheet(styel.arg(element->height() / 2).arg(buttonStyle.value(button).name()));
					break;
				}
			}
		}
	}
}
void PageView::buttonClick()
{
	buttonArray[0][0]->clicked();
}


END_NX_NAMESPACE
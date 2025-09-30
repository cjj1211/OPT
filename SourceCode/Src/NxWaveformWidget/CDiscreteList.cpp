#include "CDiscreteList.h"

BEGIN_NX_NAMESPACE

int CDiscreteList::addItem(const std::string& valueName, const std::string& displayedValueName,
	double numericalValue)
{
	std::vector<std::string> valueNames;
	valueNames.emplace_back(valueName);
	Discrete item = { valueNames, displayedValueName, numericalValue };
	items.emplace_back(item);
	return static_cast<int>(items.size()) - 1;    // Return index to new item.
}

bool CDiscreteList::addAlternateValueName(const std::string& valueName, const std::string& alternateValueName)
{
	int index = getIndex(valueName);
	if (index < 0) return false;
	items[index].valueNames.push_back(alternateValueName);
	return true;
}

int CDiscreteList::numberOfItems() const
{
	return static_cast<int>(items.size());
}

double CDiscreteList::getNumericValue(int index) const
{
	if (index < 0 || index >= (int)items.size()) {
		return 0.0;
	}
	return items[index].numericValue;
}

double CDiscreteList::getNumericValue() const
{
	return getNumericValue(currentIndex);
}

bool CDiscreteList::setValue(const std::string& valueName)
{
	int index = getIndex(valueName);
	if (index < 0) return false;
	if (currentIndex == index) return true;
	currentIndex = index;
	//systemState.forceUpdate();
	return true;
}

std::string CDiscreteList::getValue()
{
	return getValue(currentIndex);
}

inline std::string CDiscreteList::getValue(int index) const
{
	if (index < 0 || index >= (int)items.size()) {
		return "";
	}
	return items[index].valueNames[0];
}

inline std::string CDiscreteList::getDisplayValueString(int index) const {
	if (index < 0 || index >= static_cast<int>(items.size())) {
		return "";
	}
	return items[index].displayedValueName;
}

inline int CDiscreteList::getIndex()
{
	return currentIndex;
}

inline int CDiscreteList::getIndex(const std::string& valueName)
{
	const int size = static_cast<int>(items.size());
	for (int i = 0; i < size; ++i) {
		for (auto& name : items[i].valueNames) {
			if (QString::fromStdString(name).toLower() == QString::fromStdString(valueName).toLower()) {
				return i;
			}
		}
		if (items[i].displayedValueName == valueName) {
			return i;
		}
	}
	return -1;
}

inline int CDiscreteList::shiftIndex(int delta)
{
	int newIndex = currentIndex + delta;
	if (newIndex < 0) {
		newIndex = 0;
	}
	else if (newIndex >= numberOfItems()) {
		newIndex = numberOfItems() - 1;
	}
	int actualChange = newIndex - currentIndex;
	setIndex(newIndex);  // Let setCurrentIndex() determine if state needs to signal that it has changed.
	return actualChange;
}

inline bool CDiscreteList::setIndex(int index)
{
	if (index < 0 || index >= (int)items.size()) return false;
	if (currentIndex == index) return true;
	currentIndex = index;
	return true;
}

END_NX_NAMESPACE
#include "DiscreteList.h"
BEGIN_NX_NAMESPACE

int DiscreteList::addItem(const std::string& valueName, const std::string& displayedValueName,
	double numericalValue)
{
	std::vector<std::string> valueNames;
	valueNames.emplace_back(valueName);
	Discrete item = { valueNames, displayedValueName, numericalValue };
	items.emplace_back(item);
	return static_cast<int>(items.size()) - 1;    // Return index to new item.
}

bool DiscreteList::addAlternateValueName(const std::string& valueName, const std::string& alternateValueName)
{
	int index = getIndex(valueName);
	if (index < 0) return false;
	items[index].valueNames.push_back(alternateValueName);
	return true;
}

int DiscreteList::numberOfItems() const
{
	return static_cast<int>(items.size());
}

double DiscreteList::getNumericValue(int index) const
{
	if (index < 0 || index >= (int)items.size()) {
		return 0.0;
	}
	return items[index].numericValue;
}

double DiscreteList::getNumericValue() const
{
	return getNumericValue(currentIndex);
}

bool DiscreteList::setValue(const std::string& valueName)
{
	int index = getIndex(valueName);
	if (index < 0) return false;
	if (currentIndex == index) return true;
	currentIndex = index;
	//systemState.forceUpdate();
	return true;
}

std::string DiscreteList::getValue()
{
	return getValue(currentIndex);
}

END_NX_NAMESPACE
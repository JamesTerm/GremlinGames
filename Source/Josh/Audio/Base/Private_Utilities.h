#pragma once


//Note: I still think this should be in the base -James
template <typename List>
bool EraseFromList(List &ListToRemoveFrom, const typename List::value_type &Val)
{
	List::iterator Result=std::find(ListToRemoveFrom.begin(),ListToRemoveFrom.end(),Val);
	if (Result==ListToRemoveFrom.end())
		return false;
	ListToRemoveFrom.erase(Result);
	return true;
}


#include "stdafx.h"
#include "query.h"
#include "connection.h"
#include "database.h"

// Constructor
query::query(connection *c, const wchar_t *key_name, const wchar_t *command, DWORD timeout_ms, bool is_multiple_keys):
	m_connection(c), m_key_name(key_name), m_command(command), m_timeout_ms(timeout_ms), m_next_change_time_ms(0), m_is_multiple_keys(is_multiple_keys)
{
}

// Destructor
query::~query()
{
}

// get_next_change_time
// Returns the next time that this query is scheduled to be sent to the database.
DWORD query::get_next_change_time() const
{
	return m_next_change_time_ms;
}

// execute
// Opens a recordset with the stored query on the database. Calls database::set_pair if the recordset
// is not empty. If m_is_multiple_keys is true, also calls database::set_pair with an affixed dot and
// incrementing index for each record.
unsigned long query::execute(database *db, DWORD current_time, bool suspend_if_no_connection)
{
	ADODB::_RecordsetPtr recordset = NULL;
	_variant_t variant = VT_NULL;
	
	std::wstring key_name_plus_period(m_key_name + L".");
	wchar_t number_string[12];
	unsigned long num_changed_pairs = 0;

	CoInitialize(NULL);	// CoInitialize added here because of com error from the recordset.CreateInstance

	if (m_connection->open() != NULL)
	{
		try
		{
			database::TESTHR(recordset.CreateInstance(__uuidof(ADODB::Recordset)));
			database::TESTHR(recordset->Open(m_command.c_str(), m_connection->get_connection()->GetInterfacePtr(), ADODB::adOpenForwardOnly, ADODB::adLockReadOnly, ADODB::adCmdText));

			if (!recordset->ADOEOF)
			{
				database::TESTHR(recordset->Fields->GetItem(long(0))->get_Value(&variant));
				
				// If variant is a null, skip it.
				if (variant.vt!=VT_NULL)
				{
					// Fixes problem with non-bstr variants. Coerces non-string values to a string (VT_BSTR).
					if (variant.vt != VT_BSTR)
						VariantChangeType(&variant, &variant, NULL, VT_BSTR);
					
					if (variant.bstrVal)
					{
						if (db->set_pair(m_key_name.c_str(), variant.bstrVal))
							num_changed_pairs = 1;

						if (m_is_multiple_keys)
						{
							for (unsigned long i = 1; !recordset->ADOEOF; i++)
							{
								database::TESTHR(recordset->Fields->GetItem(long(0))->get_Value(&variant));
								
								// Fixes problem with non-bstr variants. Coerces non-string values to a string (VT_BSTR).
								if (variant.vt != VT_BSTR)
									VariantChangeType(&variant, &variant, NULL, VT_BSTR);

								_ultow(i, number_string, 10);
								std::wstring key_name_plus_index = key_name_plus_period + number_string;
								if (db->set_pair(key_name_plus_index.c_str(), (variant.vt != VT_NULL)? variant.bstrVal: L""))
									num_changed_pairs++;
								database::TESTHR(recordset->MoveNext());
							}
						}
					}
				}
// 				else
// 					assert false;
			}
		}
		catch (_com_error e)
		{
		}
	}
	m_next_change_time_ms = (m_connection->get_connection() == NULL && suspend_if_no_connection)? INFINITE: current_time + m_timeout_ms;

	return num_changed_pairs;
}

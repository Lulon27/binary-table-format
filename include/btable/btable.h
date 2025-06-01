#pragma once

#include <cinttypes>
#include <cstring>

#define BTABLE_FIELD_LIST_OFFSET 24

class BTable
{
public:
	enum DataType
	{
		INT8 = 0,
		INT16,
		INT32,
		INT64,
		FLOAT32,
		FLOAT64,
		STRING
	};

	struct FieldData
	{
		const char* name;
		uint16_t arraySize;
		enum DataType dataType;
	};

	struct Header
	{
		uint32_t magic;
		uint32_t numFields;
		uint32_t numEntries;
		uint32_t dataOffset;
		uint8_t fieldNameLength; // If zero, field names are hashes, otherwise field names are byte arrays
		uint8_t hasStringTable;
		uint8_t endianness; // ?
	};

	struct FieldListEntry
	{
		uint32_t name; // Hash or string table index
		uint32_t offset; // Offset from start of buffer
		uint16_t arraySize;
		uint8_t dataType;
	};

	static unsigned int getDatatypeSize(enum DataType dataType)
	{
		switch (dataType)
		{
		case INT8: return 1;
		case INT16: return 2;
		case INT32: return 4;
		case INT64: return 8;
		case FLOAT32: return 4;
		case FLOAT64: return 8;
		case STRING: return 4;
		default: return 0;
		}
	}

	static constexpr int getPadding(int block_size, int alignment)
	{
		return (alignment - block_size % alignment) % alignment;
	}

	static uint32_t hash(const char* str)
	{
		uint32_t ret = 0;
		while (*str != 0)
		{
			ret *= 0x1F;
			ret += *str;
			str++;
		}
		return ret;
	}

	static uint32_t calculateBufferSize(const FieldData* fields, uint32_t numFields, uint32_t numEntries)
	{
		uint32_t bytesPerEntry = 0;
		for (int i = 0; i < numFields; i++)
		{
			bytesPerEntry += getDatatypeSize(fields[i].dataType) * fields[i].arraySize;
		}
		return bytesPerEntry * numEntries;
	}

	static constexpr uint32_t magic = 0x4254424C;

	BTable(void* buffer, const FieldData* fields, uint32_t numFields, uint32_t numEntries)
	{
		bufferPtr = (int8_t*)buffer;

		Header* header = getHeader();
		header->magic = magic;
		header->numFields = numFields;
		header->numEntries = numEntries;
		header->hasStringTable = 0; // Not implemented
		header->fieldNameLength = 0; // String field names not implemented
		header->endianness = 0; // ?

		uint32_t offset = 0;
		FieldListEntry* fieldList = getFieldList();
		for (int i = 0; i < header->numFields; i++)
		{
			fieldList[i].name = hash(fields[i].name); // Hash
			fieldList[i].offset = offset;
			fieldList[i].dataType = fields[i].dataType;
			fieldList[i].arraySize = fields[i].arraySize;

			offset += getDatatypeSize(fields[i].dataType) * fields[i].arraySize * getNumEntries();
		}

		unsigned int bytesWritten = BTABLE_FIELD_LIST_OFFSET + sizeof(FieldListEntry) * numFields;
		header->dataOffset = bytesWritten + getPadding(bytesWritten, 8);
	}

	Header* getHeader()
	{
		return (Header*)bufferPtr;
	}

	Header* getHeader() const
	{
		return (Header*)bufferPtr;
	}

	FieldListEntry* getFieldList()
	{
		return (FieldListEntry*)(bufferPtr + BTABLE_FIELD_LIST_OFFSET);
	}

	int8_t* getDataSection()
	{
		return bufferPtr + getHeader()->dataOffset;
	}

	uint32_t getNumEntries() const
	{
		return getHeader()->numEntries;
	}

	uint32_t getNumFields() const
	{
		return getHeader()->numFields;
	}

	FieldListEntry* getField(uint32_t fieldIndex)
	{
		return &getFieldList()[fieldIndex];
	}

	FieldListEntry* getField(const char* fieldName)
	{
		return &getFieldList()[getFieldIndex(fieldName)];
	}

	uint32_t getFieldIndex(const char* fieldName)
	{
		uint32_t hash = this->hash(fieldName);
		const Header* header = getHeader();
		const FieldListEntry* fieldList = getFieldList();
		for (int i = 0; i < header->numFields; i++)
		{
			if (fieldList[i].name == hash)
			{
				return i;
			}
		}
		return (uint32_t)-1;
	}

	class BTableValue
	{
	public:
		explicit BTableValue(void* valuePtr) : m_valuePtr(valuePtr)
		{
		}

		int8_t toInt8() const
		{
			return *(int8_t*)m_valuePtr;
		}

	private:
		void* m_valuePtr;

		friend BTable;
	};

	class BTableValueArray
	{
	public:
		explicit BTableValueArray(void* valuePtr) : m_valuePtr(valuePtr)
		{
		}

		int8_t* toInt8Array() const
		{
			return (int8_t*)m_valuePtr;
		}

	private:
		void* m_valuePtr;

		friend BTable;
	};

	// --- Generic getters ---

	BTableValue getEntry(const FieldListEntry* field, uint32_t entry)
	{
		return BTableValue(getValuePtr(field, entry));
	}

	BTableValue getEntry(const FieldListEntry* field, uint32_t entry) const
	{
		return BTableValue(getValuePtr(field, entry));
	}

	BTableValueArray getEntries(const FieldListEntry* field)
	{
		// error check if field is an array?
		return BTableValueArray(bufferPtr + field->offset);
	}

	BTableValueArray getEntries(const FieldListEntry* field) const
	{
		// error check if field is an array?
		return BTableValueArray(bufferPtr + field->offset);
	}

	// --- Generic setters ---

	// sets the array of an entry
	void setArray(const FieldListEntry* field, uint32_t entry, void* srcArray, uint32_t n)
	{
		void* startPtr = getEntry(field, entry).m_valuePtr;
		memcpy_s(startPtr, field->arraySize, srcArray, n);
	}

	// sets every entry in a column. only for single values, not arrays
	void setEntries(const FieldListEntry* field, uint32_t startEntry, void* srcArray, uint32_t n)
	{
		// check if arraySize is != 1 and throw error
		void* startPtr = (int8_t*)getEntries(field).m_valuePtr + startEntry;
		memcpy_s(startPtr, getNumEntries() - startEntry, srcArray, n);
	}

/* -------------------------- Type specific setters ------------------------- */

	// sets only a single value
	void setValueInt8(const FieldListEntry* field, uint32_t entry, uint8_t value)
	{
		// check if arraySize is != 1 and throw error
		*(uint8_t*)getEntry(field, entry).m_valuePtr = value;
	}

private:
	void* getValuePtr(const FieldListEntry* field, uint32_t entry) const
	{
		return bufferPtr + field->offset + getDatatypeSize((DataType)field->dataType) * field->arraySize * entry;
	}

	int8_t* bufferPtr;
};

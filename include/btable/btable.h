#pragma once

#include <cinttypes>
#include <cstring>

class BTable
{
public:

	static constexpr uint32_t magic[] = { 0x42, 0x54, 0x42, 0x4C };
	static constexpr uint32_t field_list_offset = 16;

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

	enum Endianness : uint8_t
	{
		Big = 0,
		Little = 255
	};

	enum Options : uint8_t
	{
		HasStringTable = 0,
		Endianness = 1
	};

	struct FieldData
	{
		const char* name;
		uint16_t arraySize;
		enum DataType dataType;
	};

	struct Header
	{
		uint8_t magic[4];
		uint32_t numFields;
		uint32_t numEntries;
		uint16_t dataOffset;
		uint8_t options;
		uint8_t fieldNameLength; // If zero, field names are hashes, otherwise field names are byte arrays
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

	static constexpr int getPadding(unsigned int block_size, unsigned int alignment)
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
		if(!fields)
		{
			return 16;
		}
		uint32_t bytesPerEntry = 0;
		for (int i = 0; i < numFields; i++)
		{
			bytesPerEntry += getDatatypeSize(fields[i].dataType) * (fields[i].arraySize == 0 ? 1 : fields[i].arraySize);
		}
		int padding = numFields % 2 == 0 ? 0 : 4; // Padding after field list
		return bytesPerEntry * numEntries + 16 + 12 * numFields + padding;
	}

	BTable(void* buffer) : bufferPtr((int8_t*)buffer)
	{
		
	}

	void init(const FieldData* fields, uint32_t numFields, uint32_t numEntries)
	{
		Header* header = getHeader();
		header->magic[0] = magic[0];
		header->magic[1] = magic[1];
		header->magic[2] = magic[2];
		header->magic[3] = magic[3];
		header->numFields = numFields;
		header->numEntries = numEntries;
		header->options = 0;
		header->fieldNameLength = 0; // String field names not implemented

		uint32_t offset = 0;
		FieldListEntry* fieldList = getFieldList();
		for (int i = 0; i < header->numFields; i++)
		{
			fieldList[i].name = hash(fields[i].name); // Hash
			fieldList[i].offset = offset;
			fieldList[i].dataType = fields[i].dataType;
			fieldList[i].arraySize = fields[i].arraySize;
			if(fieldList[i].arraySize == 0)
			{
				fieldList[i].arraySize = 1;
			}

			offset += getDatatypeSize(fields[i].dataType) * fields[i].arraySize * getNumEntries();
		}

		unsigned int bytesWritten = field_list_offset + sizeof(FieldListEntry) * numFields;
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
		return (FieldListEntry*)(bufferPtr + field_list_offset);
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

	// --- Generic getters ---

	void* getEntry(const FieldListEntry* field, uint32_t entry)
	{
		return getValuePtr(field, entry);
	}

	void* getEntry(const FieldListEntry* field, uint32_t entry) const
	{
		return getValuePtr(field, entry);
	}

	void* getEntries(const FieldListEntry* field)
	{
		// error check if field is an array?
		return bufferPtr + getHeader()->dataOffset + field->offset;
	}

	void* getEntries(const FieldListEntry* field) const
	{
		// error check if field is an array?
		return bufferPtr + getHeader()->dataOffset + field->offset;
	}

	// --- Generic setters ---

	// sets the array of an entry
	void setArray(const FieldListEntry* field, uint32_t entry, void* srcArray, uint32_t n)
	{
		void* startPtr = getEntry(field, entry);
		memcpy_s(startPtr, field->arraySize, srcArray, n);
	}

	// sets every entry in a column. only for single values, not arrays
	void setEntries(const FieldListEntry* field, uint32_t startEntry, void* srcArray, uint32_t n)
	{
		// check if arraySize is != 1 and throw error
		void* startPtr = (int8_t*)getEntries(field) + startEntry;
		memcpy_s(startPtr, getNumEntries() - startEntry, srcArray, n);
	}

/* -------------------------- Type specific setters ------------------------- */

	// sets only a single value
	void setValueInt8(const FieldListEntry* field, uint32_t entry, uint8_t value)
	{
		// check if arraySize is != 1 and throw error
		*(uint8_t*)getEntry(field, entry) = value;
	}

/* -------------------------- Type specific getters ------------------------- */

	int8_t getEntryInt8(const FieldListEntry* field, uint32_t entry) const
	{
		return *(uint8_t*)getEntry(field, entry);
	}

private:
	void* getValuePtr(const FieldListEntry* field, uint32_t entry) const
	{
		return bufferPtr + getHeader()->dataOffset + field->offset + getDatatypeSize((DataType)field->dataType) * field->arraySize * entry;
	}

	int8_t* bufferPtr;
};

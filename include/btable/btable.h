#pragma once

#include <cinttypes>
#include <cstring>

template <typename T>
class BTableGeneric
{
public:

	static constexpr uint32_t magic[] = { 0x42, 0x54, 0x42, 0x4C };
	static constexpr uint32_t field_list_offset = 16;
	static constexpr uint32_t field_entry_size = 8;

	static bool isLittleEndianCpu()
	{
		static uint32_t i = 1;
		return *(uint8_t*)&i == 1;
	}

	static inline bool is_little_endian_cpu = isLittleEndianCpu();

	static uint16_t byteswap16(uint16_t x)
	{
		return ((x << 8) & 0xFF00) | ((x >> 8) & 0xFF);
	}

	static uint32_t byteswap32(uint32_t x)
	{
		return ((x >> 24) & 0xFF) | ((x << 8) & 0xFF0000) | ((x >> 8) & 0xFF00) | ((x << 24) & 0xFF000000);
	}

	static uint32_t be32_to_cpu(uint32_t x)
	{
		return is_little_endian_cpu ? byteswap32(x) : x;
	}

	static uint16_t be16_to_cpu(uint16_t x)
	{
		return is_little_endian_cpu ? byteswap16(x) : x;
	}

	static uint32_t cpu_to_be32(uint32_t x)
	{
		return is_little_endian_cpu ? byteswap32(x) : x;
	}

	static uint16_t cpu_to_be16(uint16_t x)
	{
		return is_little_endian_cpu ? byteswap16(x) : x;
	}

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
		uint8_t arraySize;
		enum DataType dataType;
	};

	struct Header
	{
		uint8_t magic[4];
		uint32_t numEntries;
		uint16_t numFields;
		uint16_t dataOffset;
		uint8_t options;
		uint8_t fieldNameLength; // If zero, field names are hashes, otherwise field names are byte arrays (not implemented)
		uint8_t userData[2];
	};

	struct FieldListEntry
	{
		uint32_t offset; // Offset from start of data section
		uint16_t name; // Hash or string table index
		uint8_t dataType;
		uint8_t arraySize;
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

	static uint16_t hash(const char* str)
	{
		uint16_t ret = 0;
		while (*str != 0)
		{
			ret *= 0x1F;
			ret += *str;
			str++;
		}
		return ret;
	}

	static uint32_t getBytesPerEntry(const FieldData* field)
	{
		return getDatatypeSize(field->dataType) * (field->arraySize == 0 ? 1 : field->arraySize);
	}

	static uint32_t getBytesPerEntry(const FieldListEntry* fieldListEntry)
	{
		return getDatatypeSize((DataType)fieldListEntry->dataType) * (fieldListEntry->arraySize == 0 ? 1 : fieldListEntry->arraySize);
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
			bytesPerEntry += getBytesPerEntry(&fields[i]);
		}
		return bytesPerEntry * numEntries + 16 + 8 * numFields;
	}

	BTableGeneric(T buffer, uint32_t size) : bufferPtr(buffer), m_size(size)
	{
		
	}

	void init(const FieldData* fields, uint16_t numFields, uint32_t numEntries)
	{
		Header* header = getHeader();
		header->magic[0] = magic[0];
		header->magic[1] = magic[1];
		header->magic[2] = magic[2];
		header->magic[3] = magic[3];
		header->numEntries = cpu_to_be32(numEntries);
		header->numFields = cpu_to_be16(numFields);
		header->options = 0;
		header->fieldNameLength = 0; // String field names not implemented
		header->userData[0] = 0;
		header->userData[1] = 0;

		uint32_t offset = 0;
		FieldListEntry* fieldList = getFieldList();
		for (int i = 0; i < numFields; i++)
		{
			fieldList[i].offset = cpu_to_be32(offset);
			fieldList[i].name = cpu_to_be16(hash(fields[i].name)); // Hash
			fieldList[i].dataType = fields[i].dataType;
			fieldList[i].arraySize = fields[i].arraySize;
			if(fieldList[i].arraySize == 0)
			{
				fieldList[i].arraySize = 1;
			}

			offset += getDatatypeSize(fields[i].dataType) * fields[i].arraySize * numEntries;
		}

		unsigned int bytesWritten = field_list_offset + sizeof(FieldListEntry) * numFields;
		header->dataOffset = cpu_to_be16(bytesWritten + getPadding(bytesWritten, 8));
	}

	bool validate() const
	{
		const Header* header = getHeader();

		if(m_size < field_list_offset)
		{
			return false;
		}

		if(header->magic[0] != magic[0] ||
		   header->magic[1] != magic[1] ||
		   header->magic[2] != magic[2] ||
		   header->magic[3] != magic[3])
		{
			return false;
		}

		uint16_t numFields = getNumFields();
		if(m_size < field_list_offset + numFields * field_entry_size)
		{
			return false;
		}

		uint32_t bytesPerEntry = 0;
		const FieldListEntry* fieldList = getFieldList();
		for (size_t i = 0; i < numFields; i++)
		{
			bytesPerEntry += getBytesPerEntry(&fieldList[i]);
			if(!(be32_to_cpu(fieldList[i].offset) < m_size))
			{
				return false;
			}
		}
		if(m_size < bytesPerEntry * getNumEntries() + field_list_offset + field_entry_size * numFields)
		{
			return false;
		}

		return true;
	}

	Header* getHeader()
	{
		return reinterpret_cast<Header*>(bufferPtr);
	}

	const Header* getHeader() const
	{
		return reinterpret_cast<const Header*>(bufferPtr);
	}

	FieldListEntry* getFieldList()
	{
		return reinterpret_cast<FieldListEntry*>(bufferPtr + field_list_offset);
	}

	const FieldListEntry* getFieldList() const
	{
		return reinterpret_cast<const FieldListEntry*>(bufferPtr + field_list_offset);
	}

	int8_t* getDataSection()
	{
		return bufferPtr + be16_to_cpu(getHeader()->dataOffset);
	}

	uint32_t getNumEntries() const
	{
		return be32_to_cpu(getHeader()->numEntries);
	}

	uint16_t getNumFields() const
	{
		return be16_to_cpu(getHeader()->numFields);
	}

	void setUserData(uint8_t high, uint8_t low)
	{
		Header* h = getHeader();
		h->userData[0] = high;
		h->userData[1] = low;
	}

	const uint8_t* getUserData() const
	{
		return getHeader()->userData;
	}

	void setUserDataInt16(int16_t userData)
	{
		userData = cpu_to_be16(userData);
		Header* h = getHeader();
		h->userData[0] = (userData & 0xFF00) >> 8;
		h->userData[1] = userData & 0x00FF;
	}

	int16_t getUserDataInt16() const
	{
		const Header* h = getHeader();
		int16_t be = (h->userData[0] << 8) | h->userData[1];
		return be16_to_cpu(be);
	}

	const FieldListEntry* getField(uint32_t fieldIndex) const
	{
		if(fieldIndex == (uint32_t)-1 || fieldIndex >= getNumFields())
		{
			return nullptr;
		}
		return &getFieldList()[fieldIndex];
	}

	const FieldListEntry* getField(const char* fieldName) const
	{
		uint32_t fieldIndex = getFieldIndex(fieldName);
		if(fieldIndex == (uint32_t)-1)
		{
			return nullptr;
		}
		return &getFieldList()[fieldIndex];
	}

	uint32_t getFieldIndex(const char* fieldName) const
	{
		uint16_t hash = cpu_to_be16(this->hash(fieldName));
		const FieldListEntry* fieldList = getFieldList();
		uint32_t numFields = getNumFields();
		for (int i = 0; i < numFields; i++)
		{
			if (fieldList[i].name == hash)
			{
				return i;
			}
		}
		return (uint32_t)-1;
	}

	// --- Generic getters ---

	const void* getValuePtr(const FieldListEntry* field, uint32_t entry) const
	{
		return bufferPtr + be16_to_cpu(getHeader()->dataOffset) + be32_to_cpu(field->offset) + getDatatypeSize((DataType)field->dataType) * field->arraySize * entry;
	}

	void* getEntries(const FieldListEntry* field)
	{
		// error check if field is an array?
		return bufferPtr + be16_to_cpu(getHeader()->dataOffset) + be32_to_cpu(field->offset);
	}

	void* getEntries(const FieldListEntry* field) const
	{
		// error check if field is an array?
		return bufferPtr + be16_to_cpu(getHeader()->dataOffset) + be32_to_cpu(field->offset);
	}

/* -------------------------- Type specific setters ------------------------- */

	// sets only a single value
	void setValueInt8(const FieldListEntry* field, uint32_t entry, uint8_t value)
	{
		// check if arraySize is != 1 and throw error
		*(uint8_t*)getValuePtr(field, entry) = value;
	}

	// sets the array of an entry
	void setArrayInt8(const FieldListEntry* field, uint32_t entry, int8_t* srcArray, uint32_t n)
	{
		// Add error handling of dest size < src size
		void* startPtr = getValuePtr(field, entry);
		if(field->arraySize > n)
		{
			n = field->arraySize;
		}
		memcpy(startPtr, srcArray, n);
	}

	// sets every entry in a column. only for single values, not arrays
	void setEntries(const FieldListEntry* field, uint32_t startEntry, int8_t* srcArray, uint32_t n)
	{
		// check if arraySize is != 1 and throw error
		void* startPtr = (int8_t*)getEntries(field) + startEntry;
		uint32_t dstSize = getNumEntries() - startEntry;
		if(dstSize > n)
		{
			n = dstSize;
		}
		memcpy(startPtr, srcArray, n);
	}

/* -------------------------- Type specific getters ------------------------- */

	int8_t getValueInt8(const FieldListEntry* field, uint32_t entry) const
	{
		return *(uint8_t*)getValuePtr(field, entry);
	}

	int8_t getValueInt8Array(const FieldListEntry* field, uint32_t entry, uint16_t index) const
	{
		// TODO: add error checking
		return ((uint8_t*)getValuePtr(field, entry))[index];
	}

private:
	void* getValuePtr(const FieldListEntry* field, uint32_t entry)
	{
		return bufferPtr + be16_to_cpu(getHeader()->dataOffset) + be32_to_cpu(field->offset) + getDatatypeSize((DataType)field->dataType) * field->arraySize * entry;
	}

	T bufferPtr;
	uint32_t m_size;
};

typedef BTableGeneric<unsigned char*> BTable;
typedef BTableGeneric<const unsigned char*> BTableReadOnly;

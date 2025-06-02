#include "btable/btable.h"
#include <gtest/gtest.h>

TEST(BTableTest, Hash)
{
	EXPECT_EQ(BTable::hash("Value"), 0x04E9A151);
	EXPECT_EQ(BTable::hash("Name"), 0x0024EEAB);
}

TEST(BTableTest, Padding)
{
	EXPECT_EQ(BTable::getPadding(0, 8), 0);
	EXPECT_EQ(BTable::getPadding(1, 8), 7);
	EXPECT_EQ(BTable::getPadding(2, 8), 6);
	EXPECT_EQ(BTable::getPadding(3, 8), 5);
	EXPECT_EQ(BTable::getPadding(4, 8), 4);
	EXPECT_EQ(BTable::getPadding(5, 8), 3);
	EXPECT_EQ(BTable::getPadding(6, 8), 2);
	EXPECT_EQ(BTable::getPadding(7, 8), 1);
	EXPECT_EQ(BTable::getPadding(8, 8), 0);
	EXPECT_EQ(BTable::getPadding(9, 8), 7);

	EXPECT_EQ(BTable::getPadding(15, 8), 1);
	EXPECT_EQ(BTable::getPadding(16, 8), 0);
	EXPECT_EQ(BTable::getPadding(17, 8), 7);

	EXPECT_EQ(BTable::getPadding(15, 16), 1);
	EXPECT_EQ(BTable::getPadding(16, 16), 0);
	EXPECT_EQ(BTable::getPadding(17, 16), 15);
}

TEST(BTableTest, CalculateBufferSizePadding)
{
	BTable::FieldData fields[2];

	fields[0].dataType = BTable::INT8;
	fields[0].arraySize = 1;

	fields[1].dataType = BTable::INT8;
	fields[1].arraySize = 1;

	EXPECT_EQ(BTable::calculateBufferSize(fields, 1, 1), 16 + 12 + 1 + 4 /* 4 is padding */);
	EXPECT_EQ(BTable::calculateBufferSize(fields, 2, 1), 16 + 24 + 2 /* No padding */);
}

TEST(BTableTest, CalculateBufferSizeNull)
{
	EXPECT_EQ(BTable::calculateBufferSize(nullptr, 0, 0), 16);
}

TEST(BTableTest, CalculateBufferSizeMinimum)
{
	BTable::FieldData fields[1];

	fields[0].dataType = BTable::INT8;
	fields[0].arraySize = 1;

	EXPECT_EQ(BTable::calculateBufferSize(fields, 1, 1), 16 + 12 + 1 + 4 /* 4 is padding */);
}

TEST(BTableTest, CalculateBufferSizeZero)
{
	BTable::FieldData fields[1];

	EXPECT_EQ(BTable::calculateBufferSize(fields, 0, 0), 16);
}

TEST(BTableTest, CalculateBufferSizeNoFields)
{
	BTable::FieldData fields[1];

	EXPECT_EQ(BTable::calculateBufferSize(fields, 0, 1), 16);
}

TEST(BTableTest, CalculateBufferSizeNoEntries)
{
	BTable::FieldData fields[1];

	fields[0].dataType = BTable::INT8;
	fields[0].arraySize = 1;

	EXPECT_EQ(BTable::calculateBufferSize(fields, 1, 0), 16 + 12 + 4);
}

TEST(BTableTest, CalculateBufferSizeFieldsEntries)
{
	BTable::FieldData fields[3];

	fields[0].dataType = BTable::INT8;
	fields[0].arraySize = 1;

	fields[1].dataType = BTable::INT8;
	fields[1].arraySize = 1;

	fields[2].dataType = BTable::INT8;
	fields[2].arraySize = 1;

	EXPECT_EQ(BTable::calculateBufferSize(fields, 1, 1), 16 + 12 + 1 + 4 /* 4 is padding */);
	EXPECT_EQ(BTable::calculateBufferSize(fields, 2, 1), 16 + 24 + 2 + 0 /* No padding */);
	EXPECT_EQ(BTable::calculateBufferSize(fields, 3, 1), 16 + 36 + 3 + 4 /* 4 is padding */);

	EXPECT_EQ(BTable::calculateBufferSize(fields, 1, 2), 16 + 12 + 2 + 4 /* 4 is padding */);
	EXPECT_EQ(BTable::calculateBufferSize(fields, 2, 2), 16 + 24 + 4 + 0 /* No padding */);
	EXPECT_EQ(BTable::calculateBufferSize(fields, 3, 2), 16 + 36 + 6 + 4 /* 4 is padding */);

	EXPECT_EQ(BTable::calculateBufferSize(fields, 1, 3), 16 + 12 + 3 + 4 /* 4 is padding */);
	EXPECT_EQ(BTable::calculateBufferSize(fields, 2, 3), 16 + 24 + 6 + 0 /* No padding */);
	EXPECT_EQ(BTable::calculateBufferSize(fields, 3, 3), 16 + 36 + 9 + 4 /* 4 is padding */);
}

TEST(BTableTest, CalculateBufferSizeArrays)
{
	BTable::FieldData fields[1];

	fields[0].dataType = BTable::INT8;
	fields[0].arraySize = 2;

	EXPECT_EQ(BTable::calculateBufferSize(fields, 1, 1), 16 + 12 + 2 + 4 /* 4 is padding */);
	EXPECT_EQ(BTable::calculateBufferSize(fields, 1, 2), 16 + 12 + 4 + 4 /* 4 is padding */);

	fields[0].arraySize = 3;

	EXPECT_EQ(BTable::calculateBufferSize(fields, 1, 1), 16 + 12 + 3 + 4 /* 4 is padding */);
	EXPECT_EQ(BTable::calculateBufferSize(fields, 1, 2), 16 + 12 + 6 + 4 /* 4 is padding */);
}

TEST(BTableTest, CalculateBufferSizeDataTypes)
{
	BTable::FieldData fields[1];

	fields[0].arraySize = 1;

	fields[0].dataType = BTable::INT8;
	EXPECT_EQ(BTable::calculateBufferSize(fields, 1, 1), 16 + 12 + 1 + 4 /* 4 is padding */);
	fields[0].dataType = BTable::INT16;
	EXPECT_EQ(BTable::calculateBufferSize(fields, 1, 1), 16 + 12 + 2 + 4 /* 4 is padding */);
	fields[0].dataType = BTable::INT32;
	EXPECT_EQ(BTable::calculateBufferSize(fields, 1, 1), 16 + 12 + 4 + 4 /* 4 is padding */);
	fields[0].dataType = BTable::INT64;
	EXPECT_EQ(BTable::calculateBufferSize(fields, 1, 1), 16 + 12 + 8 + 4 /* 4 is padding */);
	fields[0].dataType = BTable::FLOAT32;
	EXPECT_EQ(BTable::calculateBufferSize(fields, 1, 1), 16 + 12 + 4 + 4 /* 4 is padding */);
	fields[0].dataType = BTable::FLOAT64;
	EXPECT_EQ(BTable::calculateBufferSize(fields, 1, 1), 16 + 12 + 8 + 4 /* 4 is padding */);
	fields[0].dataType = BTable::STRING;
	EXPECT_EQ(BTable::calculateBufferSize(fields, 1, 1), 16 + 12 + 4 + 4 /* 4 is padding */);
}

TEST(BTableTest, ConstructorMagic)
{
	uint8_t buffer[128];
	BTable::FieldData fields[1];

	BTable(buffer).init(fields, 0, 0); 

	EXPECT_EQ(buffer[0], 0x42);
	EXPECT_EQ(buffer[1], 0x54);
	EXPECT_EQ(buffer[2], 0x42);
	EXPECT_EQ(buffer[3], 0x4C);
}

TEST(BTableTest, ConstructorFieldsEntries)
{
	uint8_t buffer[128];
	BTable::FieldData fields[1];

	BTable(buffer).init(fields, 0, 0);
	EXPECT_EQ(*(uint32_t*)(buffer + 4), 0);
	EXPECT_EQ(*(uint32_t*)(buffer + 8), 0);

	BTable(buffer).init(fields, 1, 0);
	EXPECT_EQ(*(uint32_t*)(buffer + 4), 1);
	EXPECT_EQ(*(uint32_t*)(buffer + 8), 0);

	BTable(buffer).init(fields, 0, 1);
	EXPECT_EQ(*(uint32_t*)(buffer + 4), 0);
	EXPECT_EQ(*(uint32_t*)(buffer + 8), 1);

	BTable(buffer).init(fields, 1, 1);
	EXPECT_EQ(*(uint32_t*)(buffer + 4), 1);
	EXPECT_EQ(*(uint32_t*)(buffer + 8), 1);
}

TEST(BTableTest, ConstructorDataOffset)
{
	uint8_t buffer[128];
	BTable::FieldData fields[3];

	fields[0].name = "";
	fields[1].name = "";
	fields[2].name = "";

	BTable(buffer).init(fields, 0, 0);
	EXPECT_EQ(*(uint16_t*)(buffer + 12), 16);

	BTable(buffer).init(fields, 1, 0);
	EXPECT_EQ(*(uint16_t*)(buffer + 12), 16 + 12 + 4);

	BTable(buffer).init(fields, 2, 0);
	EXPECT_EQ(*(uint16_t*)(buffer + 12), 16 + 24);
}

TEST(BTableTest, ConstructorFieldListArraySize)
{
	uint8_t buffer[128];
	BTable::FieldData fields[1];

	fields[0].name = "";
	fields[0].arraySize = 0;

	BTable(buffer).init(fields, 1, 0);
	EXPECT_EQ(*(uint16_t*)(buffer + 24), 1);

	fields[0].arraySize = 1;

	BTable(buffer).init(fields, 1, 0);
	EXPECT_EQ(*(uint16_t*)(buffer + 24), 1);

	fields[0].arraySize = 2;

	BTable(buffer).init(fields, 1, 0);
	EXPECT_EQ(*(uint16_t*)(buffer + 24), 2);
}

TEST(BTableTest, ConstructorFieldListDataType)
{
	uint8_t buffer[128];
	BTable::FieldData fields[1];

	fields[0].name = "";
	fields[0].dataType = BTable::DataType::INT64;

	BTable(buffer).init(fields, 1, 0);
	EXPECT_EQ(*(uint8_t*)(buffer + 26), BTable::DataType::INT64);
}

TEST(BTableTest, ConstructorFieldListHash)
{
	uint8_t buffer[128];
	BTable::FieldData fields[1];

	fields[0].name = "Name";

	BTable(buffer).init(fields, 1, 0);
	EXPECT_EQ(*(uint32_t*)(buffer + 16), 0x0024EEAB);
}

TEST(BTableTest, ConstructorFieldListEntries)
{
	uint8_t buffer[128];

	BTable::FieldData fields[3];

	fields[0].name = "test";
	fields[0].dataType = BTable::INT32;
	fields[0].arraySize = 1;

	fields[1].name = "jeff";
	fields[1].dataType = BTable::INT16;
	fields[1].arraySize = 3;

	fields[2].name = "value";
	fields[2].dataType = BTable::INT8;
	fields[2].arraySize = 3;

	BTable(buffer).init(fields, 3, 0);

	for(size_t i = 0; i < 3; ++i)
	{
		EXPECT_EQ(*(uint32_t*)(buffer + 16 + i * 12), BTable::hash(fields[i].name));
		EXPECT_EQ(*(uint32_t*)(buffer + 16 + i * 12 + 4), 0);
		EXPECT_EQ(*(uint16_t*)(buffer + 16 + i * 12 + 8), fields[i].arraySize);
		EXPECT_EQ(*(uint8_t*)(buffer + 16 + i * 12 + 10), fields[i].dataType);
	}
}

TEST(BTableTest, ConstructorFieldListOffset)
{
	uint8_t buffer[128];

	BTable::FieldData fields[3];

	fields[0].name = "";
	fields[1].name = "";
	fields[2].name = "";

	fields[0].arraySize = 1;
	fields[1].arraySize = 1;
	fields[2].arraySize = 1;

	fields[0].dataType = BTable::INT8;
	fields[1].dataType = BTable::INT8;
	fields[2].dataType = BTable::INT8;

	BTable(buffer).init(fields, 3, 1);
	EXPECT_EQ(*(uint32_t*)(buffer + 20), 0);
	EXPECT_EQ(*(uint32_t*)(buffer + 32), 1);
	EXPECT_EQ(*(uint32_t*)(buffer + 44), 2);

	fields[0].dataType = BTable::INT16;
	fields[1].dataType = BTable::INT16;
	fields[2].dataType = BTable::INT16;

	BTable(buffer).init(fields, 3, 1);
	EXPECT_EQ(*(uint32_t*)(buffer + 20), 0);
	EXPECT_EQ(*(uint32_t*)(buffer + 32), 2);
	EXPECT_EQ(*(uint32_t*)(buffer + 44), 4);

	fields[0].dataType = BTable::INT8;
	fields[1].dataType = BTable::INT8;
	fields[2].dataType = BTable::INT8;
	fields[0].arraySize = 2;
	fields[1].arraySize = 2;
	fields[2].arraySize = 2;

	BTable(buffer).init(fields, 3, 1);
	EXPECT_EQ(*(uint32_t*)(buffer + 20), 0);
	EXPECT_EQ(*(uint32_t*)(buffer + 32), 2);
	EXPECT_EQ(*(uint32_t*)(buffer + 44), 4);

	fields[0].dataType = BTable::INT16;
	fields[1].dataType = BTable::INT16;
	fields[2].dataType = BTable::INT16;
	fields[0].arraySize = 2;
	fields[1].arraySize = 2;
	fields[2].arraySize = 2;

	BTable(buffer).init(fields, 3, 1);
	EXPECT_EQ(*(uint32_t*)(buffer + 20), 0);
	EXPECT_EQ(*(uint32_t*)(buffer + 32), 4);
	EXPECT_EQ(*(uint32_t*)(buffer + 44), 8);
}

TEST(BTableTest, GetFieldByIndex)
{
	uint8_t buffer[128];
	BTable::FieldData fields[3];

	fields[0].name = "";
	fields[1].name = "";
	fields[2].name = "";

	BTable t(buffer);
	t.init(fields, 3, 0);
	EXPECT_EQ((void*)t.getField((uint32_t)0), (void*)(buffer + 16));
	EXPECT_EQ((void*)t.getField((uint32_t)1), (void*)(buffer + 16 + 12));
	EXPECT_EQ((void*)t.getField((uint32_t)2), (void*)(buffer + 16 + 24));
}

TEST(BTableTest, GetEntry)
{
	uint8_t buffer[128];
	BTable::FieldData fields[3];

	fields[0].name = "";
	fields[1].name = "";

	fields[0].arraySize = 1;
	fields[1].arraySize = 1;

	fields[0].dataType = BTable::DataType::INT8;
	fields[1].arraySize = BTable::DataType::INT8;

	BTable t(buffer);
	t.init(fields, 2, 1);
	EXPECT_EQ(t.getEntry(t.getField((uint32_t)0), 0), (void*)(buffer + 40));
	EXPECT_EQ(t.getEntry(t.getField((uint32_t)1), 0), (void*)(buffer + 40 + 1));

	fields[0].dataType = BTable::DataType::INT16;

	t.init(fields, 2, 1);
	EXPECT_EQ(t.getEntry(t.getField((uint32_t)0), 0), (void*)(buffer + 40));
	EXPECT_EQ(t.getEntry(t.getField((uint32_t)1), 0), (void*)(buffer + 40 + 2));

	fields[0].arraySize = 2;

	t.init(fields, 2, 1);
	EXPECT_EQ(t.getEntry(t.getField((uint32_t)0), 0), (void*)(buffer + 40));
	EXPECT_EQ(t.getEntry(t.getField((uint32_t)1), 0), (void*)(buffer + 40 + 4));

	fields[0].dataType = BTable::DataType::INT8;
	fields[0].arraySize = 1;

	t.init(fields, 2, 2);
	EXPECT_EQ(t.getEntry(t.getField((uint32_t)0), 0), (void*)(buffer + 40));
	EXPECT_EQ(t.getEntry(t.getField((uint32_t)1), 0), (void*)(buffer + 40 + 2));

	fields[0].dataType = BTable::DataType::INT16;

	t.init(fields, 2, 2);
	EXPECT_EQ(t.getEntry(t.getField((uint32_t)0), 0), (void*)(buffer + 40));
	EXPECT_EQ(t.getEntry(t.getField((uint32_t)1), 0), (void*)(buffer + 40 + 4));

	fields[0].arraySize = 2;

	t.init(fields, 2, 2);
	EXPECT_EQ(t.getEntry(t.getField((uint32_t)0), 0), (void*)(buffer + 40));
	EXPECT_EQ(t.getEntry(t.getField((uint32_t)1), 0), (void*)(buffer + 40 + 8));
}

TEST(BTableTest, GetEntries)
{
	uint8_t buffer[128];
	BTable::FieldData fields[3];

	fields[0].name = "";
	fields[1].name = "";

	fields[0].arraySize = 1;
	fields[1].arraySize = 1;

	fields[0].dataType = BTable::DataType::INT8;
	fields[1].arraySize = BTable::DataType::INT8;

	BTable t(buffer);
	t.init(fields, 2, 1);
	EXPECT_EQ(t.getEntries(t.getField((uint32_t)0)), (void*)(buffer + 40));
	EXPECT_EQ(t.getEntries(t.getField((uint32_t)1)), (void*)(buffer + 40 + 1));

	fields[0].dataType = BTable::DataType::INT16;

	t.init(fields, 2, 1);
	EXPECT_EQ(t.getEntries(t.getField((uint32_t)0)), (void*)(buffer + 40));
	EXPECT_EQ(t.getEntries(t.getField((uint32_t)1)), (void*)(buffer + 40 + 2));

	fields[0].arraySize = 2;

	t.init(fields, 2, 1);
	EXPECT_EQ(t.getEntries(t.getField((uint32_t)0)), (void*)(buffer + 40));
	EXPECT_EQ(t.getEntries(t.getField((uint32_t)1)), (void*)(buffer + 40 + 4));
}

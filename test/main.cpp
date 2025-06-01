#include "btable/btable.h"

#include <stdio.h>

int main(void)
{

	BTable::FieldData fields[2];

	fields[0].name = "test";
	fields[0].dataType = BTable::INT32;
	fields[0].arraySize = 0;

	fields[1].name = "jeff";
	fields[1].dataType = BTable::INT16;
	fields[1].arraySize = 0;

	int8_t* buffer = new int8_t[BTable::calculateBufferSize(fields, 2, 5)];
	BTable table(buffer, fields, 2, 5);
	table.setValueInt8(table.getField("jeff"), 2, 50);
	int8_t value = table.getEntry(table.getField("jeff"), 2).toInt8();

	printf("value: %d\n", value);
}

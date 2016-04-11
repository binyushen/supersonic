#include <iostream>
#include <vector>
#include "supersonic/supersonic.h"

#include "supersonic/utils/file.h"
#include "supersonic/cursor/infrastructure/file_io.h"
#include "supersonic/cursor/infrastructure/view_printer.h"
#include "supersonic/utils/basictypes.h"

#include <sstream>
using namespace std;

using namespace supersonic;

int main()
{
	const unsigned size = 1024 * 5 + 200;
	
	TupleSchema schema;
	schema.add_attribute(Attribute("str",STRING,NOT_NULLABLE));
	schema.add_attribute(Attribute("a",INT32,NOT_NULLABLE));
	schema.add_attribute(Attribute("b",DOUBLE,NOT_NULLABLE));

	Table table(schema, HeapBufferAllocator::Get());
	for(int i = 0; i < size; i++) {
		table.AddRow();
		stringstream ss;
		ss << "test" << i;
		table.Set<STRING>(0, i, ss.str());
		table.Set<INT32>(1, i, i);
		table.Set<DOUBLE>(2, i, i * 0.1);
	}
	table.RebuildColumnPieceVector();
	
	scoped_ptr<Operation> sco(ScanView(table.view()));
	scoped_ptr<Cursor> scan_cursor(SucceedOrDie(sco->CreateCursor()));
	
	ViewPrinter view_printer;

	Table table2(schema, HeapBufferAllocator::Get());
	vector<StorageType> storage_type_vector1({DISK, MEMORY, DISK});
	vector<StorageType> storage_type_vector2({MEMORY, DISK, MEMORY});

	vector<rowcount_t> ircv;
	ircv.push_back(size / 2 + 50);
	ircv.push_back(size / 2 + 50);
	ircv.push_back(size / 2 + 50);
	table2.ReserveRowCapacityOneTime(size, ircv);
	
	int i = 0;
	while(1) {
		ResultView result_view(scan_cursor->Next(rowGroupSize));
		if(result_view.has_data()) {
			table2.AppendView(result_view.view(),i++ % 2 ? storage_type_vector1 : storage_type_vector2);
		} else {
			break;
		}
	}
	table2.view().PrintViewColumnPieceInfo();
	// view_printer.AppendViewToStream(table2.view(), &cout);
	return 0;
}

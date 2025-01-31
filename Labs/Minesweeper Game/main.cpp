#include "field.h"

#include <QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	Field field;
	if (argc == 2 && strcmp(argv[1], "dbg") == 0)
	{
		field.dbg();
	}
	field.setWindowTitle("Minesweeper");
	field.setWindowIcon(QIcon(":/icons/icon.png"));
	field.show();
	return a.exec();
}

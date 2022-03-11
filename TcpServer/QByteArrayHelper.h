#pragma once

#include <qbytearray.h>

class QByteArrayHelper
{
public:
	QByteArrayHelper();
	~QByteArrayHelper();

	static QByteArray IntToQByteArray(int num);
	static int QByteArrayToInt(QByteArray array, int start);
};
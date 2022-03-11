#pragma execution_character_set("utf-8")
#include "QByteArrayHelper.h"

QByteArrayHelper::QByteArrayHelper()
{
}

QByteArrayHelper::~QByteArrayHelper()
{
}

QByteArray QByteArrayHelper::IntToQByteArray(int num)
{
	QByteArray buf;
	buf.resize(4);
	buf[0] = (uchar)(0x000000ff & num);
	buf[1] = (uchar)((0x0000ff00 & num) >> 8);
	buf[2] = (uchar)((0x00ff0000 & num) >> 16);
	buf[3] = (uchar)((0xff000000 & num) >> 24);
	return buf;
}

int QByteArrayHelper::QByteArrayToInt(QByteArray array, int start)
{
	array = array.mid(start, 4);
	int num = array[0] & 0x000000FF;
	num |= ((array[1] << 8) & 0x0000FF00);
	num |= ((array[2] << 16) & 0x00FF0000);
	num |= ((array[3] << 24) & 0xFF000000);
	return num;
}

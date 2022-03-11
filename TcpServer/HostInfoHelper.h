#pragma once

#include <QHostInfo>

class HostInfoHelper
{
public:
	HostInfoHelper();
	~HostInfoHelper();
	static HostInfoHelper* getInstance();

	QString localHostName();			//��ȡ������������
	QList<QString> localIPAddresses();	//��ȡ��������IP��ַ
	QString localIPv4();				//��ȡ����IPv4��ַ
	QString localIPv6();				//��ȡ����IPv6��ַ

private:
	static HostInfoHelper* instance;

};
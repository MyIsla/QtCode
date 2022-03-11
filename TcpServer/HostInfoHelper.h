#pragma once

#include <QHostInfo>

class HostInfoHelper
{
public:
	HostInfoHelper();
	~HostInfoHelper();
	static HostInfoHelper* getInstance();

	QString localHostName();			//获取本地主机名称
	QList<QString> localIPAddresses();	//获取本地所有IP地址
	QString localIPv4();				//获取本地IPv4地址
	QString localIPv6();				//获取本地IPv6地址

private:
	static HostInfoHelper* instance;

};
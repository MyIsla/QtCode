#pragma execution_character_set("utf-8")
#include "HostInfoHelper.h"

HostInfoHelper::HostInfoHelper()
{
}

HostInfoHelper::~HostInfoHelper()
{
}

HostInfoHelper* HostInfoHelper::instance = nullptr;

HostInfoHelper* HostInfoHelper::getInstance()
{
	if (instance == nullptr)
		instance = new HostInfoHelper();
	return instance;
}

/// <summary>
/// 获取本地主机名称
/// </summary>
/// <returns>本地主机名称</returns>
QString HostInfoHelper::localHostName()
{
	return QHostInfo::localHostName();
}

/// <summary>
/// 获取本地所有IP地址
/// </summary>
QList<QString> HostInfoHelper::localIPAddresses()
{
	QList<QString> ipAddresses;
	QString localName = localHostName();
	QHostInfo info = QHostInfo::fromName(localName);
	for each (QHostAddress address in info.addresses())
	{
		ipAddresses.append(address.toString());
	}
	return ipAddresses;
}

/// <summary>
/// 获取本地IPv4地址
/// </summary>
QString HostInfoHelper::localIPv4()
{
	QString ipAddress = "";
	QString localName = localHostName();
	QHostInfo info = QHostInfo::fromName(localName);
	for each (QHostAddress address in info.addresses())
	{
		if(address.protocol() == QAbstractSocket::IPv4Protocol)
			ipAddress = address.toString();
	}
	return ipAddress;
}

/// <summary>
/// 获取本地IPv6地址
/// </summary>
QString HostInfoHelper::localIPv6()
{
	QString ipAddress = "";
	QString localName = localHostName();
	QHostInfo info = QHostInfo::fromName(localName);
	for each (QHostAddress address in info.addresses())
	{
		if (address.protocol() == QAbstractSocket::IPv6Protocol)
			ipAddress = address.toString();
	}
	return ipAddress;
}



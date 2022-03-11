#pragma once

#include <QObject>
#include <QTimer>
#include <QDebug>

#define TCPDATA_TIMER_RUN 1	 //开启计时
#define TCPDATA_DATA_TIME 60 //数据定时器

/// 数据类型
/// 套接字规定头的格式为
/// type,filename,data,filesize,state,md5
/// 
/// type:数据类型 message,csv,png...
/// filename:文件名称
/// data:message中的消息
/// filesize:文件大小
/// state:0表示他发过来告诉我头,1表示我发过去的头他收到了准备发数据了，2表示我要发下一块的头
/// md5:校验
class TcpData : public QObject
{
	Q_OBJECT
public:
	QString type;				//数据类型 message,png,csv...
	QString fileName;			//文件名称/设备名称
	quint64 fileSize;			//数据整体大小
	quint64 nowSize = 0;		//目前总共收到/发送大小
	QByteArray buf = {};		//内容
	QString md5;				//md5每次更新
	  
	//构造函数
	TcpData();
	TcpData(QString type = "", QString fileName = "", quint64 fileSize = 0, QString md5 = "", quint64 nowSize = 0, QByteArray buf = {});
	//析构函数
	~TcpData();

#if TCPDATA_TIMER_RUN
public:
	void resetDataTime();	//重置计时
private:
	int dataTime;		//上次接收/发送文件的时间，秒为单位
	QTimer* timer;		//计时器，用于计算接收数据的时间，如果超过60秒没有数据更新，就把数据清空
private slots:
	void timesUp();		//timer触发的槽函数，用于定时删除没有进度的数据
signals:
	void clearData(TcpData* data);	//删除没有进度的数据信号
#endif // TCPDATA_TIMER_RUN

};
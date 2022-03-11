#pragma once

#include <QObject>
#include <QTcpServer>
#include <QList>
#include <qhostinfo.h>

#include "TcpThread.h"
#include "HostInfoHelper.h"

/// <summary>
/// 服务器，开启/关闭及确认客户端连接，属于一个TcpModel及界面的中转站
/// 界面通过TcpController控制Model，Model通过TcpController控制界面
/// </summary>
class TcpController : public QTcpServer
{
	Q_OBJECT
public:
	TcpController();
	~TcpController();

	static TcpController* getInstance();						//信号与槽连接调用该函数
public slots:
	//From View
	void startServer();											//开启服务器
	void startServer(QString addr, QString port);				//开启服务器
	void closeServer();											//关闭服务器
	void receMsgFromView(QString msg, QString id = "");			//接收到从界面来的消息（槽是为了分辨发给哪个客户端,空时为全发）
protected:
	void incomingConnection(qintptr socketDescriptor)override;	//重载客户端连接函数，将每个客户端放入一个线程中
private:
	static TcpController* instance;								//便于信号与槽的连接
	QList<TcpThread*> threadList;								//每个thread对应处理一个客户端的事件，用列表存储可以获得它们的标记以便于以后发送消息给单一的客户端
	void createConnections(TcpThread *thread);					//创建与thread的连接
private slots:
	//From Thread
	void deleteThread();										//删除断开连接的客户端
signals:
	//To Thread
	void sendMsgToThread(QString msg);							//发送消息给thread
	void sendFilePathToThread(QString filePath);				//发送文件路劲给Thread
	//To View
	void sendMsgToView(QString msg);							//发送消息给View
};


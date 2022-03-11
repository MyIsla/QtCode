#pragma once

#include <QThread>
#include <QObject>

#include "TcpModel.h"

/// <summary>
/// 线程，为了单独构造一个客户端对应一个线程
/// </summary>
class TcpThread : public QThread
{
	Q_OBJECT
public:
	TcpThread(qintptr socketDescriptor);
	~TcpThread();
protected:
	void run() override;
private:
	qintptr socketDescriptor;									//客户端套接字指针
	void createConnection(TcpModel* model);						//创建连接
signals:
	//To Model
	void sendMsgToModel(QString msg);							//发送信息Model
	void sendFilePathToModel(QString filePath);					//发送文件路径
	//To Controller
	void disconnected();
	void sendMsgToController(QString msg);						//发送信息Controller
};
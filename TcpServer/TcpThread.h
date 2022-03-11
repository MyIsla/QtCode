#pragma once

#include <QThread>
#include <QObject>

#include "TcpModel.h"

/// <summary>
/// �̣߳�Ϊ�˵�������һ���ͻ��˶�Ӧһ���߳�
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
	qintptr socketDescriptor;									//�ͻ����׽���ָ��
	void createConnection(TcpModel* model);						//��������
signals:
	//To Model
	void sendMsgToModel(QString msg);							//������ϢModel
	void sendFilePathToModel(QString filePath);					//�����ļ�·��
	//To Controller
	void disconnected();
	void sendMsgToController(QString msg);						//������ϢController
};
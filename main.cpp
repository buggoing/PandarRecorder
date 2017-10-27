#include "main.h"
#include "ui_MainWindow.h"
#include <QtGui/QFileDialog>
using namespace std;

MainWindow::MainWindow()
{
	Ui.setupUi(this);
	this->Ui.udpPortLineEdit->setText(QString::number(DEFAULT_UDP_LISTEN_PORT));
	this->Ui.gpsPortLineEdit->setText(QString::number(DEFAULT_GPS_LISTEN_PORT));	
	isRecording = false;

	pandarSource = PandarSource();
	pandarSource.start(DEFAULT_UDP_LISTEN_PORT, DEFAULT_GPS_LISTEN_PORT);

	connect(this->Ui.recordButton, SIGNAL(released()), this, SLOT(buttonRecordClicked()));
	connect(this->Ui.okPortButton, SIGNAL(released()), this, SLOT(buttonOKClicked()));		
	connect(&timerUpdateRate, SIGNAL(timeout()), this, SLOT(updateRate()));
	connect(&timerUpdateSavedData, SIGNAL(timeout()), this, SLOT(updateSavedData()));
	this->timerUpdateRate.start(1000);
	
	this->show();
	this->raise();
	this->activateWindow();
}

MainWindow::~MainWindow()
{
}

void MainWindow::buttonRecordClicked()
{
	if (!isRecording)
	{
		QString fileName;
		fileName = QFileDialog::getSaveFileName(
		this, tr("Save Pandar Pcap"), "~","Wireshark Capture (*.pcap)");
		if (fileName.isEmpty())
		{
			return;
		}
		this->Ui.filepathLabel->setText(fileName);
		this->Ui.recordButton->setText("recroding...");
		pandarSource.StartRecording(fileName.QString::toLocal8Bit().data());
		timerUpdateSavedData.start(1000);
		isRecording = true;
	}
	else
	{
		pandarSource.StopRecording();
		this->Ui.recordButton->setText("record");
		timerUpdateSavedData.stop();
		isRecording = false;
	}

}

void MainWindow::buttonOKClicked()
{
	unsigned int udpPort = this->Ui.udpPortLineEdit->text().toUInt();
	unsigned gpsPort = this->Ui.gpsPortLineEdit->text().toUInt();
	if (udpPort < MAX_UDP_PORT && gpsPort < MAX_UDP_PORT)
		pandarSource.start(udpPort, gpsPort);
}

void MainWindow::updateRate()
{
	unsigned int packetCounter = pandarSource.getAndUpdatePacketCounter();
	double rate = packetCounter * PACKET_SIZE / 1048576.0; // M/S, 1048576 = 1024 × 1024
	this->Ui.rateLabel->setText(QString::number(rate).left(5) + " M/S"); // keep 3 precision
}

void MainWindow::updateSavedData()
{
	uint64_t savedPacketCounter = pandarSource.getSavedPacketCounter();
	uint64_t saveDataSize = savedPacketCounter * PACKET_SIZE / 1048576; // M, PACKET_SIZE = 1282, 1282 bytes per packet except the gps packet(there is only one gps packet per second, so just ignore it), 1024 × 1024 = 1048576, 1282 / 1048576.0 = 0.0012226104736328125
	this->Ui.savedLabel->setText(QString::number(saveDataSize) + " M");
}


void MainWindow::closeEvent(QCloseEvent *event)
{
	if (isRecording)
	{
		pandarSource.StopRecording();
	}
}


int main(int argc , char**argv)
{
	initilizeTimer();	
	QApplication qapp(argc, argv);
	MainWindow mainWindow;
	mainWindow.setWindowTitle("PandarRecorder");
	mainWindow.setWindowIcon(QIcon("Logo.png"));
	return qapp.exec();
}
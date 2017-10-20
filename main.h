#include "ui_MainWindow.h"
#include "pandarSource.h"
#include <QTimer>


class MainWindow : public QMainWindow
{
	Q_OBJECT
	typedef QMainWindow Superclass;

protected:
	void closeEvent(QCloseEvent *event);

public:
	MainWindow();
	virtual ~MainWindow();
	void setupServer(unsigned int, unsigned int);
	Ui::MainWindow Ui;

private:
	Q_DISABLE_COPY(MainWindow);
	PandarSource pandarSource;
	bool isRecording;
	QTimer timerUpdateRate, timerUpdateSavedData;
public slots:
	void updateRate();
	void updateSavedData();
	void buttonRecordClicked();
	void buttonOKClicked();
};
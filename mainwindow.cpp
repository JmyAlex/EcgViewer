#include <QtGui>
#include "mainwindow.h"

#include "plotter.h"
#include "signal.h"

#include <vector>
#include <algorithm>

MainWindow::MainWindow()
{
    plotter = new Plotter;
    //plotter->clearCurve(2);
    setCentralWidget(plotter);
    //PlotSettings settings;
    //plotter->setPlotSettings(settings);

    createActions();
    createMenus();
    createToolbars();
    createStatusBar();

    setWindowIcon(QIcon(":/images/ecg.png"));
    setCurrentFile("");

    //signal = new Signal;
}

MainWindow::~MainWindow()
{

}

void MainWindow::createActions()
{
    openAction = new QAction(tr("&Open..."), this);
    openAction->setIcon(QIcon(":/images/open.png"));
    openAction->setShortcut(QKeySequence::Open);
    openAction->setStatusTip(tr("Open file"));
    connect(openAction, SIGNAL(triggered()), this, SLOT(open()));

    exitAction = new QAction(tr("E&xit"), this);
    exitAction->setShortcut(tr("Ctrl+Q"));
    exitAction->setStatusTip(tr("Exit the application"));
    connect(exitAction, SIGNAL(triggered()), this, SLOT(close()));

    /*showGridAction = new QAction(tr("&Show Grid"), this);             //TODO
    showGridAction->setCheckable(true);
    showGridAction->setChecked(spreadsheet->showGrid());
    showGridAction->setStatusTip(tr("Show or hide the spreadsheet's grid"));
    connect(showGridAction, SIGNAL(toggled(bool)), spreadsheet, SLOT(setShowGrid(bool)));*/

    aboutAction = new QAction(tr("&About"), this);
    aboutAction->setStatusTip(tr("Show the application's About box"));
    connect(aboutAction, SIGNAL(triggered()), this, SLOT(about()));

    aboutQtAction = new QAction(tr("About &Qt"), this);
    aboutQtAction->setStatusTip(tr("Show the Qt library's About box"));
    connect(aboutQtAction, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

    findWavesAction = new QAction(tr("&Find Waves"), this);
    findWavesAction->setIcon(QIcon(":/images/ecg1.png"));
    findWavesAction->setStatusTip(tr("Analise ECG"));
    connect(findWavesAction, SIGNAL(triggered()), this, SLOT(findWaves()));

    filterSignalAction = new QAction(tr("&Filter Signal"), this);
    filterSignalAction->setIcon(QIcon(":/images/filter.png"));
    filterSignalAction->setStatusTip(tr("Filter ECG signal"));
    connect(filterSignalAction, SIGNAL(triggered()), this, SLOT(filterSignal()));

    set100HzAction = new QAction(tr("&100 Hz"), this);
    set100HzAction->setStatusTip(tr("Set signal sample rate to 100 Hz"));
    connect(set100HzAction, SIGNAL(triggered()), this, SLOT(set100Hz()));
}

void MainWindow::createMenus()
{
    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(openAction);
    fileMenu->addAction(exitAction);

    operationsMenu = menuBar()->addMenu(tr("&Operations"));
    operationsMenu->addAction(findWavesAction);
    operationsMenu->addAction(filterSignalAction);
	operationsMenu->addAction(set100HzAction);

    menuBar()->addSeparator();

    helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(aboutAction);
    helpMenu->addAction(aboutQtAction);
}

void MainWindow::createToolbars()
{
    fileToolBar = addToolBar(tr("&File"));
    QMainWindow::addToolBar(Qt::TopToolBarArea, fileToolBar);
    fileToolBar->addAction(openAction);
    fileToolBar->addSeparator();
    fileToolBar->addAction(findWavesAction);
    fileToolBar->addAction(filterSignalAction);
    fileToolBar->setIconSize(QSize(20, 20));
}

void MainWindow::createStatusBar()
{
    sampleRateLabelText = new QLabel("Sample Rate");
    sampleRateLabelText->setAlignment(Qt::AlignHCenter);
    sampleRateLabelText->setMinimumSize(sampleRateLabelText->sizeHint());

    sampleRateLabelData = new QLabel(QString::number(0));

    sizeLabelText = new QLabel("Signal size");
    sizeLabelText->setAlignment(Qt::AlignHCenter);
    sizeLabelText->setMinimumSize(sampleRateLabelText->sizeHint());
    sizeLabelText->setIndent(3);

    sizeLabelData = new QLabel(QString::number(0));

//    progressBar = new QProgressBar;
//    //progressBar->minimumSize();
//    progressBar->setRange(0, 0);
//    progressBar->resize(100, 50);
//    progressBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
//    //progressBar->baseSize();
//    progressBar->setOrientation(Qt::Horizontal);
//    progressBar->setAlignment(Qt::AlignCenter);

    statusBar()->addWidget(sampleRateLabelText);
    statusBar()->addWidget(sampleRateLabelData);
    statusBar()->addWidget(sizeLabelText);
    statusBar()->addWidget(sizeLabelData, 1);

    //statusBar()->addWidget(progressBar);

    //connect(spreadsheet, SIGNAL(currentCellChanged(int, int, int, int)), this, SLOT(updateStatusBar()));
    //connect(spreadsheet, SIGNAL(modified()), this, SLOT(spreadsheetModified()));

    updateStatusBar();
}

void MainWindow::updateStatusBar()
{
    sampleRateLabelData->setText(QString::number(plotter->getSignalSampleRate()));
    sizeLabelData->setText(QString::number(plotter->getSignalSize()));
}

void MainWindow::open()
{
    QString selectedFilter;
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open file"), ".", tr("Ecg files (*.dat);;All files (*)"), &selectedFilter);

    if(!fileName.isEmpty())
        loadFile(fileName);
}

bool MainWindow::loadFile(const QString &fileName)
{
    if(!plotter->readFile(fileName))
    {
        statusBar()->showMessage(tr("Loading canceled"), 2000);
        return false;
    }
    setCurrentFile(fileName);
    statusBar()->showMessage(tr("File loaded"), 2000);

    updateStatusBar();
    return true;
}

void MainWindow::setCurrentFile(const QString &fileName)
{
    curFile = fileName;

    QString showName = "Untitled";
    if(!curFile.isEmpty())
    {
        showName = strippedName(curFile);
    }

    setWindowTitle(tr("%1[*] - %2").arg(showName).arg(tr("EcgTest")));
}

QString MainWindow::strippedName(const QString &fullFileName)
{
    return QFileInfo(fullFileName).fileName();
}

void MainWindow::about()
{
    QMessageBox::about(this, tr("About EcgTest"), tr("<h2>Ecg Viewer 0.0</h2>"
                                                 "<p> Copyright &copy; Alexander Solovov ^_^ 2011"
                                                 "<p> Ecg test viewer"
                                                 "<p> <h2>Help</h2>"
                                                 "<p> Navigating on signal : keyboard arrows(up, down, left, right)"
                                                 "<p> Zoom in '+', zoom out '-'"));
}

void MainWindow::findWaves()
{
	plotter->setAnnotationData();
}

void MainWindow::filterSignal()
{
    plotter->setFilterData();
}

void MainWindow::set100Hz()
{
	plotter->setSignalSampleRate();
	updateStatusBar();
}

//void MainWindow::changeEvent(QEvent *e)
//{
//    QMainWindow::changeEvent(e);
//    switch (e->type()) {
//    case QEvent::LanguageChange:
//        ui->retranslateUi(this);
//        break;
//    default:
//        break;
//    }
//}

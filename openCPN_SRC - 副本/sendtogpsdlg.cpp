#include "sendtogpsdlg.h"
#include "ui_sendtogpsdlg.h"
#include "Route.h"
#include "RoutePoint.h"
#include "chart1.h"
#include <QVBoxLayout>
#include <QHBoxLayout>


extern QString g_uploadConnection;


SendToGpsDlg::SendToGpsDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SendToGpsDlg)
{
    ui->setupUi(this);
    m_itemCommListBox = NULL;
    m_pgauge = NULL;
    m_SendButton = NULL;
    m_CancelButton = NULL;
    m_pRoute = NULL;
    m_pRoutePoint = NULL;
    Create();
}

SendToGpsDlg::~SendToGpsDlg()
{
    delete ui;
}


SendToGpsDlg::SendToGpsDlg(const QString& caption, const QString& hint, const QSize& size, long style, QWidget* parent) :
    QDialog(parent),
    ui(new Ui::SendToGpsDlg)
{
    ui->setupUi(this);
    m_itemCommListBox = NULL;
    m_pgauge = NULL;
    m_SendButton = NULL;
    m_CancelButton = NULL;
    m_pRoute = NULL;
    m_pRoutePoint = NULL;
    Create( caption, hint, size, style );
}

bool SendToGpsDlg::Create( const QString& caption, const QString& hint, const QSize& size, long style)
{
    this->setWindowFlags(style);
    this->setWindowTitle(caption);
    this->setFixedSize(size);

    QVBoxLayout *total_layout = new QVBoxLayout(this);
    total_layout->setSpacing(20);
    this->setLayout(total_layout);

    QHBoxLayout *com_layout = new QHBoxLayout(this);
    total_layout->addLayout(com_layout);
    com_layout->addWidget(new QLabel(tr("GPS/Plotter Port"), this);
    QStringList *pSerialArray = EnumerateSerialPorts();
    m_itemCommListBox = new  QComboBox( this);
    com_layout->addWidget(m_itemCommListBox);

    //    Fill in the listbox with all detected serial ports
    if(pSerialArray )
    {
        for( unsigned int iPortIndex = 0; iPortIndex < pSerialArray->count(); iPortIndex++ ) {
            QString full_port = tr("Serial:") + pSerialArray->at(iPortIndex);
            m_itemCommListBox->addItem(full_port);
        }
        delete pSerialArray;
    }

    //    Make the proper inital selection
    if( !g_uploadConnection.isEmpty() )
    {
        m_itemCommListBox->setCurrentText(g_uploadConnection );
    } else
    {
        m_itemCommListBox->setCurrentIndex(0);
    }

    total_layout->addWidget( new QLabel(tr("Prepare GPS for Route/Waypoint upload and press Send..."), this ));

    QHBoxLayout *process_layout = new QHBoxLayout(this);
    total_layout->addLayout(process_layout);
    process_layout->addWidget(new QLabel(tr("Progress..."), this);
    m_pgauge = new QProgressBar( this);
    m_pgauge->setOrientation(Qt::Horizontal);
    m_pgauge->setRange(0, 100);
    m_pgauge->setValue(-1);


    QHBoxLayout *btn_layout = new QHBoxLayout(this);
    total_layout->addLayout(btn_layout);
    m_CancelButton = new QPushButton(tr("Cancel"), this);
    m_SendButton = new QPushButton(tr("Send"), this);
    btn_layout->addWidget(m_CancelButton);
    btn_layout->addWidget(m_SendButton);
    connect(m_CancelButton, SIGNAL(clicked(bool)), this, SLOT(OnCancelClick()));
    connect(m_SendButton, SIGNAL(clicked(bool)), this, SLOT(OnSendClick()));
    m_SendButton->setDefault(true);

}

void SendToGpsDlg::OnSendClick()
{
    //    Get the selected comm port
    QString src = m_itemCommListBox->currentText();
    g_uploadConnection = src;                   // save for persistence
    //    And send it out
    if( m_pRoute ) m_pRoute->SendToGPS( src, true, m_pgauge );
    if( m_pRoutePoint ) m_pRoutePoint->SendToGPS( src, m_pgauge );
    close();
}

void SendToGpsDlg::OnCancelClick()
{
    close();
}




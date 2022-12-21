#include <QFileDialog>
#include <QPainter>

#include "mainframe.h"
#include "ui_mainframe.h"
#include "imageform.h"


MainFrame::MainFrame(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MainFrame)
{
    ui->setupUi(this);

    _plpImageForm       = new KPtrList<ImageForm*>(10,false,false);
    _q_pFormFocused     = 0;

    //객체 맴버의 초기화


    //get a current directory
    char st[100];
    GetCurrentDirectoryA(100,st);

    //리스트 출력창을 안보이게
    ui->listWidget->setVisible(false);
    this->adjustSize();

    //UI 활성화 갱신
    UpdateUI();
}

MainFrame::~MainFrame()
{ 
    delete ui;         
    delete _plpImageForm;   
}

void MainFrame::Detach(ImageForm *pForm)
{    
    //ImageForm 포인터 삭제
    _plpImageForm->Remove(pForm);

    //활성화 ImageForm 초기화
    _q_pFormFocused     = 0;

    //UI 활성화 갱신
    UpdateUI();
}

void MainFrame::UpdateUI()
{
    if(ui->tabWidget->currentIndex() == 0) // hw 01-02
    {
        //ui->buttonSepiaTone->setEnabled( _q_pFormFocused &&  _q_pFormFocused->ID() == "OPEN" );

    }

}

void MainFrame::OnMousePos(const int &nX, const int &nY, ImageForm* q_pForm)
{
    UpdateUI();
}

void MainFrame::closeEvent(QCloseEvent* event)
{
    //생성된 ImageForm을 닫는다.
    for(int i=_plpImageForm->Count()-1; i>=0; i--)
        _plpImageForm->Item(i)->close();
}


void MainFrame::on_buttonOpen_clicked()
{
    //이미지 파일 선택
    QFileDialog::Options    q_Options   =  QFileDialog::DontResolveSymlinks  | QFileDialog::DontUseNativeDialog; // | QFileDialog::ShowDirsOnly
    QString                 q_stFile    =  QFileDialog::getOpenFileName(this, tr("Select a Image File"),  "./data", "Image Files(*.bmp *.ppm *.pgm *.png)",0, q_Options);

    if(q_stFile.length() == 0)
        return;

    //이미지 출력을 위한 ImageForm 생성    
    ImageForm*              q_pForm   = new ImageForm(q_stFile, "OPEN", this);

    _plpImageForm->Add(q_pForm);
    q_pForm->show();

    //UI 활성화 갱신
    UpdateUI();
}

void MainFrame::on_buttonDeleteContents_clicked()
{
    //생성된 ImageForm을 닫는다.
    for(int i=_plpImageForm->Count()-1; i>=0; i--)
        _plpImageForm->Item(i)->close();

    //리스트에서 삭제한다.
    _plpImageForm->RemoveAll();
}

void MainFrame::on_buttonShowList_clicked()
{
    static int nWidthOld = ui->tabWidget->width();

    if(ui->listWidget->isVisible())
    {
        nWidthOld = ui->listWidget->width();
        ui->listWidget->hide();
        this->adjustSize();
    }
    else
    {        
        ui->listWidget->show();
        QRect q_rcWin = this->geometry();

        this->setGeometry(q_rcWin.left(), q_rcWin.top(), q_rcWin.width()+nWidthOld, q_rcWin.height());
    }
}

/*
 * void MainFrame::on_buttonSepiaTone_clicked()
{
    //포커스 된 ImageForm으로부터 영상을 가져옴
    KImageColor   icMain;
    if(_q_pFormFocused != 0 && _q_pFormFocused->ImageColor().Address() &&  _q_pFormFocused->ID() == "OPEN")
        icMain = _q_pFormFocused->ImageColor();
    else
        return;

    //
    double  dHue = ui->spinHue->value();
    double  dSat = ui->spinSaturation->value();

    //출력 영상
    KImageColor icSepia(icMain.Row(), icMain.Col());
    //  :
    //  :


    //출력을 위한 ImageForm 생성
    ImageForm*  q_pForm = new ImageForm(icSepia, "Sepia Tone", this);

    _plpImageForm->Add(q_pForm);
    q_pForm->show();

    //UI 활성화 갱신
    UpdateUI();
}
*/

void MainFrame::on_ButtonThres_clicked()
{
    //포커스 된 ImageForm으로부터 영상을 가져옴
    KImageGray   igMain;
    if(_q_pFormFocused != 0 && _q_pFormFocused->ImageGray().Address() &&  _q_pFormFocused->ID() == "OPEN")
        igMain = _q_pFormFocused->ImageGray();
    else
        return;

    double  dT = ui->spinT->value();
    KImageGray igOut(igMain.Row(), igMain.Col());

    for(int i = 0; i<igMain.Row(); i++){
        for (int j = 0; j<igMain.Col(); j++) {
            if (igMain[i][j] >= dT){
                igOut[i][j] = 255;
            }
            else{
                igOut[i][j] = 0;
            }
        }
    }

    //출력을 위한 ImageForm 생성
    ImageForm*  q_pForm = new ImageForm(igOut, "Thresholding accoring to input value", this);

    _plpImageForm->Add(q_pForm);
    q_pForm->show();

    //UI 활성화 갱신
    UpdateUI();
}

void MainFrame::on_ButtonOtsu_clicked()
{
    //포커스 된 ImageForm으로부터 영상을 가져옴
    KImageGray   igMain;
    if(_q_pFormFocused != 0 && _q_pFormFocused->ImageGray().Address() &&  _q_pFormFocused->ID() == "OPEN")
        igMain = _q_pFormFocused->ImageGray();
    else
        return;

    //Otsu's Thresholding
    KImageGray  img(igMain.Row(), igMain.Col());
    KHisto hTmp;
    hTmp.Histogram(igMain);

    #define MAX_PIXEL_VAL 255

    double p[256], q1[256], myu1[256], myu2[256], sigmab[256];
    double myug = 0.0, T = 0.0, maxsigma = 0.0;
    q1[0] = hTmp[0]/igMain.Size();
    myu1[0] = 0;

    for(unsigned int i = 0;i < MAX_PIXEL_VAL;i++){
        p[i] = hTmp[i] / igMain.Size();
        myug += i*p[i];
    }

    for(int t = 0;t < 254;t++){
        q1[t+1] = q1[t]+p[t+1];

        if(q1[t+1] == 0.0)
        {
            myu1[t+1] = 0.0;
            continue;
        }

        myu1[t+1] = ((q1[t]*myu1[t]) + (double)(t+1)*p[t+1])/q1[t+1];
        myu2[t+1] = (myug - q1[t+1]*myu1[t+1]) / (1.0 - q1[t+1]);
    }

    for(int tt=0; tt < MAX_PIXEL_VAL; tt++){
        sigmab[tt] = q1[tt]*(1.0-q1[tt])*(pow(myu1[tt]-myu2[tt],2));

        if (sigmab[tt] > maxsigma){
            maxsigma = sigmab[tt];
            T = tt;
        }
    }

    for(unsigned int i=0; i < img.Row(); i++){
        for(unsigned int j=0; j < img.Col(); j++){
            if(igMain[i][j] < T){
                img[i][j] = 0;
            }
            else{
                img[i][j] = MAX_PIXEL_VAL;
            }
        }
    }
    KImageGray  igOut = img;

    //출력을 위한 ImageForm 생성
    ImageForm*  q_pForm = new ImageForm(igOut, "Otsu's Thresholding", this);

    _plpImageForm->Add(q_pForm);
    q_pForm->show();

    //UI 활성화 갱신
    UpdateUI();
}

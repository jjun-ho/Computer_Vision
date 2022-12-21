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
        ui->buttonSepiaTone->setEnabled( _q_pFormFocused &&  _q_pFormFocused->ID() == "OPEN" );
        ui->buttonHue->setEnabled( _q_pFormFocused &&  _q_pFormFocused->ID() == "OPEN" );
        ui->buttonSaturation->setEnabled( _q_pFormFocused &&  _q_pFormFocused->ID() == "OPEN" );
        ui->buttonValue->setEnabled( _q_pFormFocused &&  _q_pFormFocused->ID() == "OPEN" );
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

void MainFrame::on_buttonSepiaTone_clicked()
{
    //포커스 된 ImageForm으로부터 영상을 가져옴
    KImageColor   icMain;
    if(_q_pFormFocused != 0 && _q_pFormFocused->ImageColor().Address() &&  _q_pFormFocused->ID() == "OPEN")
        icMain = _q_pFormFocused->ImageColor();
    else
        return;

    //hue, saturation 깂 가져오기
    double  dHue = ui->spinHue->value();
    double  dSat = ui->spinSaturation->value();

    //출력영상 구함
    //   :
    KImageColor icSepia(icMain.Row(), icMain.Col());
    KArray<KHSV> xHSV;
    icMain.RGBtoHSV(xHSV);
    for(int i = 0; i<xHSV.Row(); i++){
        for (int j = 0; j<xHSV.Col(); j++) {
            xHSV[i][j].h = dHue;
            xHSV[i][j].s = dSat;
        }
    }
    icSepia = icMain.FromHSV(xHSV);
    //   :

    //출력을 위한 ImageForm 생성
    ImageForm*  q_pForm = new ImageForm(icSepia, "Sepia Tone", this);

    _plpImageForm->Add(q_pForm);
    q_pForm->show();

    //UI 활성화 갱신
    UpdateUI();
}


void MainFrame::on_buttonHue_clicked()
{
    //포커스 된 ImageForm으로부터 영상을 가져옴
        KImageColor   icMain;
        if(_q_pFormFocused != 0 && _q_pFormFocused->ImageColor().Address() &&  _q_pFormFocused->ID() == "OPEN")
            icMain = _q_pFormFocused->ImageColor();
        else
            return;

        //출력영상 구함
        KImageGray igHue(icMain.Row(), icMain.Col());
        //   :
        KArray<KHSV> xHSV;
        icMain.RGBtoHSV(xHSV);
        for(int i = 0; i<xHSV.Row(); i++){
            for (int j = 0; j<xHSV.Col(); j++) {
                igHue[i][j] = (unsigned char)(xHSV[i][j].h / 360.0 * 255.0);
            }
        }
        //   :

        //출력을 위한 ImageForm 생성
        ImageForm*  q_pForm = new ImageForm(igHue, "Hue Image", this);

        _plpImageForm->Add(q_pForm);
        q_pForm->show();

        //UI 활성화 갱신
        UpdateUI();
}


void MainFrame::on_buttonSaturation_clicked()
{
    //포커스 된 ImageForm으로부터 영상을 가져옴
        KImageColor   icMain;
        if(_q_pFormFocused != 0 && _q_pFormFocused->ImageColor().Address() &&  _q_pFormFocused->ID() == "OPEN")
            icMain = _q_pFormFocused->ImageColor();
        else
            return;

        //출력영상 구함
        KImageGray igSat(icMain.Row(), icMain.Col());
        //   :
        KArray<KHSV> xHSV;
        icMain.RGBtoHSV(xHSV);
        for(int i = 0; i<xHSV.Row(); i++){
            for (int j = 0; j<xHSV.Col(); j++) {
                igSat[i][j] = (unsigned char)(xHSV[i][j].s * 255.0);
            }
        }
        //   :

        //출력을 위한 ImageForm 생성
        ImageForm*  q_pForm = new ImageForm(igSat, "Saturation Image", this);

        _plpImageForm->Add(q_pForm);
        q_pForm->show();

        //UI 활성화 갱신
        UpdateUI();
}


void MainFrame::on_buttonValue_clicked()
{
    //포커스 된 ImageForm으로부터 영상을 가져옴
        KImageColor   icMain;
        if(_q_pFormFocused != 0 && _q_pFormFocused->ImageColor().Address() &&  _q_pFormFocused->ID() == "OPEN")
            icMain = _q_pFormFocused->ImageColor();
        else
            return;

        //출력영상 구함
        KImageGray igVal(icMain.Row(), icMain.Col());
        //   :
        KArray<KHSV> xHSV;
        icMain.RGBtoHSV(xHSV);
        for(int i = 0; i<xHSV.Row(); i++){
            for (int j = 0; j<xHSV.Col(); j++) {
                igVal[i][j] = xHSV[i][j].v;
            }
        }
        //   :

        //출력을 위한 ImageForm 생성
        ImageForm*  q_pForm = new ImageForm(igVal, "Value Image", this);

        _plpImageForm->Add(q_pForm);
        q_pForm->show();

        //UI 활성화 갱신
        UpdateUI();
}




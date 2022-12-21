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

void MainFrame::on_Button3Dilated_clicked()
{
    //포커스 된 ImageForm으로부터 영상을 가져옴
    KImageGray   igMain;
    if(_q_pFormFocused != 0 && _q_pFormFocused->ImageGray().Address() &&  _q_pFormFocused->ID() == "OPEN")
        igMain = _q_pFormFocused->ImageGray();
    else
        return;

    //Otsu's Thresholding
    KImageGray img(igMain.Row(), igMain.Col());
    KHisto  otsu;
    otsu.Ostu(&igMain, &img);

    KImageGray igOtsu = img;

    //Dilation
    KImageGray  igOut = img;
    for(int i=1,ii=img.Row()-2; ii; i++,ii--)
        for(int j=1,jj=img.Col()-2; jj; j++,jj--)
        {
            if(img[i][j] == 0)
                if(img[i-1][j-1] || img[i-1][j] || img[i-1][j+1] || img[i][j-1]
                        || img[i][j+1] || img[i+1][j-1] || img[i+1][j] || img[i+1][j+1])
                    igOut[i][j] = 255;
        }

    //Labeling
    const int ROW = igOut.Row();
    const int COL = igOut.Col();

    KImageGray igtrans(igOut.Row(), igOut.Col());

    int label = 0;

    for(int i=1 ; i<ROW-1 ; i++){
        for(int j=1 ; j<COL-1 ; j++){
            if(igOut[i][j]==255 && igtrans[i][j]==0)
            {
                ++label;
                igtrans[i][j] = label;
                Labeling4Neighbor(igOut, igtrans, label, i, j, ROW, COL);
            }
        }
    }

    KImageColor icOut(igtrans.Row(),igtrans.Col());

    for(int i=0 ; i<ROW ; i++){
        for(int j=0 ; j<COL ; j++){
            if(igOut[i][j]==0){
                icOut[i][j].r = 0;
                icOut[i][j].g = 0;
                icOut[i][j].b = 0;
            }
            else{
                switch ((igtrans[i][j]%4)+1) {
                case 1:
                    icOut[i][j].r = 255;
                    icOut[i][j].g = 255;
                    icOut[i][j].b = 0;
                    break;
                case 2:
                    icOut[i][j].r = 255;
                    icOut[i][j].g = 0;
                    icOut[i][j].b = 0;
                    break;
                case 3:
                    icOut[i][j].r = 0;
                    icOut[i][j].g = 255;
                    icOut[i][j].b = 0;
                    break;
                case 4:
                    icOut[i][j].r = 0;
                    icOut[i][j].g = 0;
                    icOut[i][j].b = 255;
                    break;
                default:
                    break;
                }
            }
        }
    }

    //출력을 위한 ImageForm 생성
    ImageForm*  q_pForm1 = new ImageForm(igOtsu, "Otsu's Image", this);
    ImageForm*  q_pForm2 = new ImageForm(igOut, "3*3 Dilated Image", this);
    ImageForm*  q_pForm3 = new ImageForm(icOut, "Labeling", this);

    _plpImageForm->Add(q_pForm1);
    _plpImageForm->Add(q_pForm2);
    _plpImageForm->Add(q_pForm3);

    q_pForm1->show();
    q_pForm2->show();
    q_pForm3->show();

    //UI 활성화 갱신
    UpdateUI();
}

void MainFrame::on_Button5Dilated_clicked()
{
    //포커스 된 ImageForm으로부터 영상을 가져옴
    KImageGray   igMain;
    if(_q_pFormFocused != 0 && _q_pFormFocused->ImageGray().Address() &&  _q_pFormFocused->ID() == "OPEN")
        igMain = _q_pFormFocused->ImageGray();
    else
        return;

    //Otsu's Thresholding
    KImageGray img(igMain.Row(), igMain.Col());
    KHisto  otsu;
    otsu.Ostu(&igMain, &img);

    KImageGray igOtsu = img;

    //Dilation
    KImageGray  igOut = img;
    for(int i=2,ii=img.Row()-4; ii; i++,ii--)
        for(int j=2,jj=img.Col()-4; jj; j++,jj--)
        {
            if(img[i][j] == 0)
                if(img[i-2][j-2] || img[i-2][j-1] || img[i-2][j] || img[i-2][j+1] || img[i-2][j+2]
                        || img[i-1][j-2] || img[i-1][j-1] || img[i-1][j] || img[i-1][j+1] || img[i-1][1+2]
                        || img[i][j-2] || img[i][j-1] || img[i][j+1] || img[i][j+2]
                        || img[i+1][j-2] || img[i+1][j-1] || img[i+1][j] || img[i+1][j+1] || img[i+1][j+2]
                        || img[i+2][j-2] || img[i+2][j-1] || img[i+2][j] || img[i+2][j+1] || img[i+2][j+2])
                    igOut[i][j] = 255;
        }
    /*
    //Labeling
    const int ROW = igOut.Row();
    const int COL = igOut.Col();

    KImageGray igtrans(igOut.Row(), igOut.Col());

    int label = 0;

    for(int i=1 ; i<ROW-1 ; i++){
        for(int j=1 ; j<COL-1 ; j++){
            if(igOut[i][j]==255 && igtrans[i][j]==0)
            {
                ++label;
                igtrans[i][j] = label;
                Labeling4Neighbor(igOut, igtrans, label, i, j, ROW, COL);
            }
        }
    }

    KImageColor icOut(igtrans.Row(),igtrans.Col());

    for(int i=0 ; i<ROW ; i++){
        for(int j=0 ; j<COL ; j++){
            if(igOut[i][j]==0){
                icOut[i][j].r = 0;
                icOut[i][j].g = 0;
                icOut[i][j].b = 0;
            }
            else{
                switch ((igtrans[i][j]%4)+1) {
                case 1:
                    icOut[i][j].r = 255;
                    icOut[i][j].g = 255;
                    icOut[i][j].b = 0;
                    break;
                case 2:
                    icOut[i][j].r = 255;
                    icOut[i][j].g = 0;
                    icOut[i][j].b = 0;
                    break;
                case 3:
                    icOut[i][j].r = 0;
                    icOut[i][j].g = 255;
                    icOut[i][j].b = 0;
                    break;
                case 4:
                    icOut[i][j].r = 0;
                    icOut[i][j].g = 0;
                    icOut[i][j].b = 255;
                    break;
                default:
                    break;
                }
            }
        }
    }
    */
    //출력을 위한 ImageForm 생성
    ImageForm*  q_pForm1 = new ImageForm(igOtsu, "Otsu's Image", this);
    ImageForm*  q_pForm2 = new ImageForm(igOut, "5*5 Dilated Image", this);
    //ImageForm*  q_pForm3 = new ImageForm(icOut, "Labeling", this);

    _plpImageForm->Add(q_pForm1);
    _plpImageForm->Add(q_pForm2);
    //_plpImageForm->Add(q_pForm3);

    q_pForm1->show();
    q_pForm2->show();
    //q_pForm3->show();

    //UI 활성화 갱신
    UpdateUI();
}

void MainFrame::on_Button3Eroded_clicked()
{
    //포커스 된 ImageForm으로부터 영상을 가져옴
    KImageGray   igMain;
    if(_q_pFormFocused != 0 && _q_pFormFocused->ImageGray().Address() &&  _q_pFormFocused->ID() == "OPEN")
        igMain = _q_pFormFocused->ImageGray();
    else
        return;

    //Otsu's Thresholding
    KImageGray img(igMain.Row(), igMain.Col());
    KHisto  otsu;
    otsu.Ostu(&igMain, &img);

    KImageGray igOtsu = img;

    //Erosion
    KImageGray  igOut = img;
    for(int i=1,ii=img.Row()-2; ii; i++,ii--)
        for(int j=1,jj=img.Col()-2; jj; j++,jj--)
        {
            if(img[i][j])
                if(img[i-1][j-1]==0 || img[i-1][j]==0 || img[i-1][j+1==0] || img[i][j-1]==0
                        || img[i][j+1]==0 || img[i+1][j-1]==0 || img[i+1][j]==0 || img[i+1][j+1]==0)
                    igOut[i][j] = 0;
        }

    //Labeling
    const int ROW = igOut.Row();
    const int COL = igOut.Col();

    KImageGray igtrans(igOut.Row(), igOut.Col());

    int label = 0;

    for(int i=1 ; i<ROW-1 ; i++){
        for(int j=1 ; j<COL-1 ; j++){
            if(igOut[i][j]==255 && igtrans[i][j]==0)
            {
                ++label;
                igtrans[i][j] = label;
                Labeling4Neighbor(igOut, igtrans, label, i, j, ROW, COL);
            }
        }
    }

    KImageColor icOut(igtrans.Row(),igtrans.Col());

    for(int i=0 ; i<ROW ; i++){
        for(int j=0 ; j<COL ; j++){
            if(igOut[i][j]==0){
                icOut[i][j].r = 0;
                icOut[i][j].g = 0;
                icOut[i][j].b = 0;
            }
            else{
                switch ((igtrans[i][j]%4)+1) {
                case 1:
                    icOut[i][j].r = 255;
                    icOut[i][j].g = 255;
                    icOut[i][j].b = 0;
                    break;
                case 2:
                    icOut[i][j].r = 255;
                    icOut[i][j].g = 0;
                    icOut[i][j].b = 0;
                    break;
                case 3:
                    icOut[i][j].r = 0;
                    icOut[i][j].g = 255;
                    icOut[i][j].b = 0;
                    break;
                case 4:
                    icOut[i][j].r = 0;
                    icOut[i][j].g = 0;
                    icOut[i][j].b = 255;
                    break;
                default:
                    break;
                }
            }
        }
    }

    //출력을 위한 ImageForm 생성
    ImageForm*  q_pForm1 = new ImageForm(igOtsu, "Otsu's Image", this);
    ImageForm*  q_pForm2 = new ImageForm(igOut, "3*3 Erode Image", this);
    ImageForm*  q_pForm3 = new ImageForm(icOut, "Labeling", this);

    _plpImageForm->Add(q_pForm1);
    _plpImageForm->Add(q_pForm2);
    _plpImageForm->Add(q_pForm3);

    q_pForm1->show();
    q_pForm2->show();
    q_pForm3->show();

    //UI 활성화 갱신
    UpdateUI();
}

void MainFrame::on_Button5Eroded_clicked()
{
    //포커스 된 ImageForm으로부터 영상을 가져옴
    KImageGray   igMain;
    if(_q_pFormFocused != 0 && _q_pFormFocused->ImageGray().Address() &&  _q_pFormFocused->ID() == "OPEN")
        igMain = _q_pFormFocused->ImageGray();
    else
        return;

    //Otsu's Thresholding
    KImageGray img(igMain.Row(), igMain.Col());
    KHisto  otsu;
    otsu.Ostu(&igMain, &img);

    KImageGray igOtsu = img;

    //Erosion
    KImageGray  igOut = img;
    for(int i=2,ii=img.Row()-4; ii; i++,ii--)
        for(int j=2,jj=img.Col()-4; jj; j++,jj--)
        {
            if(img[i-2][j-2]==0 || img[i-2][j-1]==0 || img[i-2][j]==0 || img[i-2][j+1]==0 || img[i-2][j+2]==0
                    || img[i-1][j-2]==0 || img[i-1][j-1]==0 || img[i-1][j]==0 || img[i-1][j+1]==0 || img[i-1][1+2]==0
                    || img[i][j-2]==0 || img[i][j-1]==0 || img[i][j+1]==0 || img[i][j+2]==0
                    || img[i+1][j-2]==0 || img[i+1][j-1]==0 || img[i+1][j]==0 || img[i+1][j+1]==0 || img[i+1][j+2]==0
                    || img[i+2][j-2]==0 || img[i+2][j-1]==0 || img[i+2][j]==0 || img[i+2][j+1]==0 || img[i+2][j+2]==0)
                    igOut[i][j] = 0;
        }

    //Labeling
    const int ROW = igOut.Row();
    const int COL = igOut.Col();

    KImageGray igtrans(igOut.Row(), igOut.Col());

    int label = 0;

    for(int i=1 ; i<ROW-1 ; i++){
        for(int j=1 ; j<COL-1 ; j++){
            if(igOut[i][j]==255 && igtrans[i][j]==0)
            {
                ++label;
                igtrans[i][j] = label;
                Labeling4Neighbor(igOut, igtrans, label, i, j, ROW, COL);
            }
        }
    }

    KImageColor icOut(igtrans.Row(),igtrans.Col());

    for(int i=0 ; i<ROW ; i++){
        for(int j=0 ; j<COL ; j++){
            if(igOut[i][j]==0){
                icOut[i][j].r = 0;
                icOut[i][j].g = 0;
                icOut[i][j].b = 0;
            }
            else{
                switch ((igtrans[i][j]%4)+1) {
                case 1:
                    icOut[i][j].r = 255;
                    icOut[i][j].g = 255;
                    icOut[i][j].b = 0;
                    break;
                case 2:
                    icOut[i][j].r = 255;
                    icOut[i][j].g = 0;
                    icOut[i][j].b = 0;
                    break;
                case 3:
                    icOut[i][j].r = 0;
                    icOut[i][j].g = 255;
                    icOut[i][j].b = 0;
                    break;
                case 4:
                    icOut[i][j].r = 0;
                    icOut[i][j].g = 0;
                    icOut[i][j].b = 255;
                    break;
                default:
                    break;
                }
            }
        }
    }

    //출력을 위한 ImageForm 생성
    ImageForm*  q_pForm1 = new ImageForm(igOtsu, "Otsu's Image", this);
    ImageForm*  q_pForm2 = new ImageForm(igOut, "5*5 Erode Image", this);
    ImageForm*  q_pForm3 = new ImageForm(icOut, "Labeling", this);

    _plpImageForm->Add(q_pForm1);
    _plpImageForm->Add(q_pForm2);
    _plpImageForm->Add(q_pForm3);

    q_pForm1->show();
    q_pForm2->show();
    q_pForm3->show();

    //UI 활성화 갱신
    UpdateUI();
}


void MainFrame::Labeling4Neighbor(KImageGray& imgray, KImageGray& imgray2, int label, int row, int col, int Max_row, int Max_col){
    if((row-1<1||row-1>=Max_row)||(col<1||col>=Max_col)){
    }else{ //위
        if(imgray[row-1][col]==255 && imgray2[row-1][col]==0){
            imgray2[row-1][col] = label;
            Labeling4Neighbor(imgray, imgray2, label, row-1, col, Max_row, Max_col);
        }
    }
    if((row<1||row>=Max_row)||(col-1<1||col-1>=Max_col)){
    }else{ //왼쪽
        if(imgray[row][col-1]==255 && imgray2[row][col-1]==0){
            imgray2[row][col-1] = label;
            Labeling4Neighbor(imgray, imgray2, label, row, col-1, Max_row, Max_col);
        }
    }
    if((row+1<1||row+1>=Max_row)||(col<1||col>=Max_col)){
    }else{ //아래
        if(imgray[row+1][col]==255 && imgray2[row+1][col]==0){
            imgray2[row+1][col] = label;
            Labeling4Neighbor(imgray, imgray2, label, row+1, col, Max_row, Max_col);
        }
    }
    if((row<1||row>=Max_row)||(col+1<1||col+1>=Max_col)){
    }else{ //오른쪽
        if(imgray[row][col+1]==255 && imgray2[row][col+1]==0){
            imgray2[row][col+1] = label;
            Labeling4Neighbor(imgray, imgray2, label, row, col+1, Max_row, Max_col);
        }
    }
}

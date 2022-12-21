#include <QFileDialog>
#include <QPainter>

#include "mainframe.h"
#include "ui_mainframe.h"
#include "imageform.h"

#include <iostream>
#include <stack>
using namespace std;

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
    if(ui->tabWidget->currentIndex() == 0)
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


struct edge{
    double dx;
    double dy;
    double magnitude;
    int    dir;
};

void MainFrame::on_ButtonCannyEdge_clicked()
{
        //포커스 된 ImageForm으로부터 영상을 가져옴
        KImageGray   igMain;
        if(_q_pFormFocused != 0 && _q_pFormFocused->ImageGray().Address() &&  _q_pFormFocused->ID() == "OPEN")
            igMain = _q_pFormFocused->ImageGray();
        else
            return;

        int dLow = ui->SpinLow->text().toInt();
        int dHigh = ui->SpinHigh->text().toInt();
        double sigma = ui->doubleSpinSigma->text().toDouble();


        vector<vector<edge>*> e_data;
        vector<edge> *col;

        if(!e_data.empty()){
            for(unsigned int i=0; i < e_data.size(); i++){
                delete e_data[i];
            }
        }

        for(unsigned int i=0; i < igMain.Row(); i++){
            col = new vector<edge>(igMain.Col());
            e_data.push_back(col);
        }

        int Half = 3.0*sigma + 0.5;
        int MaskLength = Half*2 + 1;

        double** KernalX;
        double** KernalY;
        KernalX = new double*[MaskLength];
        KernalY = new double*[MaskLength];

        for(int i=0; i< Half*2 + 1; i++){
            KernalX[i] = new double[MaskLength];
            KernalY[i] = new double[MaskLength];
        }

        double Tmp;
        double Scale = 0.0;
        double Sigma2 = 2.0*_SQR(sigma);

        for(int i = -Half; i<=Half; i++){
            Tmp = -i * exp(-(i*i)/2./sigma/sigma);
            for(int j = -Half; j<=Half; j++){
                KernalY[i+Half][j+Half] = Tmp * exp( -(j*j) / Sigma2) * Sigma2;
                KernalX[j+Half][i+Half] = KernalY[i+Half][j+Half];
                if(i < 0) Scale += KernalY[i+Half][j+Half];
            }
        }


        for(int i = -Half; i<=Half; i++){
            for(int j = -Half; j<=Half; j++){
                KernalX[i+Half][j+Half] /= -Scale;
                KernalY[i+Half][j+Half] /= -Scale;
            }
        }

        KImageGray igOut(igMain.Row(), igMain.Col());
        int Sy = Half, Ey = igMain.Row() - Half;
        int Sx = Half, Ex = igMain.Col() - Half;
        double GradX, GradY;

        for(int i = Sy; i<Ey; i++){
            for(int j = Sx; j<Ex; j++){

                GradX = GradY = 0.0;

                for(int r = -Half; r<=Half; r++){
                    for(int c = -Half; c<=Half; c++){
                        GradX += KernalX[r+Half][c+Half] * igMain[i+r][j+c];
                        GradY += KernalY[r+Half][c+Half] * igMain[i+r][j+c];
                    }
                }

                igOut[i][j] = abs(GradX)+abs(GradY);

                Tmp = atan2((float)GradY, (float)GradX);

                e_data[i]->at(j).magnitude  = abs(GradX)+abs(GradY);
                e_data[i]->at(j).dx    = abs(GradX);
                e_data[i]->at(j).dy    = abs(GradY);
                e_data[i]->at(j).dir   = (unsigned char)((((int)(Tmp * (180.0/M_PI)/22.5)+1)>>1) & 0x00000003);
            }
        }

        int ShiftX[4] = {1, 1, 0, -1};
        int ShiftY[4] = {0, 1, 1, 1};
        int H = igMain.Row() - Half - 1;
        int W = igMain.Col() - Half - 1;

        int dir=0;
        for(int i=Half+1; i<H; i++){
            for(int j=Half+1; j<W; j++){
                dir = e_data[i]->at(j).dir;

                if(e_data[i]->at(j).magnitude > e_data[i + ShiftY[dir]]->at(j + ShiftX[dir]).magnitude &&
                        e_data[i]->at(j).magnitude > e_data[i - ShiftY[dir]]->at(j - ShiftX[dir]).magnitude){

                    if(e_data[i]->at(j).magnitude >= dHigh){
                        igOut[i][j] = 255;
                    }
                    else if(e_data[i]->at(j).magnitude < dLow){
                        igOut[i][j] = 0;
                    }
                }
                else{
                    e_data[i]->at(j).magnitude = 0;
                }
            }
        }

        int Neighbor_Row[8] = {0, 1, 1, 1, 0, -1, -1, -1};
        int Neighbor_Col[8] = {1, 1, 0, -1, -1, -1, 0, 1};

        stack<pair<int,int>> point;

        for(unsigned int i=0; i<igOut.Row(); i++){
            for(unsigned int j=0; j<igOut.Col(); j++){

                if(igOut[i][j] == 255){
                    point.push(make_pair(i, j));

                    while(!point.empty()){
                        int Row = point.top().first;
                        int Col = point.top().second;
                        point.pop();

                        for(int k=0; k<8; k++){
                            if(Row + Neighbor_Row[k] == -1 || Row + Neighbor_Row[k] == (int)igOut.Row() ||
                                    Col + Neighbor_Col[k] == -1 || Col + Neighbor_Col[k] == (int)igOut.Col()){
                                continue;
                            }
                            if((e_data[Row + Neighbor_Row[k]]->at(Col + Neighbor_Col[k]).magnitude > dLow) && igOut[i][j] != 255){
                                igOut[Row + Neighbor_Row[k]][Col + Neighbor_Col[k]] = 255;
                                point.push(make_pair(Row + Neighbor_Row[k], Col + Neighbor_Col[k]));
                            }
                        }
                    }
                }
            }
        }

        //출력을 위한 ImageForm 생성
        ImageForm*  q_pForm = new ImageForm(igOut, "Canny Edge", this);

        _plpImageForm->Add(q_pForm);
        q_pForm->show();

        //UI 활성화 갱신
        UpdateUI();
}

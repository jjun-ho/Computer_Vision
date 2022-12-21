#include <QFileDialog>
#include <QPainter>

#include "mainframe.h"
#include "ui_mainframe.h"
#include "imageform.h"

#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <stack>
using namespace std;

#define DEG2RAD 0.0174533
#define RAD2DEG 57.2958
#define SQARE(x) (x*x)

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

void MainFrame::on_C_Hough_clicked()
{
    KImageGray   igMain;

    if(_q_pFormFocused != 0 && _q_pFormFocused->ImageGray().Address() &&  _q_pFormFocused->ID() == "OPEN")
        igMain = _q_pFormFocused->ImageGray();
    else
        return;

    KImageGray oGrayImg = igMain;
    std::vector<std::pair<int,int>>* pvnnEdge = new std::vector<std::pair<int, int>>();

    int m_nHeight = igMain.Row();
    int m_nWidth = igMain.Col();
    int nThreshold_U = 16;
    int nThreshold_D = 10;


    double dSobel_X[3][3] = {{-1, 0, 1},
                             {-2, 0, 2},
                             {-1, 0, 1}};
    double dSobel_Y[3][3] = {{-1, -2, -1},
                             { 0,  0,  0},
                             { 1,  2,  1}};

    double** ppdMagnitude = new double*[m_nHeight - 2];
    double** ppdTheta = new double*[m_nHeight - 2];
    double** ppdMiddle = new double*[m_nHeight - 2];
    std::vector<std::pair<int,int>> vnnMiddle;

    for(int i = 0; i < m_nHeight - 2; i++)
    {
        ppdMagnitude[i] = new double[m_nWidth - 2];
        ppdTheta[i] = new double[m_nWidth - 2];
        ppdMiddle[i] = new double[m_nWidth - 2];
    }

    for(int i = 0; i < m_nHeight - 2; i++)
    {
        for(int j = 0; j < m_nWidth - 2; j++)
        {
            double dX    = (oGrayImg._ppA[i][j]      * dSobel_X[0][0] +
                            oGrayImg._ppA[i][j+1]   * dSobel_X[0][1] +
                            oGrayImg._ppA[i][j+2]   * dSobel_X[0][2] +
                            oGrayImg._ppA[i+1][j]   * dSobel_X[1][0] +
                            oGrayImg._ppA[i+1][j+1] * dSobel_X[1][1] +
                            oGrayImg._ppA[i+1][j+2] * dSobel_X[1][2] +
                            oGrayImg._ppA[i+2][j]   * dSobel_X[2][0] +
                            oGrayImg._ppA[i+2][j+1] * dSobel_X[2][1] +
                            oGrayImg._ppA[i+2][j+2] * dSobel_X[2][2] ) / 8;
            double dY    = (oGrayImg._ppA[i][j]      * dSobel_Y[0][0] +
                            oGrayImg._ppA[i][j+1]   * dSobel_Y[0][1] +
                            oGrayImg._ppA[i][j+2]   * dSobel_Y[0][2] +
                            oGrayImg._ppA[i+1][j]   * dSobel_Y[1][0] +
                            oGrayImg._ppA[i+1][j+1] * dSobel_Y[1][1] +
                            oGrayImg._ppA[i+1][j+2] * dSobel_Y[1][2] +
                            oGrayImg._ppA[i+2][j]   * dSobel_Y[2][0] +
                            oGrayImg._ppA[i+2][j+1] * dSobel_Y[2][1] +
                            oGrayImg._ppA[i+2][j+2] * dSobel_Y[2][2] ) / 8;

            ppdMagnitude[i][j] = sqrt(SQARE(dX) + SQARE(dY));
            ppdTheta[i][j] = atan2(dY,dX);

            if( ppdMagnitude[i][j] > nThreshold_U)
            {
                pvnnEdge->push_back(std::pair<int,int>(i+1, j+1));
                ppdMiddle[i][j] = 2;
            }
            else if(ppdMagnitude[i][j] > nThreshold_D)
            {
                vnnMiddle.push_back(std::pair<int,int>(i,j));
                ppdMiddle[i][j] = 0;
            }
            else
            {
                ppdMiddle[i][j] = 0;
            }
        }
    }


    for(int i = 0; i < vnnMiddle.size(); i++)
    {
        std::pair<int, int> nnTmp = vnnMiddle.at(i);
        int nRow = nnTmp.first;
        int nCol = nnTmp.second;
        bool Flag = 0;

        if(nRow == 0)
        {
            if(nCol == 0)
            {
                if( ppdMiddle[nRow][nCol + 1] == 2     ||
                    ppdMiddle[nRow + 1][nCol] == 2     ||
                    ppdMiddle[nRow + 1][nCol + 1] == 2 )
                    Flag = 1;
            }
            else if(nCol == m_nWidth - 2)
            {
                if( ppdMiddle[nRow][nCol - 1] == 2     ||
                    ppdMiddle[nRow + 1][nCol] == 2     ||
                    ppdMiddle[nRow + 1][nCol - 1] == 2 )
                    Flag = 1;
            }
            else
            {
                if( ppdMiddle[nRow][nCol - 1] == 2     ||
                    ppdMiddle[nRow][nCol + 1] == 2     ||
                    ppdMiddle[nRow + 1][nCol - 1] == 2 ||
                    ppdMiddle[nRow + 1][nCol] == 2     ||
                    ppdMiddle[nRow + 1][nCol + 1] == 2 )
                    Flag = 1;
            }
        }
        else if(nRow == m_nHeight - 2)
        {
            if(nCol == 0)
            {
                if( ppdMiddle[nRow][nCol + 1] == 2     ||
                    ppdMiddle[nRow - 1][nCol] == 2     ||
                    ppdMiddle[nRow - 1][nCol + 1] == 2 )
                    Flag = 1;
            }
            else if(nCol == m_nWidth - 2)
            {
                if( ppdMiddle[nRow][nCol - 1] == 2     ||
                    ppdMiddle[nRow - 1][nCol] == 2     ||
                    ppdMiddle[nRow - 1][nCol - 1] == 2 )
                    Flag = 1;
            }
            else
            {
                if( ppdMiddle[nRow][nCol - 1] == 2     ||
                    ppdMiddle[nRow][nCol + 1] == 2     ||
                    ppdMiddle[nRow - 1][nCol - 1] == 2 ||
                    ppdMiddle[nRow - 1][nCol] == 2     ||
                    ppdMiddle[nRow - 1][nCol + 1] == 2 )
                    Flag = 1;
            }
        }
        else
        {
            if(nCol == 0)
            {
                if( ppdMiddle[nRow + 1][nCol + 1] == 2 ||
                    ppdMiddle[nRow + 1][nCol] == 2     ||
                    ppdMiddle[nRow][nCol + 1] == 2     ||
                    ppdMiddle[nRow - 1][nCol] == 2     ||
                    ppdMiddle[nRow - 1][nCol + 1] == 2 )
                    Flag = 1;
            }
            else if(nCol == m_nWidth - 2)
            {
                if( ppdMiddle[nRow + 1][nCol - 1] == 2 ||
                    ppdMiddle[nRow + 1][nCol] == 2     ||
                    ppdMiddle[nRow][nCol - 1] == 2     ||
                    ppdMiddle[nRow - 1][nCol] == 2     ||
                    ppdMiddle[nRow - 1][nCol - 1] == 2 )
                    Flag = 1;
            }
            else
            {
                if( ppdMiddle[nRow + 1][nCol - 1] == 2 ||
                    ppdMiddle[nRow + 1][nCol + 1] == 2 ||
                    ppdMiddle[nRow + 1][nCol] == 2     ||
                    ppdMiddle[nRow][nCol - 1] == 2     ||
                    ppdMiddle[nRow][nCol + 1] == 2     ||
                    ppdMiddle[nRow - 1][nCol - 1] == 2 ||
                    ppdMiddle[nRow - 1][nCol] == 2     ||
                    ppdMiddle[nRow - 1][nCol + 1] == 2 )
                    Flag = 1;
            }
        }

        if(Flag == 1)
        {
            pvnnEdge->push_back(std::pair<int, int>(nRow + 1, nCol + 1));
        }
    }


    for(int i = 0; i < m_nHeight - 2; i++)
    {
        delete ppdMagnitude[i];
        delete ppdTheta[i];
        delete ppdMiddle[i];
    }

//    delete ppdMagnitude;
//    delete ppdTheta;
//    delete ppdMiddle;

    //////////////////////////////////////////
    /// \brief q_pForm
    ///   //Edge Detection
    std::vector<std::pair<int,int>>* pnnEdgePoint = pvnnEdge;
    double** ppdMap = new double*[m_nHeight];
    int nMaxVoting = 0;
    int nMaxIndex = 0;
    double dRadius = 51.5;

    for(int i = 0; i < m_nHeight; i++)
    {
        ppdMap[i] = new double[m_nWidth]{0.0, };
    }

    for(int i = 0; i < pnnEdgePoint->size(); i++)
    {
        std::pair<int, int> nnTempPoint = pnnEdgePoint->at(i);

        ppdMap[nnTempPoint.first][nnTempPoint.second] = 1;
    }

    // Circle
    for(int i = 0; i < pnnEdgePoint->size(); i++)
    {
        std::pair<int, int> nnTempPoint = pnnEdgePoint->at(i);
        int nVote = 0;

        for(int j = 0; j < 360; j += 10)
        {
            int nRow = nnTempPoint.first - dRadius * cos(j * DEG2RAD);
            int nCol = nnTempPoint.second - dRadius * sin(j * DEG2RAD);

            if(nRow < 0)                nRow = 0;
            else if(nRow >= m_nHeight)   nRow = m_nHeight - 1;
            if(nCol < 0)                nCol = 0;
            else if(nCol >= m_nWidth)    nCol = m_nWidth - 1;

            if(i >= 3446)
            {
                int nnn = 0;
            }
            if(ppdMap[nRow][nCol] == 1)
                nVote++;
        }

        if(nMaxVoting < nVote)
        {
            nMaxVoting = nVote;
            nMaxIndex = i;
        }
    }

    std::pair<int, int> nnTempPoint = pnnEdgePoint->at(nMaxIndex);
    std::pair<int, int>* pnnCenterPoint = new std::pair<int,int>(nnTempPoint.first, nnTempPoint.second);


    // Memory UnLock
    for(int i = 0; i < m_nHeight; i++)
    {
        delete ppdMap[i];
    }

//    delete ppdMap;
//    delete pnnEdgePoint;

    KImageColor icOut(igMain.Row(),igMain.Col());
    icOut = igMain.GrayToRGB();

    for(int i = 0; i < 360; i++)
    {
        int nRow = pnnCenterPoint->first  + dRadius * cos(i);
        int nCol = pnnCenterPoint->second + dRadius * sin(i);

        icOut._ppA[nRow][nCol].r = 255;
        icOut._ppA[nRow][nCol].g = 0;
        icOut._ppA[nRow][nCol].b = 0;
    }

    delete pnnCenterPoint;

    ImageForm*  q_pForm = new ImageForm(icOut, "CircleHoughTransform", this); //코너가 포함된 이미지 출력

    _plpImageForm->Add(q_pForm);
    q_pForm->show();

    //UI 활성화 갱신
    UpdateUI();
}

void MainFrame::on_G_Hough_clicked()
{
    KImageGray   igMain;

    if(_q_pFormFocused != 0 && _q_pFormFocused->ImageGray().Address() &&  _q_pFormFocused->ID() == "OPEN")
        igMain = _q_pFormFocused->ImageGray();
    else
        return;

    std::string strFile = "plug.txt";
    std::ifstream in(strFile);
    std::string strTemp;
    std::vector<std::pair<int, int>> vnnPoint;
    std::vector<int> vnTheta;
    std::vector<std::pair<double, double>> vddR_A[4];

    int nPtSize = 0;
    int nCenterX = 0;
    int nCenterY = 0;

    if(in.is_open())
    {
        in >> strTemp;
        nPtSize = atoi(strTemp.c_str());

        while(in >> strTemp)
        {
            int nX = 0;
            int nY = 0;

            nX = atoi(strTemp.c_str());
            in >> strTemp;
            nY = atoi(strTemp.c_str());

            nCenterX += nX;
            nCenterY += nY;

            vnnPoint.push_back(std::pair<int,int>(nY, nX));
        }
    }

    nCenterX /= vnnPoint.size();
    nCenterY /= vnnPoint.size();


    for(int i = 0; i < vnnPoint.size(); i++)
    {
        std::pair<int, int> nnTemp;
        std::pair<int, int> nnPreTemp;
        double dTheta = 0.0;
        if(i == 0)
        {
            nnTemp = vnnPoint.at(i + 1);
            nnPreTemp = vnnPoint.at(vnnPoint.size() - 1);
        }
        else if(i == vnnPoint.size()-1)
        {
            nnTemp = vnnPoint.at(0);
            nnPreTemp = vnnPoint.at(i - 1);
        }
        else
        {
            nnTemp = vnnPoint.at(i + 1);
            nnPreTemp = vnnPoint.at(i - 1);
        }

        double dX = nnTemp.second - nnPreTemp.second;
        double dY = nnTemp.first - nnPreTemp.first;

        dTheta = atan2(dY, dX) * RAD2DEG + 90;

        if      ((67.5 >= dTheta && 22.5 < dTheta) || (247.5 >= dTheta && 202.5 < dTheta))      vnTheta.push_back(3);
        else if((112.5 >= dTheta && 67.5 < dTheta) || (292.5 >= dTheta && 247.5 < dTheta))      vnTheta.push_back(2);
        else if((157.5 >= dTheta && 112.5 < dTheta) || (337.5 >= dTheta && 292.5 < dTheta))     vnTheta.push_back(1);
        else                                                                                    vnTheta.push_back(0);


        double dR       = sqrt(SQARE((nnTemp.first - nCenterY)) + SQARE((nnTemp.second - nCenterX)));
        double dAlpha   = atan2((nnTemp.first - nCenterY), (nnTemp.second - nCenterX)) * RAD2DEG;

        vddR_A[vnTheta.at(vnTheta.size() - 1)].push_back(std::pair<double, double> (dR, dAlpha));
    }

    std::vector<double> vdTheta;

    int m_nHeight = igMain.Row();
    int m_nWidth = igMain.Col();
    int nThreshold_U = 34;
    int nThreshold_D = 10;
    KImageGray oGrayImg = igMain;
    std::vector<std::pair<int,int>>* pvnnEdge = new std::vector<std::pair<int, int>>();

    double dSobel_X[3][3] = {{-1, 0, 1},
                             {-2, 0, 2},
                             {-1, 0, 1}};
    double dSobel_Y[3][3] = {{-1, -2, -1},
                             { 0,  0,  0},
                             { 1,  2,  1}};

    double** ppdMagnitude = new double*[m_nHeight - 2];
    double** ppdTheta = new double*[m_nHeight - 2];
    double** ppdMiddle = new double*[m_nHeight - 2];
    std::vector<std::pair<int,int>> vnnMiddle;

    for(int i = 0; i < m_nHeight - 2; i++)
    {
        ppdMagnitude[i] = new double[m_nWidth - 2];
        ppdTheta[i] = new double[m_nWidth - 2];
        ppdMiddle[i] = new double[m_nWidth - 2];
    }

    for(int i = 0; i < m_nHeight - 2; i++)
    {
        for(int j = 0; j < m_nWidth - 2; j++)
        {
            double dX    = (oGrayImg._ppA[i][j]      * dSobel_X[0][0] +
                            oGrayImg._ppA[i][j+1]   * dSobel_X[0][1] +
                            oGrayImg._ppA[i][j+2]   * dSobel_X[0][2] +
                            oGrayImg._ppA[i+1][j]   * dSobel_X[1][0] +
                            oGrayImg._ppA[i+1][j+1] * dSobel_X[1][1] +
                            oGrayImg._ppA[i+1][j+2] * dSobel_X[1][2] +
                            oGrayImg._ppA[i+2][j]   * dSobel_X[2][0] +
                            oGrayImg._ppA[i+2][j+1] * dSobel_X[2][1] +
                            oGrayImg._ppA[i+2][j+2] * dSobel_X[2][2] ) / 8;
            double dY    = (oGrayImg._ppA[i][j]      * dSobel_Y[0][0] +
                            oGrayImg._ppA[i][j+1]   * dSobel_Y[0][1] +
                            oGrayImg._ppA[i][j+2]   * dSobel_Y[0][2] +
                            oGrayImg._ppA[i+1][j]   * dSobel_Y[1][0] +
                            oGrayImg._ppA[i+1][j+1] * dSobel_Y[1][1] +
                            oGrayImg._ppA[i+1][j+2] * dSobel_Y[1][2] +
                            oGrayImg._ppA[i+2][j]   * dSobel_Y[2][0] +
                            oGrayImg._ppA[i+2][j+1] * dSobel_Y[2][1] +
                            oGrayImg._ppA[i+2][j+2] * dSobel_Y[2][2] ) / 8;

            ppdMagnitude[i][j] = sqrt(SQARE(dX) + SQARE(dY));
            ppdTheta[i][j] = atan2(dY,dX);

            if( ppdMagnitude[i][j] > nThreshold_U)
            {
                pvnnEdge->push_back(std::pair<int,int>(i+1, j+1));
                vdTheta.push_back(ppdTheta[i][j] * RAD2DEG + 90);
                ppdMiddle[i][j] = 2;
            }
            else if(ppdMagnitude[i][j] > nThreshold_D)
            {
                vnnMiddle.push_back(std::pair<int,int>(i,j));
                ppdMiddle[i][j] = 0;
            }
            else
            {
                ppdMiddle[i][j] = 0;
            }
        }
    }


    for(int i = 0; i < vnnMiddle.size(); i++)
    {
        std::pair<int, int> nnTmp = vnnMiddle.at(i);
        int nRow = nnTmp.first;
        int nCol = nnTmp.second;
        bool Flag = 0;

        if(nRow == 0)
        {
            if(nCol == 0)
            {
                if( ppdMiddle[nRow][nCol + 1] == 2     ||
                    ppdMiddle[nRow + 1][nCol] == 2     ||
                    ppdMiddle[nRow + 1][nCol + 1] == 2 )
                    Flag = 1;
            }
            else if(nCol == m_nWidth - 2)
            {
                if( ppdMiddle[nRow][nCol - 1] == 2     ||
                    ppdMiddle[nRow + 1][nCol] == 2     ||
                    ppdMiddle[nRow + 1][nCol - 1] == 2 )
                    Flag = 1;
            }
            else
            {
                if( ppdMiddle[nRow][nCol - 1] == 2     ||
                    ppdMiddle[nRow][nCol + 1] == 2     ||
                    ppdMiddle[nRow + 1][nCol - 1] == 2 ||
                    ppdMiddle[nRow + 1][nCol] == 2     ||
                    ppdMiddle[nRow + 1][nCol + 1] == 2 )
                    Flag = 1;
            }
        }
        else if(nRow == m_nHeight - 2)
        {
            if(nCol == 0)
            {
                if( ppdMiddle[nRow][nCol + 1] == 2     ||
                    ppdMiddle[nRow - 1][nCol] == 2     ||
                    ppdMiddle[nRow - 1][nCol + 1] == 2 )
                    Flag = 1;
            }
            else if(nCol == m_nWidth - 2)
            {
                if( ppdMiddle[nRow][nCol - 1] == 2     ||
                    ppdMiddle[nRow - 1][nCol] == 2     ||
                    ppdMiddle[nRow - 1][nCol - 1] == 2 )
                    Flag = 1;
            }
            else
            {
                if( ppdMiddle[nRow][nCol - 1] == 2     ||
                    ppdMiddle[nRow][nCol + 1] == 2     ||
                    ppdMiddle[nRow - 1][nCol - 1] == 2 ||
                    ppdMiddle[nRow - 1][nCol] == 2     ||
                    ppdMiddle[nRow - 1][nCol + 1] == 2 )
                    Flag = 1;
            }
        }
        else
        {
            if(nCol == 0)
            {
                if( ppdMiddle[nRow + 1][nCol + 1] == 2 ||
                    ppdMiddle[nRow + 1][nCol] == 2     ||
                    ppdMiddle[nRow][nCol + 1] == 2     ||
                    ppdMiddle[nRow - 1][nCol] == 2     ||
                    ppdMiddle[nRow - 1][nCol + 1] == 2 )
                    Flag = 1;
            }
            else if(nCol == m_nWidth - 2)
            {
                if( ppdMiddle[nRow + 1][nCol - 1] == 2 ||
                    ppdMiddle[nRow + 1][nCol] == 2     ||
                    ppdMiddle[nRow][nCol - 1] == 2     ||
                    ppdMiddle[nRow - 1][nCol] == 2     ||
                    ppdMiddle[nRow - 1][nCol - 1] == 2 )
                    Flag = 1;
            }
            else
            {
                if( ppdMiddle[nRow + 1][nCol - 1] == 2 ||
                    ppdMiddle[nRow + 1][nCol + 1] == 2 ||
                    ppdMiddle[nRow + 1][nCol] == 2     ||
                    ppdMiddle[nRow][nCol - 1] == 2     ||
                    ppdMiddle[nRow][nCol + 1] == 2     ||
                    ppdMiddle[nRow - 1][nCol - 1] == 2 ||
                    ppdMiddle[nRow - 1][nCol] == 2     ||
                    ppdMiddle[nRow - 1][nCol + 1] == 2 )
                    Flag = 1;
            }
        }

        if(Flag == 1)
        {
            pvnnEdge->push_back(std::pair<int, int>(nRow + 1, nCol + 1));
            vdTheta.push_back(ppdTheta[nRow][nCol] * RAD2DEG + 90);
        }
    }


    for(int i = 0; i < m_nHeight - 2; i++)
    {
        delete ppdMagnitude[i];
        delete ppdTheta[i];
        delete ppdMiddle[i];
    }

//    delete ppdMagnitude;
//    delete ppdTheta;
//    delete ppdMiddle;

    //return pvnnEdge;

    std::vector<std::pair<int, int>>* pvnnPoint_Input =  pvnnEdge;

    ////////

#if 1 //Scale Only 1

    std::vector<std::vector<std::vector<double>>> vdVotiongMap;

    vdVotiongMap.resize(m_nHeight);

    for(int ii = 0; ii < m_nHeight; ii++)
    {
        vdVotiongMap[ii].resize(m_nWidth);
        for(int jj = 0; jj < m_nWidth; jj++)
        {
            vdVotiongMap[ii][jj].resize(16);
            for(int kk = 0; kk < 16; kk++)
            {
                vdVotiongMap[ii][jj][kk] = 0;
            }
        }
    }

    //Edge
    for(int i = 0; i < vdTheta.size(); i++)
    {
        std::pair<int,int> nnPoint = pvnnPoint_Input->at(i);

        //Theta
        for(double j = 0.0; j < 360; j += 22.5)
        {
            double dTheta = vdTheta[i] - j;
            int nIndex = 0;

            if((67.5 >= dTheta && 22.5 < dTheta) || (247.5 >= dTheta && 202.5 < dTheta))            nIndex = 3;
            else if((112.5 >= dTheta && 67.5 < dTheta) || (292.5 >= dTheta && 247.5 < dTheta))      nIndex = 2;
            else if((157.5 >= dTheta && 112.5 < dTheta) || (337.5 >= dTheta && 292.5 < dTheta))     nIndex = 1;
            else                                                                                    nIndex = 0;


            for(int k = 0; k < vddR_A[nIndex].size(); k++)
            {

                std::pair<double, double> ddR_A = vddR_A[nIndex].at(k);
                int nCx = nnPoint.second - (ddR_A.first * cos((ddR_A.second + j)* DEG2RAD)) + 0.5; // 0.5 is Half Up
                int nCy = nnPoint.first  - (ddR_A.first * sin((ddR_A.second + j)* DEG2RAD)) + 0.5;
                int nJ = j/22.5;

                if(nCy < 1 || nCy > m_nHeight - 2)
                    continue;
                if(nCx < 1 || nCx > m_nWidth - 2)
                    continue;

                for(int ii = -1; ii < 2; ii++)
                    for(int jj = -1; jj < 2; jj++)
                        for(int kk = -1; kk < 2; kk++)
                        {
                            if(nJ == 0 || nJ == 15)
                                continue;

                            vdVotiongMap[nCy+ii][nCx+jj][nJ+kk] += 0.7;
                        }

                vdVotiongMap[nCy][nCx][nJ] += 0.3;
            }
        }
    }


    double dMaxVote = 0;
    int nMaxIndex[3] = {0,};

    for(int i = 0; i < m_nHeight; i++)
    {
        for(int j = 0; j < m_nWidth; j++)
        {
            for(int k = 0; k < 16; k++)
            {
                if(dMaxVote < vdVotiongMap[i][j][k])
                {
                    dMaxVote = vdVotiongMap[i][j][k];
                    nMaxIndex[0] = i;
                    nMaxIndex[1] = j;
                    nMaxIndex[2] = k;
                }
            }
        }
    }

    nMaxIndex[0] -= 10;


    std::vector<std::pair<int, int>>* FormTable = new std::vector<std::pair<int, int>>();

    FormTable->push_back(std::pair<int,int>(nMaxIndex[0], nMaxIndex[1]));

    for(int i = 0; i < 4; i++)
    {
        for(int j = 0; j < vddR_A[i].size(); j++)
        {
            std::pair<double, double> ddTemp = vddR_A[i].at(j);
            int nRow = nMaxIndex[0] + 0.8*ddTemp.first * sin((ddTemp.second + (nMaxIndex[2] * 22.5)) * DEG2RAD);
            int nCol = nMaxIndex[1] + 0.8*ddTemp.first * cos((ddTemp.second + (nMaxIndex[2] * 22.5)) * DEG2RAD);

            FormTable->push_back(std::pair<int,int>(nRow, nCol));
        }
    }

    // Memory UnLock
    delete pvnnPoint_Input;
    vdVotiongMap.clear();

#else
    std::vector<std::vector<std::vector<std::vector<double>>>> vdVotiongMap;
    VectorIntialize4D_double(vdVotiongMap, m_nHeight, m_nWidth, 10, 16);

    //Edge
    for(int i = 0; i < vdTheta_Input.size(); i++)
    {
        std::pair<int,int> nnPoint = pvnnPoint_Input->at(i);

        //Theta
        for(double j = 0.0; j < 360; j += 22.5)
        {
            double dTheta = vdTheta_Input[i] - j;
            int nIndex = 0;

            if((67.5 >= dTheta && 22.5 < dTheta) || (247.5 >= dTheta && 202.5 < dTheta))            nIndex = 3;
            else if((112.5 >= dTheta && 67.5 < dTheta) || (292.5 >= dTheta && 247.5 < dTheta))      nIndex = 2;
            else if((157.5 >= dTheta && 112.5 < dTheta) || (337.5 >= dTheta && 292.5 < dTheta))     nIndex = 1;
            else                                                                                    nIndex = 0;


            for(int k = 0; k < vddR_A[nIndex].size(); k++)
            {
                for(double l = 0.8; l <= 1.2; l += 0.1)
                {
                    std::pair<double, double> ddR_A = vddR_A[nIndex].at(k);
                    int nCx = nnPoint.second - (l * ddR_A.first * cos((ddR_A.second + j)* DEG2RAD)) + 0.5; // 0.5 is Half Up
                    int nCy = nnPoint.first  - (l * ddR_A.first * sin((ddR_A.second + j)* DEG2RAD)) + 0.5;
                    int nJ = j/22.5;
                    int nIndexS = l*10 - 8;

                    if(nCy < 1 || nCy > m_nHeight - 2)
                        continue;
                    if(nCx < 1 || nCx > m_nWidth - 2)
                        continue;

                    for(int ii = -1; ii < 2; ii++)
                        for(int jj = -1; jj < 2; jj++)
                            for(int kk = -1; kk < 2; kk++)
                            {

                                if(nIndexS == 0 || nIndexS == 4)
                                    continue;

                                for(int ll = -1; ll < 2; ll++)
                                {
                                    if(nJ == 0 || nJ == 15)
                                        continue;

                                    vdVotiongMap[nCy+ii][nCx+jj][nIndexS+kk][nJ+ll] += 0.7;
                                }
                            }
                    vdVotiongMap[nCy][nCx][nIndexS][nJ] += 0.3;
                }
            }
        }
    }


    double dMaxVote = 0;
    int nMaxIndex[4] = {0,};

    for(int i = 0; i < m_nHeight; i++)
    {
        for(int j = 0; j < m_nWidth; j++)
        {
            for(int k = 0; k < 5; k++)
            {
                for(int l = 0; l < 16; l++)
                {
                    if(dMaxVote < vdVotiongMap[i][j][k][l])
                    {
                        dMaxVote = vdVotiongMap[i][j][k][l];
                        nMaxIndex[0] = i;
                        nMaxIndex[1] = j;
                        nMaxIndex[2] = k;
                        nMaxIndex[3] = l;
                    }
                }
            }
        }
    }


    std::vector<std::pair<int, int>>* FormTable = new std::vector<std::pair<int, int>>();
    FormTable->push_back(std::pair<int,int>(nMaxIndex[0], nMaxIndex[1]));

    for(int i = 0; i < 4; i++)
    {
        for(int j = 0; j < vddR_A[i].size(); j++)
        {
            std::pair<double, double> ddTemp = vddR_A[i].at(j);
            int nRow = nMaxIndex[0] + (double)(nMaxIndex[2]*0.1 + 0.8) * ddTemp.first * sin((ddTemp.second + (nMaxIndex[3] * 22.5)) * DEG2RAD);
            int nCol = nMaxIndex[1] + (double)(nMaxIndex[2]*0.1 + 0.8) * ddTemp.first * cos((ddTemp.second + (nMaxIndex[3] * 22.5)) * DEG2RAD);

            FormTable->push_back(std::pair<int,int>(nRow, nCol));
        }
    }

    // Memory UnLock
    delete pvnnPoint_Input;
    vdVotiongMap.clear();
#endif

    std::vector<std::pair<int, int>>* nnPoint = FormTable;
    ////////////

    KImageColor icOut(igMain.Row(),igMain.Col());
    icOut = igMain.GrayToRGB();

    // Hough Transform Image
    for(int i = -1; nnPoint->size() > 0 && i < 2;  i++)
    {
        std::pair<int, int> nnTemp = nnPoint->at(0);

        for(int j = 0; j < 2; j++)
        {
            icOut._ppA[nnTemp.first + i][nnTemp.second + j].r = 255;
            icOut._ppA[nnTemp.first + i][nnTemp.second + j].g = 0;
            icOut._ppA[nnTemp.first + i][nnTemp.second + j].b = 0;
        }

    }
    for(int i = 1; i < nnPoint->size(); i++)
    {
        std::pair<int, int> nnTemp = nnPoint->at(i);
        icOut._ppA[nnTemp.first][nnTemp.second].r = 255;
        icOut._ppA[nnTemp.first][nnTemp.second].g = 0;
        icOut._ppA[nnTemp.first][nnTemp.second].b = 0;
    }

    //Memory Unlock
    delete nnPoint;

    ImageForm*  q_pForm = new ImageForm(icOut, "GeneralizedHoughTransform", this); //코너가 포함된 이미지 출력

    _plpImageForm->Add(q_pForm);
    q_pForm->show();


    //UI 활성화 갱신
    UpdateUI();
}


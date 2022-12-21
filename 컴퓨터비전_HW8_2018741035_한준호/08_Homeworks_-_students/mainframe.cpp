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

    m_qstFile = q_stFile;

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

void MainFrame::on_ButtonOpticalFlow_clicked()
{
    KImageGray   igMain;

    if(_q_pFormFocused != 0 && _q_pFormFocused->ImageGray().Address() &&  _q_pFormFocused->ID() == "OPEN")
        igMain = _q_pFormFocused->ImageGray();
    else
        return;
#if 1
    int nTarget = ui->spinOptical->text().toInt();
    QString q_stTmp = m_qstFile;
    q_stTmp.chop(6);
    QString qstrBeforeImg = m_qstFile;
    QString qstrAfterImg = q_stTmp;



    if(nTarget < 10)        q_stTmp.sprintf("0%d.pgm", nTarget);
    else                    q_stTmp.sprintf("%d.pgm", nTarget);
    qstrAfterImg.append(q_stTmp);

    ImageForm* q_pBeforeForm   = new ImageForm(qstrBeforeImg, "OPEN", this);
    ImageForm* q_pAfterForm   = new ImageForm(qstrAfterImg, "OPEN", this);

    int m_nHeight = igMain.Row();
    int m_nWidth = igMain.Col();

    KImageGray igMain_before    = q_pBeforeForm->_igMain;
    KImageGray igMain_after     = q_pAfterForm->_igMain;

    double dSobel_X[3][3] = {{-1, 0, 1},
                             {-2, 0, 2},
                             {-1, 0, 1}};
    double dSobel_Y[3][3] = {{-1, -2, -1},
                             { 0,  0,  0},
                             { 1,  2,  1}};
    double dGausian[5][5] = {{1,  4,  7,  4, 1},
                             {4, 16, 26, 16, 4},
                             {7, 26, 41, 26, 7},
                             {4, 16, 26, 16, 4},
                             {1,  4,  7,  4, 1}};
    double** ppdX = new double*[m_nHeight];
    double** ppdY = new double*[m_nHeight];
    double** ppdT = new double*[m_nHeight];
    double** ppdV_X = new double*[m_nHeight/5];
    double** ppdV_Y = new double*[m_nHeight/5];

    for(int i = 0; i < m_nHeight ; i++)
    {
        ppdX[i] = new double[m_nWidth];
        ppdY[i] = new double[m_nWidth];
        ppdT[i] = new double[m_nWidth];

        if(i%5 == 0)
        {
            ppdV_X[i/5] = new double[m_nWidth/5];
            ppdV_Y[i/5] = new double[m_nWidth/5];
        }
    }


    // dX dY dT
    for(int i = 0; i < m_nHeight; i++)
    {
        for(int j = 0; j < m_nWidth; j++)
        {
            ppdT[i][j] = igMain_before._ppA[i][j] - igMain_after._ppA[i][j];    // Ix Iy = -It

            if(!(i == 0 || j == 0 || i == m_nHeight - 1 || j == m_nWidth - 1))
            {
                double dX    = (igMain_after._ppA[i-1][j-1] * dSobel_X[0][0] +
                                igMain_after._ppA[i-1][j]   * dSobel_X[0][1] +
                                igMain_after._ppA[i-1][j+1] * dSobel_X[0][2] +
                                igMain_after._ppA[i][j-1]   * dSobel_X[1][0] +
                                igMain_after._ppA[i][j]     * dSobel_X[1][1] +
                                igMain_after._ppA[i][j+1]   * dSobel_X[1][2] +
                                igMain_after._ppA[i+1][j-1] * dSobel_X[2][0] +
                                igMain_after._ppA[i+1][j]   * dSobel_X[2][1] +
                                igMain_after._ppA[i+1][j+1] * dSobel_X[2][2] ) / 8;
                double dY    = (igMain_after._ppA[i-1][j-1] * dSobel_Y[0][0] +
                                igMain_after._ppA[i-1][j]   * dSobel_Y[0][1] +
                                igMain_after._ppA[i-1][j+1] * dSobel_Y[0][2] +
                                igMain_after._ppA[i][j-1]   * dSobel_Y[1][0] +
                                igMain_after._ppA[i][j]     * dSobel_Y[1][1] +
                                igMain_after._ppA[i][j+1]   * dSobel_Y[1][2] +
                                igMain_after._ppA[i+1][j-1] * dSobel_Y[2][0] +
                                igMain_after._ppA[i+1][j]   * dSobel_Y[2][1] +
                                igMain_after._ppA[i+1][j+1] * dSobel_Y[2][2] ) / 8;
                ppdX[i][j] = dX;
                ppdY[i][j] = dY;
            }
            else
            {
                ppdX[i][j] = 0;
                ppdY[i][j] = 0;
            }
        }
    }


    for(int i = 1; i < m_nHeight - 1; i+= 5)
    {

        for(int j = 1; j < m_nWidth - 1; j += 5)
        {
            double AWA[2][2] = {0,};
            double AWb[2]   = {0,};
            double A_T[2][25] = {0,};
            double Wb[25]= {ppdT[i][j]   * dGausian[0][0], ppdT[i][j+1]   * dGausian[0][1], ppdT[i][j+2]   * dGausian[0][2], ppdT[i][j+3]   * dGausian[0][3], ppdT[i][j+4]   * dGausian[0][4],
                            ppdT[i+1][j] * dGausian[1][0], ppdT[i+1][j+1] * dGausian[1][1], ppdT[i+1][j+2] * dGausian[1][2], ppdT[i+1][j+3] * dGausian[1][3], ppdT[i+1][j+4] * dGausian[1][4],
                            ppdT[i+2][j] * dGausian[2][0], ppdT[i+2][j+1] * dGausian[2][1], ppdT[i+2][j+2] * dGausian[2][2], ppdT[i+2][j+3] * dGausian[2][3], ppdT[i+2][j+4] * dGausian[2][4],
                            ppdT[i+3][j] * dGausian[3][0], ppdT[i+3][j+1] * dGausian[3][1], ppdT[i+3][j+2] * dGausian[3][2], ppdT[i+3][j+3] * dGausian[3][3], ppdT[i+3][j+4] * dGausian[3][4],
                            ppdT[i+4][j] * dGausian[4][0], ppdT[i+4][j+1] * dGausian[4][1], ppdT[i+4][j+2] * dGausian[4][2], ppdT[i+4][j+3] * dGausian[4][3], ppdT[i+4][j+4] * dGausian[4][4]};
            double ddX = 0.0;
            double ddY = 0.0;
            double dXY = 0.0;

            for(int ii = i; ii < i+5; ii++)
            {
                for(int jj = j; jj < j+5; jj++)
                {
                    ddX += ppdX[ii][jj] * ppdX[ii][jj] * dGausian[ii-i][jj-j]/273;
                    ddY += ppdX[ii][jj] * ppdY[ii][jj] * dGausian[ii-i][jj-j]/273;
                    dXY += ppdY[ii][jj] * ppdY[ii][jj] * dGausian[ii-i][jj-j]/273;

                    A_T[0][5*(ii - i) + (jj - j)] = ppdX[ii][jj];
                    A_T[1][5*(ii - i) + (jj - j)] = ppdY[ii][jj];
//                    Wb[5*(ii - i) + (jj - j)] = ppdT[ii][jj]*dGausian[ii][jj]/273;
                    AWb[0] += A_T[0][5*(ii - i) + (jj - j)] * Wb[5*(ii - i) + (jj - j)]/273;
                    AWb[1] += A_T[1][5*(ii - i) + (jj - j)] * Wb[5*(ii - i) + (jj - j)]/273;
                }
            }



            double det = ddX*ddY - dXY*dXY;

            AWA[0][0] =  ddY / det;
            AWA[0][1] = -dXY / det;
            AWA[1][0] = -dXY / det;
            AWA[1][1] =  ddX / det;

            ppdV_X[i/5][j/5] = AWA[0][0] * AWb[0] + AWA[0][1] * AWb[1];
            ppdV_Y[i/5][j/5] = AWA[1][0] * AWb[0] + AWA[1][1] * AWb[1];
        }
    }

    for(int i = 0; i < m_nHeight ; i++)
    {
        delete ppdX[i];
        delete ppdY[i];
        delete ppdT[i];
    }

    delete[] ppdX;
    delete[] ppdY;
    delete[] ppdT;

    std::pair<double**, double**>* ReturnValue = new std::pair<double**, double**>(ppdV_X, ppdV_Y);

    KImageGray igTemp = q_pAfterForm->ImageGray();
    KImageColor icMain(igTemp.Row(),igTemp.Col());
    icMain    = igTemp.GrayToRGB();

    for(unsigned int i = 2; i < icMain.Row(); i+= 5)
    {
        for(unsigned int j = 2; j < icMain.Col(); j+= 5)
        {
            if(abs(ReturnValue->first[i/5][j/5]) < abs(ReturnValue->second[i/5][j/5]))
            {
                for(int l = 0; l < abs(ReturnValue->second[i/5][j/5]); l++)
                {
                    if(l > 7) break;

                    int r = i + (ReturnValue->second[i/5][j/5]   /  abs(ReturnValue->second[i/5][j/5]))*l;
                    int c = j + (ReturnValue->first[i/5][j/5]    /  abs(ReturnValue->second[i/5][j/5]))*l;

                    if((r >= 0 && r < icMain.Row()) && (c >= 0 && c < icMain.Col()))
                    {
                        icMain._ppA[r][c].r = 0;
                        icMain._ppA[r][c].g = 255;
                        icMain._ppA[r][c].b = 0;
                    }

                    else break;
                }
            }
            else
            {
                for(int l = 0; l < abs(ReturnValue->first[i/5][j/5]); l++)
                {
                    if(l > 7) break;

                    int r = i + (ReturnValue->second[i/5][j/5]   /  abs(ReturnValue->first[i/5][j/5]))*l;
                    int c = j + (ReturnValue->first[i/5][j/5]    /  abs(ReturnValue->first[i/5][j/5]))*l;

                    if((r >= 0 && r < icMain.Row()) && (c >= 0 && c < icMain.Col()))
                    {
                        icMain._ppA[r][c].r = 0;
                        icMain._ppA[r][c].g = 255;
                        icMain._ppA[r][c].b = 0;
                    }

                    else break;
                }
            }
        }
    }

    ImageForm*  q_pForm = new ImageForm(icMain, "Optical Flow",this); //코너가 포함된 이미지 출력

    _plpImageForm->Add(q_pForm);
    q_pForm->show();

    //UI 활성화 갱신
    UpdateUI();
#else
    QString q_stFile = m_qstFile;
    QString qstrNum = q_stFile.mid(q_stFile.length() - 6, 2);
    int nNum = qstrNum.toInt();
    q_stFile.chop(6);


    for(int ii = 0; ii < 16; ii++)
    {
        QString qstrBeforeImg = q_stFile;
        QString qstrAfterImg = q_stFile;
        QString qstrTemp;
        int nCnt = nNum + ii;

        if(nCnt < 9)
        {
            qstrTemp.sprintf("0%d.pgm",nCnt);
            qstrBeforeImg.append(qstrTemp);
            qstrTemp.sprintf("0%d.pgm",nCnt + 1);
            qstrAfterImg.append(qstrTemp);
        }
        else if(nCnt == 9)
        {
            qstrTemp.sprintf("0%d.pgm",nCnt);
            qstrBeforeImg.append(qstrTemp);
            qstrTemp.sprintf("%d.pgm",nCnt + 1);
            qstrAfterImg.append(qstrTemp);
        }
        else if(nCnt <= 16)
        {
            qstrTemp.sprintf("%d.pgm",nCnt);
            qstrBeforeImg.append(qstrTemp);

            if(nCnt == 16)
                qstrTemp.sprintf("01.pgm");
            else
                qstrTemp.sprintf("%d.pgm",nCnt + 1);

            qstrAfterImg.append(qstrTemp);
        }
        else
        {
            if(nCnt > 25)
            {
                qstrTemp.sprintf("%d.pgm",nCnt - 16);
                qstrBeforeImg.append(qstrTemp);
                qstrTemp.sprintf("%d.pgm",nCnt - 15);
                qstrAfterImg.append(qstrTemp);
            }
            else if(nCnt == 25)
            {
                qstrTemp.sprintf("0%d.pgm",nCnt - 16);
                qstrBeforeImg.append(qstrTemp);
                qstrTemp.sprintf("%d.pgm",nCnt - 15);
                qstrAfterImg.append(qstrTemp);
            }
            else
            {
                qstrTemp.sprintf("0%d.pgm",nCnt - 16);
                qstrBeforeImg.append(qstrTemp);
                qstrTemp.sprintf("0%d.pgm",nCnt - 15);
                qstrAfterImg.append(qstrTemp);
            }
        }
        ImageForm* q_pBeforeForm   = new ImageForm(qstrBeforeImg, "OPEN", this);
        ImageForm* q_pAfterForm   = new ImageForm(qstrAfterImg, "OPEN", this);

        KImageGray igMain_before    = q_pBeforeForm->_igMain;
        KImageGray igMain_after     = q_pAfterForm->_igMain;

        double dSobel_X[3][3] = {{-1, 0, 1},
                                 {-2, 0, 2},
                                 {-1, 0, 1}};
        double dSobel_Y[3][3] = {{-1, -2, -1},
                                 { 0,  0,  0},
                                 { 1,  2,  1}};
        double dGausian[5][5] = {{1,  4,  7,  4, 1},
                                 {4, 16, 26, 16, 4},
                                 {7, 26, 41, 26, 7},
                                 {4, 16, 26, 16, 4},
                                 {1,  4,  7,  4, 1}};
        double** ppdX = new double*[m_nHeight];
        double** ppdY = new double*[m_nHeight];
        double** ppdT = new double*[m_nHeight];
        double** ppdV_X = new double*[m_nHeight/5];
        double** ppdV_Y = new double*[m_nHeight/5];

        for(int i = 0; i < m_nHeight ; i++)
        {
            ppdX[i] = new double[m_nWidth];
            ppdY[i] = new double[m_nWidth];
            ppdT[i] = new double[m_nWidth];

            if(i%5 == 0)
            {
                ppdV_X[i/5] = new double[m_nWidth/5];
                ppdV_Y[i/5] = new double[m_nWidth/5];
            }
        }


        // dX dY dT
        for(int i = 0; i < m_nHeight; i++)
        {
            for(int j = 0; j < m_nWidth; j++)
            {
                ppdT[i][j] = igMain_before._ppA[i][j] - igMain_after._ppA[i][j];    // Ix Iy = -It

                if(!(i == 0 || j == 0 || i == m_nHeight - 1 || j == m_nWidth - 1))
                {
                    double dX    = (igMain_after._ppA[i-1][j-1] * dSobel_X[0][0] +
                                    igMain_after._ppA[i-1][j]   * dSobel_X[0][1] +
                                    igMain_after._ppA[i-1][j+1] * dSobel_X[0][2] +
                                    igMain_after._ppA[i][j-1]   * dSobel_X[1][0] +
                                    igMain_after._ppA[i][j]     * dSobel_X[1][1] +
                                    igMain_after._ppA[i][j+1]   * dSobel_X[1][2] +
                                    igMain_after._ppA[i+1][j-1] * dSobel_X[2][0] +
                                    igMain_after._ppA[i+1][j]   * dSobel_X[2][1] +
                                    igMain_after._ppA[i+1][j+1] * dSobel_X[2][2] ) / 8;
                    double dY    = (igMain_after._ppA[i-1][j-1] * dSobel_Y[0][0] +
                                    igMain_after._ppA[i-1][j]   * dSobel_Y[0][1] +
                                    igMain_after._ppA[i-1][j+1] * dSobel_Y[0][2] +
                                    igMain_after._ppA[i][j-1]   * dSobel_Y[1][0] +
                                    igMain_after._ppA[i][j]     * dSobel_Y[1][1] +
                                    igMain_after._ppA[i][j+1]   * dSobel_Y[1][2] +
                                    igMain_after._ppA[i+1][j-1] * dSobel_Y[2][0] +
                                    igMain_after._ppA[i+1][j]   * dSobel_Y[2][1] +
                                    igMain_after._ppA[i+1][j+1] * dSobel_Y[2][2] ) / 8;
                    ppdX[i][j] = dX;
                    ppdY[i][j] = dY;
                }
                else
                {
                    ppdX[i][j] = 0;
                    ppdY[i][j] = 0;
                }
            }
        }


        for(int i = 1; i < m_nHeight - 1; i+= 5)
        {

            for(int j = 1; j < m_nWidth - 1; j += 5)
            {
                double AWA[2][2] = {0,};
                double AWb[2]   = {0,};
                double A_T[2][25] = {0,};
                double Wb[25]= {ppdT[i][j]   * dGausian[0][0], ppdT[i][j+1]   * dGausian[0][1], ppdT[i][j+2]   * dGausian[0][2], ppdT[i][j+3]   * dGausian[0][3], ppdT[i][j+4]   * dGausian[0][4],
                                ppdT[i+1][j] * dGausian[1][0], ppdT[i+1][j+1] * dGausian[1][1], ppdT[i+1][j+2] * dGausian[1][2], ppdT[i+1][j+3] * dGausian[1][3], ppdT[i+1][j+4] * dGausian[1][4],
                                ppdT[i+2][j] * dGausian[2][0], ppdT[i+2][j+1] * dGausian[2][1], ppdT[i+2][j+2] * dGausian[2][2], ppdT[i+2][j+3] * dGausian[2][3], ppdT[i+2][j+4] * dGausian[2][4],
                                ppdT[i+3][j] * dGausian[3][0], ppdT[i+3][j+1] * dGausian[3][1], ppdT[i+3][j+2] * dGausian[3][2], ppdT[i+3][j+3] * dGausian[3][3], ppdT[i+3][j+4] * dGausian[3][4],
                                ppdT[i+4][j] * dGausian[4][0], ppdT[i+4][j+1] * dGausian[4][1], ppdT[i+4][j+2] * dGausian[4][2], ppdT[i+4][j+3] * dGausian[4][3], ppdT[i+4][j+4] * dGausian[4][4]};
                double ddX = 0.0;
                double ddY = 0.0;
                double dXY = 0.0;

                for(int ii = i; ii < i+5; ii++)
                {
                    for(int jj = j; jj < j+5; jj++)
                    {
                        ddX += ppdX[ii][jj] * ppdX[ii][jj] * dGausian[ii-i][jj-j]/273;
                        ddY += ppdX[ii][jj] * ppdY[ii][jj] * dGausian[ii-i][jj-j]/273;
                        dXY += ppdY[ii][jj] * ppdY[ii][jj] * dGausian[ii-i][jj-j]/273;

                        A_T[0][5*(ii - i) + (jj - j)] = ppdX[ii][jj];
                        A_T[1][5*(ii - i) + (jj - j)] = ppdY[ii][jj];
    //                    Wb[5*(ii - i) + (jj - j)] = ppdT[ii][jj]*dGausian[ii][jj]/273;
                        AWb[0] += A_T[0][5*(ii - i) + (jj - j)] * Wb[5*(ii - i) + (jj - j)]/273;
                        AWb[1] += A_T[1][5*(ii - i) + (jj - j)] * Wb[5*(ii - i) + (jj - j)]/273;
                    }
                }



                double det = ddX*ddY - dXY*dXY;

                AWA[0][0] =  ddY / det;
                AWA[0][1] = -dXY / det;
                AWA[1][0] = -dXY / det;
                AWA[1][1] =  ddX / det;

                ppdV_X[i/5][j/5] = AWA[0][0] * AWb[0] + AWA[0][1] * AWb[1];
                ppdV_Y[i/5][j/5] = AWA[1][0] * AWb[0] + AWA[1][1] * AWb[1];
            }
        }

        for(int i = 0; i < m_nHeight ; i++)
        {
            delete ppdX[i];
            delete ppdY[i];
            delete ppdT[i];
        }
        delete[] ppdX;
        delete[] ppdY;
        delete[] ppdT;

        std::pair<double**, double**>* ReturnValue = new std::pair<double**, double**>(ppdV_X, ppdV_Y);

        KImageGray igTemp = q_pAfterForm->ImageGray();
        KImageColor icMain(igTemp.Row(),igTemp.Col());
        icMain    = igTemp.GrayToRGB();

        for(int i = 2; i < icMain.Row(); i+= 5)
        {
            for(int j = 2; j < icMain.Col(); j+= 5)
            {
                if(abs(ReturnValue->first[i/5][j/5]) < abs(ReturnValue->second[i/5][j/5]))
                {
                    for(int l = 0; l < abs(ReturnValue->second[i/5][j/5]); l++)
                    {
                        if(l > 7) break;

                        int r = i + (ReturnValue->second[i/5][j/5]   /  abs(ReturnValue->second[i/5][j/5]))*l;
                        int c = j + (ReturnValue->first[i/5][j/5]    /  abs(ReturnValue->second[i/5][j/5]))*l;

                        if((r >= 0 && r < icMain.Row()) && (c >= 0 && c < icMain.Col()))
                        {
                            icMain._ppA[r][c].r = 0;
                            icMain._ppA[r][c].g = 255;
                            icMain._ppA[r][c].b = 0;
                        }

                        else break;
                    }
                }
                else
                {
                    for(int l = 0; l < abs(ReturnValue->first[i/5][j/5]); l++)
                    {
                        if(l > 7) break;

                        int r = i + (ReturnValue->second[i/5][j/5]   /  abs(ReturnValue->first[i/5][j/5]))*l;
                        int c = j + (ReturnValue->first[i/5][j/5]    /  abs(ReturnValue->first[i/5][j/5]))*l;

                        if((r >= 0 && r < icMain.Row()) && (c >= 0 && c < icMain.Col()))
                        {
                            icMain._ppA[r][c].r = 0;
                            icMain._ppA[r][c].g = 255;
                            icMain._ppA[r][c].b = 0;
                        }

                        else break;
                    }
                }
            }
        }

        QString qstrName;
        if(nCnt < 16)
        {
            qstrName.sprintf("OpticalFlow (%d)->(%d)", nCnt, nCnt + 1);
        }
        else if(nCnt == 16)
        {
            qstrName.sprintf("OpticalFlow (%d)->(%d)", nCnt, nCnt - 15);
        }
        else
        {
            qstrName.sprintf("OpticalFlow (%d)->(%d)", nCnt - 16, nCnt - 15);
        }

        ImageForm*  q_pForm = new ImageForm(icMain, qstrName,this); //코너가 포함된 이미지 출력

        _plpImageForm->Add(q_pForm);
        q_pForm->show();

        //UI 활성화 갱신
        UpdateUI();

        for(int i = 0; i < icMain.Row()/5; i++)
        {
            delete ReturnValue->first[i];
            delete ReturnValue->second[i];
        }
        delete ReturnValue->first;
        delete ReturnValue->second;
        delete ReturnValue;
        //delete q_pBeforeForm;
        //delete q_pAfterForm;

#endif
#if 0
        //이미지 출력을 위한 ImageForm 생성
        ImageForm*              q_pForm   = new ImageForm(qstrAfterImg, "OPEN", this);

        _plpImageForm->Add(q_pForm);
        q_pForm->show();

        //UI 활성화 갱신
        UpdateUI();
    }
#endif
#ifdef TEST
    QString qstrBeforeImg = q_stFile;
    QString qstrAfterImg = q_stFile;
    QString qstrTemp;

    qstrTemp.sprintf("0%d.pgm",1);
    qstrBeforeImg.append(qstrTemp);
    qstrTemp.sprintf("0%d.pgm",);
    qstrAfterImg.append(qstrTemp);

    ImageForm* q_pBeforeForm   = new ImageForm(qstrBeforeImg, "OPEN", this);
    ImageForm* q_pAfterForm   = new ImageForm(qstrAfterImg, "OPEN", this);

    KImageGray igMain_before    = q_pBeforeForm->_igMain;
    KImageGray igMain_after     = q_pAfterForm->_igMain;

    double dSobel_X[3][3] = {{-1, 0, 1},
                             {-2, 0, 2},
                             {-1, 0, 1}};
    double dSobel_Y[3][3] = {{-1, -2, -1},
                             { 0,  0,  0},
                             { 1,  2,  1}};
    double dGausian[5][5] = {{1,  4,  7,  4, 1},
                             {4, 16, 26, 16, 4},
                             {7, 26, 41, 26, 7},
                             {4, 16, 26, 16, 4},
                             {1,  4,  7,  4, 1}};
    double** ppdX = new double*[m_nHeight];
    double** ppdY = new double*[m_nHeight];
    double** ppdT = new double*[m_nHeight];
    double** ppdV_X = new double*[m_nHeight/5];
    double** ppdV_Y = new double*[m_nHeight/5];

    for(int i = 0; i < m_nHeight ; i++)
    {
        ppdX[i] = new double[m_nWidth];
        ppdY[i] = new double[m_nWidth];
        ppdT[i] = new double[m_nWidth];

        if(i%5 == 0)
        {
            ppdV_X[i/5] = new double[m_nWidth/5];
            ppdV_Y[i/5] = new double[m_nWidth/5];
        }
    }


    // dX dY dT
    for(int i = 0; i < m_nHeight; i++)
    {
        for(int j = 0; j < m_nWidth; j++)
        {
            ppdT[i][j] = igMain_before._ppA[i][j] - igMain_after._ppA[i][j];    // Ix Iy = -It

            if(!(i == 0 || j == 0 || i == m_nHeight - 1 || j == m_nWidth - 1))
            {
                double dX    = (igMain_after._ppA[i-1][j-1] * dSobel_X[0][0] +
                                igMain_after._ppA[i-1][j]   * dSobel_X[0][1] +
                                igMain_after._ppA[i-1][j+1] * dSobel_X[0][2] +
                                igMain_after._ppA[i][j-1]   * dSobel_X[1][0] +
                                igMain_after._ppA[i][j]     * dSobel_X[1][1] +
                                igMain_after._ppA[i][j+1]   * dSobel_X[1][2] +
                                igMain_after._ppA[i+1][j-1] * dSobel_X[2][0] +
                                igMain_after._ppA[i+1][j]   * dSobel_X[2][1] +
                                igMain_after._ppA[i+1][j+1] * dSobel_X[2][2] ) / 8;
                double dY    = (igMain_after._ppA[i-1][j-1] * dSobel_Y[0][0] +
                                igMain_after._ppA[i-1][j]   * dSobel_Y[0][1] +
                                igMain_after._ppA[i-1][j+1] * dSobel_Y[0][2] +
                                igMain_after._ppA[i][j-1]   * dSobel_Y[1][0] +
                                igMain_after._ppA[i][j]     * dSobel_Y[1][1] +
                                igMain_after._ppA[i][j+1]   * dSobel_Y[1][2] +
                                igMain_after._ppA[i+1][j-1] * dSobel_Y[2][0] +
                                igMain_after._ppA[i+1][j]   * dSobel_Y[2][1] +
                                igMain_after._ppA[i+1][j+1] * dSobel_Y[2][2] ) / 8;
                ppdX[i][j] = dX;
                ppdY[i][j] = dY;
            }
            else
            {
                ppdX[i][j] = 0;
                ppdY[i][j] = 0;
            }
        }
    }


    for(int i = 1; i < m_nHeight - 1; i+= 5)
    {

        for(int j = 1; j < m_nWidth - 1; j += 5)
        {
            double AWA[2][2] = {0,};
            double AWb[2]   = {0,};
            double A_T[2][25] = {0,};
            double Wb[25]= {ppdT[i][j]   * dGausian[0][0], ppdT[i][j+1]   * dGausian[0][1], ppdT[i][j+2]   * dGausian[0][2], ppdT[i][j+3]   * dGausian[0][3], ppdT[i][j+4]   * dGausian[0][4],
                            ppdT[i+1][j] * dGausian[1][0], ppdT[i+1][j+1] * dGausian[1][1], ppdT[i+1][j+2] * dGausian[1][2], ppdT[i+1][j+3] * dGausian[1][3], ppdT[i+1][j+4] * dGausian[1][4],
                            ppdT[i+2][j] * dGausian[2][0], ppdT[i+2][j+1] * dGausian[2][1], ppdT[i+2][j+2] * dGausian[2][2], ppdT[i+2][j+3] * dGausian[2][3], ppdT[i+2][j+4] * dGausian[2][4],
                            ppdT[i+3][j] * dGausian[3][0], ppdT[i+3][j+1] * dGausian[3][1], ppdT[i+3][j+2] * dGausian[3][2], ppdT[i+3][j+3] * dGausian[3][3], ppdT[i+3][j+4] * dGausian[3][4],
                            ppdT[i+4][j] * dGausian[4][0], ppdT[i+4][j+1] * dGausian[4][1], ppdT[i+4][j+2] * dGausian[4][2], ppdT[i+4][j+3] * dGausian[4][3], ppdT[i+4][j+4] * dGausian[4][4]};
            double ddX = 0.0;
            double ddY = 0.0;
            double dXY = 0.0;

            for(int ii = i; ii < i+5; ii++)
            {
                for(int jj = j; jj < j+5; jj++)
                {
                    ddX += ppdX[ii][jj] * ppdX[ii][jj] * dGausian[ii-i][jj-j]/273;
                    ddY += ppdX[ii][jj] * ppdY[ii][jj] * dGausian[ii-i][jj-j]/273;
                    dXY += ppdY[ii][jj] * ppdY[ii][jj] * dGausian[ii-i][jj-j]/273;

                    A_T[0][5*(ii - i) + (jj - j)] = ppdX[ii][jj];
                    A_T[1][5*(ii - i) + (jj - j)] = ppdY[ii][jj];
//                    Wb[5*(ii - i) + (jj - j)] = ppdT[ii][jj]*dGausian[ii][jj]/273;
                    AWb[0] += A_T[0][5*(ii - i) + (jj - j)] * Wb[5*(ii - i) + (jj - j)]/273;
                    AWb[1] += A_T[1][5*(ii - i) + (jj - j)] * Wb[5*(ii - i) + (jj - j)]/273;
                }
            }



            double det = ddX*ddY - dXY*dXY;

            AWA[0][0] =  ddY / det;
            AWA[0][1] = -dXY / det;
            AWA[1][0] = -dXY / det;
            AWA[1][1] =  ddX / det;

            ppdV_X[i/5][j/5] = AWA[0][0] * AWb[0] + AWA[0][1] * AWb[1];
            ppdV_Y[i/5][j/5] = AWA[1][0] * AWb[0] + AWA[1][1] * AWb[1];
        }
    }

    for(int i = 0; i < m_nHeight ; i++)
    {
        delete ppdX[i];
        delete ppdY[i];
        delete ppdT[i];
    }
    delete ppdX;
    delete ppdY;
    delete ppdT;

    std::pair<double**, double**>* ReturnValue = new std::pair<double**, double**>(ppdV_X, ppdV_Y);

    KImageGray igTemp = q_pAfterForm->ImageGray();
    KImageColor icMain(igTemp.Row(),igTemp.Col());
    icMain    = igTemp.GrayToRGB();

    for(int i = 2; i < icMain.Row(); i+= 5)
    {
        for(int j = 2; j < icMain.Col(); j+= 5)
        {
            if(abs(ReturnValue->first[i/5][j/5]) < abs(ReturnValue->second[i/5][j/5]))
            {
                for(int l = 0; l < abs(ReturnValue->second[i/5][j/5]); l++)
                {
                    if(l > 7) break;

                    int r = i + (ReturnValue->second[i/5][j/5]   /  abs(ReturnValue->second[i/5][j/5]))*l;
                    int c = j + (ReturnValue->first[i/5][j/5]    /  abs(ReturnValue->second[i/5][j/5]))*l;

                    if((r >= 0 && r < icMain.Row()) && (c >= 0 && c < icMain.Col()))
                    {
                        icMain._ppA[r][c].r = 0;
                        icMain._ppA[r][c].g = 255;
                        icMain._ppA[r][c].b = 0;
                    }

                    else break;
                }
            }
            else
            {
                for(int l = 0; l < abs(ReturnValue->first[i/5][j/5]); l++)
                {
                    if(l > 7) break;

                    int r = i + (ReturnValue->second[i/5][j/5]   /  abs(ReturnValue->first[i/5][j/5]))*l;
                    int c = j + (ReturnValue->first[i/5][j/5]    /  abs(ReturnValue->first[i/5][j/5]))*l;

                    if((r >= 0 && r < icMain.Row()) && (c >= 0 && c < icMain.Col()))
                    {
                        icMain._ppA[r][c].r = 0;
                        icMain._ppA[r][c].g = 255;
                        icMain._ppA[r][c].b = 0;
                    }

                    else break;
                }
            }
        }
    }


    ImageForm*  q_pForm = new ImageForm(icMain, "Optical Test",this); //코너가 포함된 이미지 출력

    _plpImageForm->Add(q_pForm);
    q_pForm->show();

    //UI 활성화 갱신
    UpdateUI();

    for(int i = 0; i < icMain.Row()/5; i++)
    {
        delete ReturnValue->first[i];
        delete ReturnValue->second[i];
    }
    delete ReturnValue->first;
    delete ReturnValue->second;
    delete ReturnValue;
#endif
}


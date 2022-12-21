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

KImageColor g_noise, sp_noise;
void MainFrame::on_ButtonNoiseAdd_clicked()
{
    //포커스 된 ImageForm으로부터 영상을 가져옴
       KImageColor   icMain;
       if(_q_pFormFocused != 0 && _q_pFormFocused->ImageColor().Address() &&  _q_pFormFocused->ID() == "OPEN")
           icMain = _q_pFormFocused->ImageColor();
       else
           return;

       g_noise.Create(icMain.Row(),icMain.Col());
       sp_noise.Create(icMain.Row(),icMain.Col());
       sp_noise = icMain;

       //Gaussian Noise
       KImageGray _Img[3], g_noise_Img[3];

       for(int i=0; i<3; i++){
           _Img[i].Create(icMain.Row(), icMain.Col());
           g_noise_Img[i].Create(icMain.Row(), icMain.Col());
       }

       for(unsigned int nRow=0; nRow < icMain.Row(); nRow++){
           for(unsigned int nCol=0; nCol < icMain.Col(); nCol++){
               _Img[0][nRow][nCol] = icMain[nRow][nCol].r;
               _Img[1][nRow][nCol] = icMain[nRow][nCol].g;
               _Img[2][nRow][nCol] = icMain[nRow][nCol].b;
           }
       }

       double m[3]  = {20, 20, 20};
       double v[3]   = {100, 100, 100};

       for(int i=0; i<3; i++){
           KGaussian* GRandom = new KGaussian();
           GRandom->Create(m[i],v[i]);
           GRandom->OnRandom(icMain.Row() * icMain.Col());

           int pixel = 0;
           int noise = 0;

           for(unsigned int nRow=0; nRow < icMain.Row(); nRow++){
               for(unsigned int nCol=0; nCol < icMain.Col(); nCol++){
                   pixel = _Img[i][nRow][nCol];
                   noise = GRandom->Generate();

                   if(pixel + noise > 255) g_noise_Img[i][nRow][nCol] = 255;
                   else if(pixel + noise < 0) g_noise_Img[i][nRow][nCol] = 0;
                   else{
                       g_noise_Img[i][nRow][nCol] = pixel + noise;
                   }
               }
           }
       }

       for(unsigned int nRow=0; nRow < icMain.Row(); nRow++){
           for(unsigned int nCol=0; nCol < icMain.Col(); nCol++){
               g_noise[nRow][nCol].r = g_noise_Img[0][nRow][nCol];
               g_noise[nRow][nCol].g = g_noise_Img[1][nRow][nCol];
               g_noise[nRow][nCol].b = g_noise_Img[2][nRow][nCol];
           }
       }


       //Salt & Pepper Noise
       KRandom* random = new KRandom(icMain.Row() * icMain.Col());

       double r = 0;
       double t = 0.03;

       for(unsigned int i=0; i < icMain.Row(); i++){
           for(unsigned int j=0; j < icMain.Col(); j++){
               r = random->Generate();
               if(r < t){
                   sp_noise[i][j].r = 255;
                   sp_noise[i][j].g = 255;
                   sp_noise[i][j].b = 255;
               }
               else if(r > (1-t)){
                   sp_noise[i][j].r = 0;
                   sp_noise[i][j].g = 0;
                   sp_noise[i][j].b = 0;
               }
           }
       }

       //출력을 위한 ImageForm 생성
       ImageForm*  q_pForm_v = new ImageForm(g_noise, "Gaussian Noise", this);
       ImageForm*  q_pForm_v_2 = new ImageForm(sp_noise, "Salt & Pepper Noise", this);

       _plpImageForm->Add(q_pForm_v);
       q_pForm_v->show();
       _plpImageForm->Add(q_pForm_v_2);
       q_pForm_v_2->show();

       //UI 활성화 갱신
       UpdateUI();
}

void MainFrame::on_ButtonBox_clicked()
{
    //포커스 된 ImageForm으로부터 영상을 가져옴
     KImageColor g_b_filtered(g_noise.Row(), g_noise.Col());
     KImageColor sp_b_filtered(sp_noise.Row(), sp_noise.Col());

     g_b_filtered = g_noise;
     sp_b_filtered = sp_noise;

     const int Row = g_b_filtered.Row();
     const int Col = g_b_filtered.Col();

     double dSigma = ui->BoxSigma->text().toDouble();

        int nHalf = (int)(3.0*dSigma+0.3);
        int dScale = (nHalf*2 + 1)*(nHalf*2 + 1);

        int nSy = nHalf, nEy = Row - nHalf;
        int nSx = nHalf, nEx = Col - nHalf;
        double g_dR,g_dG,g_dB,sp_dR,sp_dG,sp_dB;

        for(int i=nSy; i<nEy; i++)
        {
            for(int j=nSx; j<nEx; j++)
            {
                g_dR=g_dG=g_dB=sp_dR=sp_dG=sp_dB=0.0;
                for(int a=-nHalf; a<=nHalf; a++)
                {
                    for(int b=-nHalf; b<=nHalf; b++)
                    {
                        g_dR += g_b_filtered[i+a][j+b].r;
                        g_dG += g_b_filtered[i+a][j+b].g;
                        g_dB += g_b_filtered[i+a][j+b].b;
                        sp_dR += sp_b_filtered[i+a][j+b].r;
                        sp_dG += sp_b_filtered[i+a][j+b].g;
                        sp_dB += sp_b_filtered[i+a][j+b].b;
                    }
                }
                g_b_filtered[i][j].r = g_dR/dScale;
                g_b_filtered[i][j].g = g_dG/dScale;
                g_b_filtered[i][j].b = g_dB/dScale;
                sp_b_filtered[i][j].r = sp_dR/dScale;
                sp_b_filtered[i][j].g = sp_dG/dScale;
                sp_b_filtered[i][j].b = sp_dB/dScale;
            }
        }
        //출력을 위한 ImageForm 생성
        ImageForm*  q_pForm1 = new ImageForm(g_b_filtered, "Box_Filter_Gaussian_Noise", this);
        ImageForm*  q_pForm2 = new ImageForm(sp_b_filtered, "Box_Filter_Salt&Pepper_Noise", this);

        _plpImageForm->Add(q_pForm1);
        q_pForm1->show();

        _plpImageForm->Add(q_pForm2);
        q_pForm2->show();

        //UI 활성화 갱신
        UpdateUI();
}

void MainFrame::on_ButtonGaussian_clicked()
{
        KImageColor g_g_filtered(g_noise.Row(), g_noise.Col());
        KImageColor sp_g_filtered(sp_noise.Row(), sp_noise.Col());

        int     i,j,r,c,rr,cc;
        double Sigma = ui->GaussianSigma->text().toDouble();

        //create the conv. mask
        int     Half = (int)(3.0*Sigma+0.3);
        KMatrix Mask(Half*2 + 1,Half*2 + 1);

        //obtain the mask
        double  Scale = 0.0, Sigma2 = 2.0*_SQR(Sigma);
        double  Const = 1.0 / (2.0*_PI*pow(Sigma, 2));

        for(r = -Half,i=0; r <= Half; r++,i++)
        {
             for(c = -Half,j=0; c <= Half; c++,j++)
             {
                Mask[i][j] = Const*exp(-(r*r+c*c)/Sigma2);
                Scale     += Mask[i][j];
             }
        }
        Mask /= Scale;

        KImageGray igTmp_r(g_noise.Row(),g_noise.Col()), igTmp_g(g_noise.Row(),g_noise.Col()), igTmp_b(g_noise.Row(),g_noise.Col());
        KImageGray igTmp_sp_r(sp_noise.Row(),sp_noise.Col()), igTmp_sp_g(sp_noise.Row(),sp_noise.Col()), igTmp_sp_b(sp_noise.Row(),sp_noise.Col());
        for(unsigned int i=0;i<g_noise.Row();i++)
            for(unsigned int j=0;j<g_noise.Col();j++){
                igTmp_r[i][j] = (int)g_noise[i][j].r;
                igTmp_g[i][j] = (int)g_noise[i][j].g;
                igTmp_b[i][j] = (int)g_noise[i][j].b;
            }
        for(unsigned int i=0;i<sp_noise.Row();i++)
            for(unsigned int j=0;j<sp_noise.Col();j++){
                igTmp_sp_r[i][j] = (int)sp_noise[i][j].r;
                igTmp_sp_g[i][j] = (int)sp_noise[i][j].g;
                igTmp_sp_b[i][j] = (int)sp_noise[i][j].b;
            }


        double Tmp_r, Tmp_g, Tmp_b;
        double Tmp_sp_r, Tmp_sp_g, Tmp_sp_b;

        for(int i=Half; i<g_noise.Row() - Half; i++)
        {
            for(int j=Half; j<g_noise.Col() - Half; j++)
            {
                //convolution
                Tmp_r = 0.0; Tmp_g = 0.0; Tmp_b = 0.0;
                Tmp_sp_r = 0.0; Tmp_sp_g = 0.0; Tmp_sp_b = 0.0;
                for(r = -Half,rr=0; r <= Half; r++,rr++){
                    for(c = -Half,cc=0; c <= Half; c++,cc++){
                        Tmp_r += igTmp_r[i+r][j+c] * Mask[rr][cc];
                        Tmp_g += igTmp_g[i+r][j+c] * Mask[rr][cc];
                        Tmp_b += igTmp_b[i+r][j+c] * Mask[rr][cc];
                        Tmp_sp_r += igTmp_sp_r[i+r][j+c] * Mask[rr][cc];
                        Tmp_sp_g += igTmp_sp_g[i+r][j+c] * Mask[rr][cc];
                        Tmp_sp_b += igTmp_sp_b[i+r][j+c] * Mask[rr][cc];
                    }
                }
                g_g_filtered[i][j].r = (unsigned char)Tmp_r;
                g_g_filtered[i][j].g = (unsigned char)Tmp_g;
                g_g_filtered[i][j].b = (unsigned char)Tmp_b;
                sp_g_filtered[i][j].r = (unsigned char)Tmp_sp_r;
                sp_g_filtered[i][j].g = (unsigned char)Tmp_sp_g;
                sp_g_filtered[i][j].b = (unsigned char)Tmp_sp_b;
            }
        }

        //출력을 위한 ImageForm 생성
        ImageForm*  q_pForm1 = new ImageForm(g_g_filtered, "Gaussian_Filter_Gaussian_Noise", this);
        ImageForm*  q_pForm2 = new ImageForm(sp_g_filtered, "Gaussian_Filter_Salt&Pepper_Noise", this);

        _plpImageForm->Add(q_pForm1);
        q_pForm1->show();

        _plpImageForm->Add(q_pForm2);
        q_pForm2->show();

        //UI 활성화 갱신
        UpdateUI();
}

void MainFrame::on_ButtonMedian_clicked()
{
    //포커스 된 ImageForm으로부터 영상을 가져옴
     KImageColor g_m_filtered(g_noise.Row(), g_noise.Col());
     KImageColor sp_m_filtered(sp_noise.Row(), sp_noise.Col());

     g_m_filtered = g_noise;
     sp_m_filtered = sp_noise;

     const int Row = g_m_filtered.Row();
     const int Col = g_m_filtered.Col();

     double nWindow = ui->MedianSize->text().toDouble();

     //median filtering
     int		nHalf = ((int)nWindow)/2;
     int		nMid  = _SQR((nHalf*2)+1)/2;
     KVector	g_vItem_r(_SQR((nHalf*2)+1)) , g_vItem_g(_SQR((nHalf*2)+1)) , g_vItem_b(_SQR((nHalf*2)+1));
     KVector	sp_vItem_r(_SQR((nHalf*2)+1)) , sp_vItem_g(_SQR((nHalf*2)+1)) , sp_vItem_b(_SQR((nHalf*2)+1));

     for(int i=nHalf; i<Row-nHalf; i++)
     {
         for(int j=nHalf; j<Col-nHalf; j++)
         {
             for(int m=-nHalf,k=0; m<=nHalf; m++)
             {
                 for(int n=-nHalf; n<=nHalf; n++)
                 {
                     g_vItem_r[k] = g_m_filtered[i+m][j+n].r;
                     g_vItem_g[k] = g_m_filtered[i+m][j+n].g;
                     g_vItem_b[k] = g_m_filtered[i+m][j+n].b;
                     sp_vItem_r[k] = sp_m_filtered[i+m][j+n].r;
                     sp_vItem_g[k] = sp_m_filtered[i+m][j+n].g;
                     sp_vItem_b[k] = sp_m_filtered[i+m][j+n].b;
                     k++;
                 }
             }
             //sort
             g_vItem_r.Sorted(_ASCEND);
             g_vItem_g.Sorted(_ASCEND);
             g_vItem_b.Sorted(_ASCEND);
             sp_vItem_r.Sorted(_ASCEND);
             sp_vItem_g.Sorted(_ASCEND);
             sp_vItem_b.Sorted(_ASCEND);
             //set the median value
             g_m_filtered[i][j].r = g_vItem_r[nMid];
             g_m_filtered[i][j].g = g_vItem_g[nMid];
             g_m_filtered[i][j].b = g_vItem_b[nMid];
             sp_m_filtered[i][j].r = sp_vItem_r[nMid];
             sp_m_filtered[i][j].g = sp_vItem_g[nMid];
             sp_m_filtered[i][j].b = sp_vItem_b[nMid];
         }
     }

     //출력을 위한 ImageForm 생성
     ImageForm*  q_pForm1 = new ImageForm(g_m_filtered, "Median_Filter_Gaussian_Noise", this);
     ImageForm*  q_pForm2 = new ImageForm(sp_m_filtered, "Median_Filter_Salt&Pepper_Noise", this);

     _plpImageForm->Add(q_pForm1);
     q_pForm1->show();

     _plpImageForm->Add(q_pForm2);
     q_pForm2->show();

     //UI 활성화 갱신
     UpdateUI();
}

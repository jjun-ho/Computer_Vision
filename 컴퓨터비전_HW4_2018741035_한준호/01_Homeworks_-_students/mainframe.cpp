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

void MainFrame::on_HEQ_clicked()
{
    //포커스 된 ImageForm으로부터 영상을 가져옴
    KImageColor   icMain;
    if(_q_pFormFocused != 0 && _q_pFormFocused->ImageColor().Address() &&  _q_pFormFocused->ID() == "OPEN")
        icMain = _q_pFormFocused->ImageColor();
    else
        return;

    KImageGray r(icMain.Row(), icMain.Col()), g(icMain.Row(), icMain.Col()), b(icMain.Row(), icMain.Col());

    for(unsigned int i=0;i<icMain.Row();i++)
        for(unsigned int j=0;j<icMain.Col();j++){
            r[i][j] = (int)icMain[i][j].r;
            g[i][j] = (int)icMain[i][j].g;
            b[i][j] = (int)icMain[i][j].b;
        }

    //histograming according to intensities
    KHisto ht_r, ht_g, ht_b;
    ht_r.Histogram(r);
    ht_g.Histogram(g);
    ht_b.Histogram(b);

    //accumulated histogram
    double*  dpHisto_r = ht_r.Address();
    double*  dpHisto_g = ht_g.Address();
    double*  dpHisto_b = ht_b.Address();
    for(int i=1, ic=ht_r.Size()-1; ic; ic--, i++){
        dpHisto_r[i] += dpHisto_r[i-1];
        dpHisto_g[i] += dpHisto_g[i-1];
        dpHisto_b[i] += dpHisto_b[i-1];
    }

    //Histogram Equalization
    for(int i=0, ic=ht_r.Size(); ic; ic--, i++){
        dpHisto_r[i] = dpHisto_r[i] / (double)(r.Size());
        dpHisto_g[i] = dpHisto_g[i] / (double)(g.Size());
        dpHisto_b[i] = dpHisto_b[i] / (double)(b.Size());
    }

    //transform the original image
    for(int ic=icMain.Row(),i=0; ic; ic--,i++)
        for(int jc=icMain.Col(),j=0; jc; jc--,j++){
            icMain[i][j].r = (unsigned char)(dpHisto_r[r[i][j]]*255.0);
            icMain[i][j].g = (unsigned char)(dpHisto_g[g[i][j]]*255.0);
            icMain[i][j].b = (unsigned char)(dpHisto_b[b[i][j]]*255.0);
        }

    memset(dpHisto_r,0,sizeof(double)*ht_r.Size());
    memset(dpHisto_g,0,sizeof(double)*ht_g.Size());
    memset(dpHisto_b,0,sizeof(double)*ht_b.Size());
    for(int ic=icMain.Row(),i=0; ic; ic--,i++)
        for(int jc=icMain.Col(),j=0; jc; jc--,j++){
                dpHisto_r[r[i][j]] ++;
                dpHisto_g[g[i][j]] ++;
                dpHisto_b[b[i][j]] ++;
        }


    //출력을 위한 ImageForm 생성
    ImageForm*  q_pForm_v = new ImageForm(icMain, "Histogram Equalization", this);

    _plpImageForm->Add(q_pForm_v);
    q_pForm_v->show();

    //UI 활성화 갱신
    UpdateUI();
}

KImageColor HM_S1, HM_S2;


void MainFrame::on_HM_Source1_clicked()
{
    //포커스 된 ImageForm으로부터 영상을 가져옴
    KImageColor   icMain_s1;
    if(_q_pFormFocused != 0 && _q_pFormFocused->ImageColor().Address() &&  _q_pFormFocused->ID() == "OPEN")
        icMain_s1 = _q_pFormFocused->ImageColor();
    else
        return;

    HM_S1 = icMain_s1;
}

void MainFrame::on_HM_Source2_clicked()
{
    //포커스 된 ImageForm으로부터 영상을 가져옴
    KImageColor   icMain_s2;
    if(_q_pFormFocused != 0 && _q_pFormFocused->ImageColor().Address() &&  _q_pFormFocused->ID() == "OPEN")
        icMain_s2 = _q_pFormFocused->ImageColor();
    else
        return;

    HM_S2 = icMain_s2;
}

void MainFrame::on_HM_Out_clicked()
{
    KImageGray r_r(HM_S1.Row(), HM_S1.Col()), r_g(HM_S1.Row(), HM_S1.Col()), r_b(HM_S1.Row(), HM_S1.Col());
    KImageGray s_r(HM_S2.Row(), HM_S2.Col()), s_g(HM_S2.Row(), HM_S2.Col()), s_b(HM_S2.Row(), HM_S2.Col());

    for(unsigned int i=0;i<HM_S1.Row();i++)
        for(unsigned int j=0;j<HM_S1.Col();j++){
            r_r[i][j] = (int)HM_S1[i][j].r;
            r_g[i][j] = (int)HM_S1[i][j].g;
            r_b[i][j] = (int)HM_S1[i][j].b;
        }

    for(unsigned int i=0;i<HM_S2.Row();i++)
        for(unsigned int j=0;j<HM_S2.Col();j++){
            s_r[i][j] = (int)HM_S2[i][j].r;
            s_g[i][j] = (int)HM_S2[i][j].g;
            s_b[i][j] = (int)HM_S2[i][j].b;
        }

    KHisto ht_r_r, ht_r_g, ht_r_b, ht_s_r, ht_s_g, ht_s_b;
    ht_r_r.Histogram(r_r);
    ht_r_g.Histogram(r_g);
    ht_r_b.Histogram(r_b);

    ht_s_r.Histogram(s_r);
    ht_s_g.Histogram(s_g);
    ht_s_b.Histogram(s_b);

    double *Pr_r = ht_r_r.Address();
    double *Pr_g = ht_r_g.Address();
    double *Pr_b = ht_r_b.Address();

    double *Ps_r = ht_s_r.Address();
    double *Ps_g = ht_s_g.Address();
    double *Ps_b = ht_s_b.Address();

    for(int i=0, ic=ht_r_r.Size(); ic; ic--, i++)
    {
        Pr_r[i] = ht_r_r[i] / (double)r_r.Size();
        Pr_g[i] = ht_r_g[i] / (double)r_g.Size();
        Pr_b[i] = ht_r_b[i] / (double)r_b.Size();
    }

    for(int i=0, ic=ht_s_r.Size(); ic; ic--, i++)
    {
        Ps_r[i] = ht_s_r[i] / (double)s_r.Size();
        Ps_g[i] = ht_s_g[i] / (double)s_g.Size();
        Ps_b[i] = ht_s_b[i] / (double)s_b.Size();
    }

    for(int i=1, ic=ht_r_r.Size(); ic; ic--, i++)
    {
        Pr_r[i] += Pr_r[i-1];
        Pr_g[i] += Pr_g[i-1];
        Pr_b[i] += Pr_b[i-1];
    }

    for(int i=1, ic=ht_s_r.Size(); ic; ic--, i++)
    {
        Ps_r[i] += Ps_r[i-1];
        Ps_g[i] += Ps_g[i-1];
        Ps_b[i] += Ps_b[i-1];
    }

    KImageColor output(HM_S1.Row(), HM_S1.Col());
    int *ht_final_r = new int[HM_S1.Size()];
    int *ht_final_g = new int[HM_S1.Size()];
    int *ht_final_b = new int[HM_S1.Size()];

    int min_r_i, min_g_i, min_b_i;
    double min_r_Y, min_g_Y, min_b_Y;

    for(unsigned int i=0;i<ht_r_r.Size();i++)
    {
        min_r_i = 256; min_g_i = 256; min_b_i = 256;
        min_r_Y = 1.0; min_g_Y = 1.0; min_b_Y = 1.0;

        for(unsigned int j=0;j<ht_s_r.Size();j++){
            if (min_r_Y > abs(Pr_r[i] - Ps_r[j])){
                min_r_Y = abs(Pr_r[i] - Ps_r[j]);
                min_r_i = j;
            }
        }
        for(unsigned int j=0;j<ht_s_g.Size();j++){
            if (min_g_Y > abs(Pr_g[i] - Ps_g[j])){
                min_g_Y = abs(Pr_g[i] - Ps_g[j]);
                min_g_i = j;
            }
        }
        for(unsigned int j=0;j<ht_s_b.Size();j++){
            if (min_b_Y > abs(Pr_b[i] - Ps_b[j])){
                min_b_Y = abs(Pr_b[i] - Ps_b[j]);
                min_b_i = j;
            }
        }
        ht_final_r[i] = min_r_i;
        ht_final_g[i] = min_g_i;
        ht_final_b[i] = min_b_i;
    }

    for(int ic=output.Row(),i=0; ic; ic--,i++)
        for(int jc=output.Col(),j=0; jc; jc--,j++){
            output[i][j].r = ht_final_r[r_r[i][j]];
            output[i][j].g = ht_final_g[r_g[i][j]];
            output[i][j].b = ht_final_b[r_b[i][j]];
        }

    memset(Pr_r,0,sizeof(double)*ht_r_r.Size());
    memset(Pr_g,0,sizeof(double)*ht_r_g.Size());
    memset(Pr_b,0,sizeof(double)*ht_r_b.Size());
    for(int ic=HM_S1.Row(),i=0; ic; ic--,i++)
        for(int jc=HM_S2.Col(),j=0; jc; jc--,j++){
                Pr_r[r_r[i][j]] ++;
                Pr_g[r_g[i][j]] ++;
                Pr_b[r_b[i][j]] ++;
        }

    //출력을 위한 ImageForm 생성
    ImageForm*  q_pForm_v = new ImageForm(output, "Histogram Matching", this);

    _plpImageForm->Add(q_pForm_v);
    q_pForm_v->show();

    //UI 활성화 갱신
    UpdateUI();
}

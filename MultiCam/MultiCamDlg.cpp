
// MultiCamDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "MultiCam.h"
#include "MultiCamDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#pragma region �������ӵ��ļ��ͺ궨��

#include <opencv2\opencv.hpp>
#include "CvvImage.h"
#define MAX_CORNERS 500

CvCapture *capture1,*capture2;
IplImage  *m_Frame1,*m_Frame2,*Img_zh,*outImgDetect,
    *hsv, *HChannel, *SChannel,*VChannel,*screenImg;
CvvImage  m_CvvImage1,m_CvvImage2,m_CvvImage3;
CDC *pDC1,*pDC2;
HDC hDC1,hDC2,hDC3,hDCDetect;
CRect rect1,rect2,rect3,rectDetect,rectXY,rectRgnXY;
float factorWCam,factorHCam;
CWnd *pWnd;
CRgn rgn;
CvVideoWriter *writer;
int isColor;
double fps ;

#pragma endregion



// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialogEx
{
public:
    CAboutDlg();

    // �Ի�������
    enum { IDD = IDD_ABOUTBOX };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

    // ʵ��
protected:
    DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CMultiCamDlg �Ի���

CMultiCamDlg::CMultiCamDlg(CWnd* pParent /*=NULL*/)
    : CDialogEx(CMultiCamDlg::IDD, pParent)
    , frameCount1(0)
    , frameCount2(0)
    , isLDownCam(false)
    , isVedioStitching(false)
    , isDetectEdge(false)
    , isDetectCorner(false)
    , pointCam(0)
    , nPort(0)
    , isTrackObject(0)
    , isLDownTrack(false)
    , isSelectObject(false)
	, widthDlg(0)
	, heightDlg(0)
{
    m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMultiCamDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_FRAME_COUNT1, frameCount1);
    DDX_Text(pDX, IDC_FRAME_COUNT2, frameCount2);
    DDX_Text(pDX, IDC_EDIT_PORT, nPort);
}

BEGIN_MESSAGE_MAP(CMultiCamDlg, CDialogEx)
    ON_WM_SYSCOMMAND()
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()

    ON_MESSAGE(WM_RECVDATA,&CMultiCamDlg::OnRecvData) //�Զ�����ϢWM_RECVDATA����Ϣӳ��

    ON_BN_CLICKED(IDC_OPEN_CAM1, &CMultiCamDlg::OnClickedOpenCam1)
    ON_BN_CLICKED(IDC_CLOSE_CAM1, &CMultiCamDlg::OnClickedCloseCam1)
    ON_WM_TIMER()
    ON_BN_CLICKED(IDC_OPEN_CAM2, &CMultiCamDlg::OnClickedOpenCam2)
    ON_BN_CLICKED(IDC_CLOSE_CAM2, &CMultiCamDlg::OnClickedCloseCam2)
    //    ON_BN_CLICKED(IDC_GRAB_FRAME1, &CMultiCamDlg::OnBnClickedGrabFrame1)
    ON_BN_CLICKED(IDC_GRAB_FRAME2, &CMultiCamDlg::OnClickedGrabFrame2)
    ON_BN_CLICKED(IDC_VIDEO_STITCH, &CMultiCamDlg::OnBnClickedVideoStitch)
    ON_BN_CLICKED(IDC_GRAB_FRAME, &CMultiCamDlg::OnClickedGrabFrame)
    //    ON_WM_LBUTTONDOWN()
    //ON_STN_CLICKED(IDC_VEDIO_STITCHING, &CMultiCamDlg::OnClickedVedioStitching)
    ON_BN_CLICKED(IDC_EDGE_DETECT, &CMultiCamDlg::OnBnClickedEdgeDetect)
    ON_WM_LBUTTONDOWN()
    //ON_STN_CLICKED(IDC_VEDIO_STITCHING, &CMultiCamDlg::OnStnClickedVedioStitching)
    ON_BN_CLICKED(IDC_GRAB_FRAME1, &CMultiCamDlg::OnClickedGrabFrame1)
    ON_BN_CLICKED(IDC_CORNER_DETECT, &CMultiCamDlg::OnBnClickedCornerDetect)
    ON_BN_CLICKED(IDC_BEGIN_LOCATE, &CMultiCamDlg::OnBnClickedBeginLocate)
    ON_BN_CLICKED(IDC_BTN_SEND, &CMultiCamDlg::OnBnClickedBtnSend)
    ON_COMMAND(IDM_SCREEN_SHOT, &CMultiCamDlg::OnScreenShot)
    ON_COMMAND(IDM_SCREEN_VIDEO, &CMultiCamDlg::OnScreenVideo)
    ON_BN_CLICKED(IDC_BEGIN_TRACK, &CMultiCamDlg::OnBnClickedBeginTrack)
    ON_WM_MOUSEMOVE()
    ON_WM_LBUTTONUP()
END_MESSAGE_MAP()


// CMultiCamDlg ��Ϣ�������

BOOL CMultiCamDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    // ��������...���˵�����ӵ�ϵͳ�˵��С�

    // IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
    ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
    ASSERT(IDM_ABOUTBOX < 0xF000);

    CMenu* pSysMenu = GetSystemMenu(FALSE);
    if (pSysMenu != NULL)
    {
        BOOL bNameValid;
        CString strAboutMenu;
        bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
        ASSERT(bNameValid);
        if (!strAboutMenu.IsEmpty())
        {
            pSysMenu->AppendMenu(MF_SEPARATOR);
            pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
        }
    }

    // ���ô˶Ի����ͼ�ꡣ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
    //  ִ�д˲���
    SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
    SetIcon(m_hIcon, FALSE);		// ����Сͼ��

    // TODO: �ڴ���Ӷ���ĳ�ʼ������

    //��������ʼ���׽���
    InitSocket();
    RECVPARAM *pRecvParam = new RECVPARAM;
    pRecvParam->sock = m_socket;
    pRecvParam->hwnd = m_hWnd;
    //�����������ݵ��߳�
    HANDLE hThread = CreateThread(NULL,0,RecvProcThread,(LPVOID)pRecvParam,0,NULL);
    //�رոý����߳̾�����ͷ������ü���
    CloseHandle(hThread);

	//���öԻ���Ĵ�С��λ��
	// ::SetWindowPos(this->m_hWnd,HWND_BOTTOM,50,10,800,600,SWP_NOZORDER);
	// CenterWindow(this); //������ʾ�Ի���

	//��ȡ�Ի���ĸ߶ȺͿ��
	CRect rectDlg;
	GetWindowRect(rectDlg);
	widthDlg = rectDlg.Width();
	heightDlg = rectDlg.Height();
	/*
	CWnd *pwnd;
	pwnd = GetDlgItem(IDC_CAM1);
	pwnd->MoveWindow(10,10,240,320);
	*/
    return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

void CMultiCamDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
    if ((nID & 0xFFF0) == IDM_ABOUTBOX)
    {
        CAboutDlg dlgAbout;
        dlgAbout.DoModal();
    }
    else
    {
        CDialogEx::OnSysCommand(nID, lParam);
    }
}

// �����Ի��������С����ť������Ҫ����Ĵ���
// �����Ƹ�ͼ�ꡣ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
// �⽫�ɿ���Զ���ɡ�

void CMultiCamDlg::OnPaint()
{
    if (IsIconic())
    {
        CPaintDC dc(this); // ���ڻ��Ƶ��豸������

        SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

        // ʹͼ���ڹ����������о���
        int cxIcon = GetSystemMetrics(SM_CXICON);
        int cyIcon = GetSystemMetrics(SM_CYICON);
        CRect rect;
        GetClientRect(&rect);
        int x = (rect.Width() - cxIcon + 1) / 2;
        int y = (rect.Height() - cyIcon + 1) / 2;

        // ����ͼ��
        dc.DrawIcon(x, y, m_hIcon);
    }
    else
    {
        CDialogEx::OnPaint();
    }


    /**************************************************************
    *             ��IDC_COORDINATE�ؼ�������ͼ���ϻ�ͼ            *
    **************************************************************/

    //��ȡIDC_COORDINATE�ؼ���DC������
    CWnd *pWnd  = GetDlgItem(IDC_COORDINATE);
    CDC *pDC    = pWnd->GetDC();
    CRect rect;
    pWnd->GetClientRect(&rect);

    /***********  ��IDC_COORDINATE�ؼ�������ͼ���ϻ�����ͼ  *************/
    //�����ߡ�����
    CPen Pen(PS_SOLID,0,RGB(255,0,0));
    CPen *pPen  = pDC->SelectObject(&Pen);
    for(int row=20; row<rect.Height(); row+=20)
        for(int col=20; col<rect.Width(); col+=20)
        {
            pDC->MoveTo(0,row);
            pDC->LineTo(rect.Width()-rect.Width()%20,row);
            pDC->MoveTo(col,0);
            pDC->LineTo(col,rect.Height()-rect.Height()%20);
        }
    //��������
    CPen PenXY(PS_JOIN_BEVEL,3,RGB(255,0,0));
    pDC->SelectObject(&PenXY);
    //��Y������
    pDC->MoveTo( 0, 0);
    pDC->LineTo( 0, rect.Height()-5);
    pDC->LineTo( 3, rect.Height()-5);//����ͷ
    pDC->LineTo( 0, rect.Height()  );
    pDC->LineTo(-3, rect.Height()-5);
    pDC->LineTo( 0, rect.Height()-5);
    //��X������
    pDC->MoveTo(0,0);
    pDC->LineTo(rect.Width(),  0);
    pDC->LineTo(rect.Width(),  3); //����ͷ
    pDC->LineTo(rect.Width()+5,0);
    pDC->LineTo(rect.Width(), -3);
    pDC->LineTo(rect.Width(),  0);
    //��ԭ��
    pDC->Ellipse(-3,-3,+3,+3);

    /*******  ����ɫ��IDC_COORDINATE�ؼ�������ͼ���ϱ��������λ��  ********/
    if(isLDownCam)
    {   
        pointXY = pointCam;
        CPen PenMark(PS_JOIN_BEVEL,3,RGB(0,0,255));
        pDC->SelectObject(&PenMark);
        pDC->Ellipse(pointXY.x-5,pointXY.y-5,pointXY.x+5,pointXY.y+5);
    }

    //��ԭ����ѡ���豸���������ͷ�DC
    pDC->SelectObject(pPen);
    ReleaseDC(pDC);
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CMultiCamDlg::OnQueryDragIcon()
{
    return static_cast<HCURSOR>(m_hIcon);
}


/*********************   ����ͷ1����  **********************/
void CMultiCamDlg::OnClickedOpenCam1()
{
    // TODO: �ڴ���ӿؼ�֪ͨ����������
    HWND pParam = m_hWnd; //����������ͷ1���߳�
    HANDLE hThreadCam1 = CreateThread(NULL, 0, OpenCam1Thread, (LPVOID)pParam, 0, NULL);
    CloseHandle(hThreadCam1);//�ر��߳̾���������̶߳����ʹ�ü��� 

    // �õ� hDC2 �� rect2
    CWnd *pWnd = GetDlgItem(IDC_CAM1);
    pDC1 = pWnd->GetDC();
    hDC1 = pDC1->GetSafeHdc();
    pWnd->GetClientRect(&rect1);
}
//������ͷ1���߳�
DWORD WINAPI CMultiCamDlg::OpenCam1Thread(LPVOID lpParam)
{
    capture1 = cvCaptureFromCAM(0);
    //����ÿһ֡�Ŀ�Ⱥ͸߶�
    cvSetCaptureProperty(capture1, CV_CAP_PROP_FRAME_WIDTH,  320);
    cvSetCaptureProperty(capture1, CV_CAP_PROP_FRAME_HEIGHT, 240);

    //�ж��Ƿ��������ͷ1
    if(capture1)
        ::SetTimer((HWND)lpParam, 1, 50, NULL);
    else
        AfxMessageBox(_T("����ͷ1��ʧ�ܣ�"));
    return 0;
}
void CMultiCamDlg::OnClickedGrabFrame1()
{
    // TODO: �ڴ���ӿؼ�֪ͨ����������
    cvSaveImage("..\\Output\\img1.jpg",m_Frame1);
}
void CMultiCamDlg::OnClickedCloseCam1()
{
    // TODO: �ڴ���ӿؼ�֪ͨ����������
    KillTimer(1);
    KillTimer(3); 
    //cvReleaseImage(&m_Frame1);
    cvReleaseCapture(&capture1);

    CBrush *pBrush = CBrush::FromHandle((HBRUSH)GetStockObject(GRAY_BRUSH));
    pDC1->FillRect(&rect1,pBrush);
}

/*********************   ����ͷ2����  **********************/
void CMultiCamDlg::OnClickedOpenCam2()
{
    // TODO: �ڴ���ӿؼ�֪ͨ����������
    HWND pParam = m_hWnd; //����������ͷ2���߳�
    HANDLE hThreadCam2 = CreateThread(NULL, 0, OpenCam2Thread, (LPVOID)pParam, 0, NULL);
    CloseHandle(hThreadCam2);//�ر��߳̾���������̶߳����ʹ�ü��� 

    // �õ� hDC2 �� rect2
    CWnd *pWnd = FromHandle((HWND)pParam)->GetDlgItem(IDC_CAM2);
    pDC2 = pWnd->GetDC();
    hDC2 = pDC2->GetSafeHdc();
    pWnd->GetClientRect(&rect2);
}
//������ͷ2���߳�
DWORD  WINAPI CMultiCamDlg::OpenCam2Thread(LPVOID lpParam)
{
    capture2 = cvCaptureFromCAM(1);
    //����ÿһ֡�Ŀ�Ⱥ͸߶�
    cvSetCaptureProperty(capture2, CV_CAP_PROP_FRAME_WIDTH,  320);
    cvSetCaptureProperty(capture2, CV_CAP_PROP_FRAME_HEIGHT, 240);

    //�ж��Ƿ��������ͷ2
    if(capture2)
        ::SetTimer((HWND)lpParam, 2, 50, NULL);
    else
        AfxMessageBox(_T("����ͷ2��ʧ�ܣ�"));
    return 0;
}
void CMultiCamDlg::OnClickedGrabFrame2()
{
    // TODO: �ڴ���ӿؼ�֪ͨ����������
    cvSaveImage("..\\Output\\img2.jpg",m_Frame2);
}
void CMultiCamDlg::OnClickedCloseCam2()
{
    // TODO: �ڴ���ӿؼ�֪ͨ����������
    KillTimer(2);
    KillTimer(3);
    cvReleaseCapture(&capture2);
    //cvReleaseImage(&m_Frame2);

    //��ʾλͼ������
    CBitmap m_Bitmap;
    m_Bitmap.LoadBitmap(IDB_GRAY);
    CDC dcCompatibleDC;
    dcCompatibleDC.CreateCompatibleDC(NULL);
    dcCompatibleDC.SelectObject(&m_Bitmap);
    BITMAP bmp;
    m_Bitmap.GetBitmap(&bmp);
    pDC2->StretchBlt(rect2.left,rect2.top,rect2.Width(),rect2.Height(),
        &dcCompatibleDC,0,0,bmp.bmWidth,bmp.bmHeight,SRCCOPY);
    m_Bitmap.DeleteObject();
    dcCompatibleDC.DeleteDC();
}

/****************** ��Ƶƴ�� *********************/
void CMultiCamDlg::OnBnClickedVideoStitch()
{
    // TODO: �ڴ���ӿؼ�֪ͨ����������

    CWnd *pWnd = GetDlgItem(IDC_VEDIO_STITCHING);
    CDC *pDC = pWnd->GetDC();
    hDC3 = pDC->GetSafeHdc();      
    pWnd->GetClientRect(rect3);

    if(capture1 && capture2)//�ж���������ͷ�Ƿ�ȫ����
    {
        isVedioStitching = true;
        SetTimer(3, 50, NULL);
    }
    else
        MessageBox(_T("�������ͷ��"));
}
void CMultiCamDlg::OnClickedGrabFrame()
{
    // TODO: �ڴ���ӿؼ�֪ͨ����������
    cvSaveImage("..\\Output\\img_zh.jpg",Img_zh);//��ƴ����Ƶ�ϱ���һ֡ͼƬ
}

/*********************  ��Ե���  *************************/
void CMultiCamDlg::OnBnClickedEdgeDetect()
{
    // TODO: �ڴ���ӿؼ�֪ͨ����������
    
    CWnd *pWnd4 = GetDlgItem(IDC_EDGE_DETECTING);
    CDC *pDC4 = pWnd4->GetDC();
    hDCDetect = pDC4->GetSafeHdc();
    pWnd4->GetClientRect(&rectDetect);

    isDetectEdge    = true;
    isDetectCorner  = false;
}
/*********************  �ǵ���  *************************/
void CMultiCamDlg::OnBnClickedCornerDetect()
{
    // TODO: �ڴ���ӿؼ�֪ͨ����������
    CWnd *pWnd4 = GetDlgItem(IDC_EDGE_DETECTING);
    CDC *pDC4 = pWnd4->GetDC();
    hDCDetect = pDC4->GetSafeHdc();
    pWnd4->GetClientRect(&rectDetect);

    isDetectEdge    = false;
    isDetectCorner  = true;
}

/******************  ��ʼ��λ  *******************/
void CMultiCamDlg::OnBnClickedBeginLocate()
{
    // TODO: �ڴ���ӿؼ�֪ͨ����������
    isLDownCam = true;
    CRect rectCam;
    GetDlgItem(IDC_VEDIO_STITCHING)->GetWindowRect(&rectCam);
    ScreenToClient(rectCam);
	factorWCam = 320/(float)rectCam.Width();
	factorHCam = 480/(float)rectCam.Height();

    SetTimer(5, 1000, NULL);
}

/******************  ��Ļ¼��  *******************/
void CMultiCamDlg::ScreenVideo(void)
{
    SetTimer(6,100,NULL);
}
void CMultiCamDlg::OnScreenVideo()
{
    // TODO: �ڴ���������������
    ScreenVideo();
}

/******************  ��ʼ����  *******************/
int b_flagTracking=0;

IplImage *imageTrack, *hsvTrack, *hueTrack, 
    *maskTrack, *histimgTrack,*backprojectTrack;
CvHistogram *histTrack;
CvRect selection;
CvRect track_window;
CvPoint origin;
CvConnectedComp track_comp;
CvBox2D track_box; // Meanshift�����㷨���ص�Box��

int hdims = 50; // ����ֱ��ͼbins�ĸ�����Խ��Խ��ȷ
float hranges_arr[] = {0,180};//����ֵ�ķ�Χ
float* hranges = hranges_arr;//���ڳ�ʼ��CvHistogram��
float factorWTrack,factorHTrack;
int backproject_mode = 0;

HDC hDCTrack;
CRect rectTrack;
CvvImage imgTrack;
void CMultiCamDlg::OnBnClickedBeginTrack()
{
    // TODO: �ڴ���ӿؼ�֪ͨ����������
    CWnd *pWnd = GetDlgItem(IDC_TRACK);
    CDC *pDCTrack = pWnd->GetDC();
    hDCTrack = pDCTrack->GetSafeHdc();
    pWnd->GetClientRect(&rectTrack);

	factorWTrack = 320/(float)rectTrack.Width();
	factorHTrack = 480/(float)rectTrack.Height();

	b_flagTracking=1;

    imageTrack=0;
    if (!imageTrack)
    {
        imageTrack  = cvCreateImage( cvGetSize(Img_zh), 8, 3 );
        //imageTrack->origin = Img_zh->origin;
        hsvTrack    = cvCreateImage( cvGetSize(Img_zh), 8, 3 );
        hueTrack    = cvCreateImage( cvGetSize(Img_zh), 8, 1 );
        maskTrack   = cvCreateImage( cvGetSize(Img_zh), 8, 1 );//������Ĥͼ��ռ�
        //���䷴��ͶӰͼ�ռ䣬��Сһ������ͨ��
        backprojectTrack = cvCreateImage( cvGetSize(Img_zh), 8, 1 );

        //���佨��ֱ��ͼ�ռ�
        histTrack = cvCreateHist( 1, &hdims, CV_HIST_ARRAY, &hranges, 1 ); 
        //�������ڻ�ֱ��ͼ�Ŀռ�
        histimgTrack = cvCreateImage( cvSize(320,240), 8, 3 );
        cvZero( histimgTrack );//��ֱ��ͼ�ı�����Ϊ��ɫ
    }
    SetTimer(7,100,NULL);
}

/******************   ��ʱ��   **************************/
void CMultiCamDlg::OnTimer(UINT_PTR nIDEvent)
{
    // TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
    switch(nIDEvent)
    {
    case 1:     //��ʾ����ͷ1
        m_Frame1 = cvQueryFrame(capture1);
        m_CvvImage1.CopyOf(m_Frame1, 1);
        m_CvvImage1.DrawToHDC(hDC1, &rect1);
        frameCount1++;
        UpdateData(false);
        break;
    case 2:     //��ʾ����ͷ2
        m_Frame2 = cvQueryFrame(capture2);
        m_CvvImage2.CopyOf(m_Frame2, 1);
        m_CvvImage2.DrawToHDC(hDC2, &rect2);
        frameCount2++;
        UpdateData(false);
        break;
    case 3:     //��Ƶƴ�Ӳ���ʾ
        IplImage* IMG;
        CvSize size;
        BYTE* RGB;
        BYTE* rgb1, *rgb2;
        IMG=new IplImage;
        size=cvSize(320,480);	    
        IMG=cvCreateImage(size,8,3);        
        RGB=new BYTE[320*480*3];
        for (int w=0;w<320;w++)
            for(int h=0;h<480;h++)
                for (int k=0;k<3;k++)
                    RGB[h*320*3+w*3+k]=0;			

        //      IplImage* img1;
        //    	img1=cvLoadImage("img1.jpg",-1);
        rgb1=new BYTE[320*240*3];
        memcpy(rgb1,m_Frame1->imageData,320*240*3);	

        //      IplImage* img2;	
        //  	img2=cvLoadImage("img2.jpg",-1);
        rgb2=new BYTE[320*240*3];
        memcpy(rgb2,m_Frame2->imageData,320*240*3);
        //ͼ���ϱ߲���
        for(int w=0;w<320;w++)
            for(int h=0;h<240;h++)
                for(int k=0;k<3;k++)
                {
                    RGB[h*320*3+w*3+k]=rgb1[h*320*3+w*3+k];
                }
                /*
        //ͼ���ص�����
        for(int w=0;w<320;w++)
            for(int h=0;h<40;h++)
                for(int k=0;k<3;k++)
                {
                    RGB[(h+240)*320*3+w*3+k]=rgb2[h*320*3+w*3+k];
                }  */
        //ͼ���±߲���
        for(int w=0;w<320;w++)
            for(int h=0;h<240;h++)
                for(int k=0;k<3;k++)
                {
                    RGB[(h+240)*320*3+w*3+k]=rgb2[h*320*3+w*3+k];
                }
        
        cvSetData(IMG,RGB,320*3);
        //����ͼ��
//        Img_zh=new IplImage;
        Img_zh=cvCloneImage(IMG);
        m_CvvImage3.CopyOf(Img_zh, 1);
        m_CvvImage3.DrawToHDC(hDC3, rect3);
		/*
		delete IMG;
		delete rgb1;
		delete rgb2;
		delete Img_zh;
		*/
        if(isDetectEdge)//��Ե���
        {
            IplImage *CannyImage, *GrayImage;  //, *outImage
            CannyImage = cvCreateImage( cvSize(Img_zh->width,Img_zh->height), IPL_DEPTH_8U, 1 );
            GrayImage  = cvCreateImage( cvSize(Img_zh->width,Img_zh->height), IPL_DEPTH_8U, 1 );
            outImgDetect   = cvCreateImage( cvSize(Img_zh->width,Img_zh->height), IPL_DEPTH_8U, 3 );

            cvCvtColor( Img_zh, GrayImage, CV_BGR2GRAY);    
            cvCanny(GrayImage, CannyImage, 50, 150, 3);
            cvCvtColor( CannyImage, outImgDetect, CV_GRAY2BGR); 

            CvvImage m_CvvImage;
            m_CvvImage.CopyOf(outImgDetect, 1);
            m_CvvImage.DrawToHDC(hDCDetect, &rectDetect);

            cvReleaseImage(&CannyImage);
            cvReleaseImage(&GrayImage);
            cvReleaseImage(&outImgDetect);
        }

        if(isDetectCorner)//�ǵ���
        {
            IplImage *srcImage,*grayImage,*corners1,*corners2;
            srcImage    = cvCloneImage(Img_zh);//��Ƶƴ�ӵ�ͼ��
            //������ͨ��ͼ��Ϊ������ڴ�ռ�
            grayImage   = cvCreateImage(cvGetSize(srcImage), IPL_DEPTH_8U,  1);
            corners1    = cvCreateImage(cvGetSize(srcImage), IPL_DEPTH_32F, 1);
            corners2    = cvCreateImage(cvGetSize(srcImage), IPL_DEPTH_32F, 1);
            CvPoint2D32f corners[MAX_CORNERS];//�궨�壺���ǵ���Ϊ500
            int cornerCount = 0;
            cvCvtColor(srcImage, grayImage, CV_BGR2GRAY);
            //ȷ��ͼ���ǿ�ǵ�
            cvGoodFeaturesToTrack(grayImage, 
                corners1, corners2, corners, &cornerCount, 0.05, 5, 0, 3, 0, 0.4);
            if(cornerCount>0)
            {
                CvScalar color = CV_RGB(255,0,0);//������ɫ����ɫ
                for(int i=0; i<cornerCount;i++)
                {
                    cvCircle(srcImage, cvPoint((int)(corners[i].x), 
                        (int)(corners[i].y)), 6, color, 2, CV_AA, 0);//��Բ
                }
            }
            //��ת�����ͼ�������Ӧ�ؼ�������
            CvvImage m_CvvImage;
            m_CvvImage.CopyOf(srcImage, 1);
            m_CvvImage.DrawToHDC(hDCDetect, &rectDetect);
            //�ͷ�ͼ��
            cvReleaseImage(&srcImage);
            cvReleaseImage(&grayImage);
            cvReleaseImage(&corners1);
            cvReleaseImage(&corners2);
        }
        break;
    case 5:
        pWnd = GetDlgItem(IDC_COORDINATE);
        pWnd->GetClientRect(&rectXY);
        pWnd->GetWindowRect(rectRgnXY);
        ScreenToClient(&rectRgnXY);
        //����һ����������
        rgn.CreateRectRgn(rectRgnXY.left,rectRgnXY.top,rectRgnXY.right,rectRgnXY.bottom);
        for( int w=0;w<320;w+=2)
            for(int h=0;h<480;h+=2)
            {
                m_Color.H=CV_IMAGE_ELEM(HChannel,uchar,h,w);
                m_Color.S=CV_IMAGE_ELEM(SChannel,uchar,h,w);
                m_Color.V=CV_IMAGE_ELEM(VChannel,uchar,h,w);
                if(TRUE==CheckHSV(colorMarked,m_Color))
                {	
                    pointCam.x = w/factorWCam;
                    pointCam.y = h/factorHCam;
                    //InvalidateRect(&rect,1);
                    RedrawWindow(&rectXY,&rgn); //�����ػ�
                } 
            }
        break;
    case 6:
        ScreenShot("..\\Output\\screenshot.jpg");
        screenImg = cvLoadImage("..\\Output\\screenshot.jpg");
        //��ʼ����Ƶд��
        writer = 0;
        isColor = 1;
        fps     = 25;
        writer = cvCreateVideoWriter("..\\Output\\screenvideo.avi", CV_FOURCC('D','I','V','X'), 
            fps, cvSize(screenImg->width,screenImg->height),isColor);   //XVID  DIVX
        //д����Ƶ�ļ�
        cvWriteFrame(writer, screenImg);
        //�ڲ�׽�����в鿴��õ�ÿһ֡
//        cvReleaseVideoWriter(&writer);
        break;
    case 7:   // Ŀ�����   
        int i, bin_w, c;

		IplImage* m_FrameTrack=new IplImage;
		m_FrameTrack=cvCloneImage(Img_zh);

		if (!b_flagTracking)
			return;	

        cvCopy( m_FrameTrack, imageTrack, 0 );
        cvCvtColor( imageTrack, hsvTrack, CV_BGR2HSV ); //��ͼ���RGB��ɫϵתΪHSV��ɫϵ	

        if( isTrackObject )//�����ǰ����Ҫ���ٵ�����   
        {
            int vmin = 10, vmax = 256, smin = 30;
            int _vmin = vmin, _vmax = vmax;

            //������Ĥ�壬ֻ��������ֵΪH��0~180��S��smin~256��V��vmin~vmax֮��Ĳ���
            cvInRangeS( hsvTrack, cvScalar(0,smin,MIN(_vmin,_vmax),0),cvScalar(180,256,MAX(_vmin,_vmax),0), maskTrack ); 
            cvSplit( hsvTrack, hueTrack, 0, 0, 0 ); // ȡ��H����

            //�����Ҫ���ٵ����廹û�н���������ȡ�������ѡȡ�����ͼ��������ȡ
            if( isTrackObject < 0 )
            {
                float max_val = 0.f;
                cvSetImageROI( hueTrack, selection ); // ����ԭѡ���
                cvSetImageROI( maskTrack, selection ); // ����Mask��ѡ���

                cvCalcHist( &hueTrack, histTrack, 0, maskTrack ); // �õ�ѡ�������������Ĥ���ڵ�ֱ��ͼ

                cvGetMinMaxHistValue( histTrack, 0, &max_val, 0, 0 ); 
                cvConvertScale( histTrack->bins, histTrack->bins, max_val ? 255. / max_val : 0., 0 ); // ��ֱ��ͼתΪ0~255
                cvResetImageROI( hueTrack ); //remove ROI
                cvResetImageROI( maskTrack );
                track_window = selection;
                isTrackObject = 1;

                cvZero( histimgTrack );
                bin_w = histimgTrack->width/hdims;
                for( i = 0; i < hdims; i++ )
                {
                    int val = cvRound(cvGetReal1D(histTrack->bins,i)*histimgTrack->height/255 );
                    CvScalar color = hsv2rgb(i*180.f/hdims);
                    //��ֱ��ͼ��ͼ��ռ�
                    cvRectangle( histimgTrack, cvPoint(i*bin_w,histimgTrack->height),
						cvPoint((i+1)*bin_w,histimgTrack->height - val),color, -1, 8, 0 );
                }
            }

            //�õ�hue�ķ���ͶӰͼ
            //cvCalcArrBackProject( (CvArr**)&hueTrack, backprojectTrack, histTrack );
			cvCalcBackProject( &hueTrack, backprojectTrack, histTrack ); // �õ�hue�ķ���ͶӰͼ

            //�õ�����ͶӰͼmask�ڵ�����
            cvAnd( backprojectTrack, maskTrack, backprojectTrack, 0 );
            //ʹ��MeanShift�㷨��backproject�е����ݽ������������ظ��ٽ��
			try
			{
				cvCamShift( backprojectTrack, track_window,cvTermCriteria( CV_TERMCRIT_EPS | CV_TERMCRIT_ITER, 10, 1 ),&track_comp, &track_box );

			}
			catch (CException* e)
			{
			}
            track_window = track_comp.rect;//�õ����ٽ���ľ��ο�

            if( backproject_mode )
                cvCvtColor( backprojectTrack, imageTrack, CV_GRAY2BGR ); //��ʾģʽ
            if( imageTrack->origin )
                track_box.angle = -track_box.angle;
            //�������ٽ����λ��
            cvEllipseBox( imageTrack, track_box, CV_RGB(128,0,255), 3, CV_AA, 0 );

            //�������������ѡ�񣬻���ѡ���
            if( isTrackObject && selection.width > 0 && selection.height > 0 )
            {
                //cvSetImageROI( imageTrack, selection );
                //cvXorS( imageTrack, cvScalarAll(255), imageTrack, 0 );
                //cvResetImageROI( imageTrack );
            }
            pointXY.x=(int)track_box.center.x;
            pointXY.y=240-(int)track_box.center.y;
        }
        
		imgTrack.CopyOf(imageTrack,1);
		imgTrack.DrawToHDC(hDCTrack, &rectTrack);

        break;
    }
    CDialogEx::OnTimer(nIDEvent);
}

BOOL CMultiCamDlg::PreTranslateMessage(MSG* pMsg)
{
    // TODO: �ڴ����ר�ô����/����û���
    if(pMsg->message == WM_LBUTTONDOWN && 
        GetDlgItem(IDC_VEDIO_STITCHING)->GetSafeHwnd() == pMsg->hwnd)
    {  
        if(isVedioStitching)//�ж��Ƿ������Ƶƴ�Ӵ���
        {
            OnLButtonDown(MK_LBUTTON, pMsg->pt);//ʹִ��CMultiCamDlg::OnLButtonDown����
        }
    }
    return CDialogEx::PreTranslateMessage(pMsg); 
}
//��CMultiCamDlg::PreTranslateMessage�����е��õ�OnLButtonDown����
void CMultiCamDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
    // TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
	//MessageBox(_T("OK"));
    //��ȡ�ؼ��������Ļ��λ�ã�������Ļ����ת��Ϊ�Ի��򣨿ͻ���������
    CRect rectCam;
    GetDlgItem(IDC_VEDIO_STITCHING)->GetWindowRect(&rectCam);
    ScreenToClient(rectCam);
    //��������ĵ����Ļ����ת��Ϊ�Ի��򣨿ͻ���������
    //ScreenToClient((LPPOINT)(&point));

	/*
	TCHAR Test[256];//�����IDC_VEDIO_STITCHING�ؼ��ϵ���ĵ��HSV����
	_stprintf(Test,TEXT("%d, %d \n %d %d %d %d"),point.x,point.y,
		rectCam.left,rectCam.right,rectCam.top,rectCam.bottom);
	MessageBox(Test,_T("Test1"));
	*/

    //�ж��Ƿ�������IDC_VEDIO_STITCHING����
    if(point.x>=rectCam.left && point.x<=rectCam.right && 
        point.y>=rectCam.top && point.y<=rectCam.bottom)
    {
/*******  ��ȡ�����IDC_VEDIO_STITCHING�ؼ���ƴ����Ƶ���ϵ���ĵ������  *********/

    //�����IDC_VEDIO_STITCHING�ؼ��ϵ���ĵ��XY����
        pointCam.x = point.x-rectCam.left;
        pointCam.y = point.y-rectCam.top;
        //�����ڿؼ����������ת��Ϊ��Ӧͼ�����������
        CvPoint2D32f pt;
        pt.x = factorWCam*pointCam.x;
        pt.y = factorHCam*pointCam.y;
        //����H��S��V����ͨ����ͼ�񲢷����ڴ�
        hsv     =cvCreateImage(cvGetSize(Img_zh), 8, 3);
	    HChannel=cvCreateImage(cvGetSize(Img_zh), 8, 1);
	    SChannel=cvCreateImage(cvGetSize(Img_zh), 8, 1);
	    VChannel=cvCreateImage(cvGetSize(Img_zh), 8, 1);
        //��ɫ�ռ�ת��
        cvCvtColor(Img_zh,hsv,CV_BGR2HSV);
        //�ָ��ͨ������ɼ�����ͨ��������ߴ���������ȡһ��ͨ��
        cvSplit(hsv,HChannel,SChannel,VChannel,NULL);  //cvCvtPixToPlane
        //��ȡ����������H S V
        colorMarked.H=CV_IMAGE_ELEM(HChannel,uchar,(int)pt.y,(int)pt.x);
	    colorMarked.S=CV_IMAGE_ELEM(SChannel,uchar,(int)pt.y,(int)pt.x);
	    colorMarked.V=CV_IMAGE_ELEM(VChannel,uchar,(int)pt.y,(int)pt.x);

        TCHAR szXYandHSV[256];//�����IDC_VEDIO_STITCHING�ؼ��ϵ���ĵ��HSV����
        _stprintf(szXYandHSV,TEXT("XY:  (%d, %d)\nHSV: (%d, %d, %d)"),
            pointCam.x, pointCam.y, colorMarked.H, colorMarked.S, colorMarked.V);
        MessageBox(szXYandHSV,_T("XY & HSV"));

/*********�����IDC_VEDIO_STITCHING�ؼ���ƴ����Ƶ��ʱʹIDC_COORDINATE�ؼ��ػ�*********/
        isLDownCam = true;
        CWnd *pWnd = GetDlgItem(IDC_COORDINATE);
        CRect rect,rectRgn;
        pWnd->GetClientRect(&rect);
        pWnd->GetWindowRect(rectRgn);
        ScreenToClient(&rectRgn);
        CRgn rgn;//����һ����������
        rgn.CreateRectRgn(rectRgn.left,rectRgn.top,rectRgn.right,rectRgn.bottom);
        //InvalidateRect(&rect,1);
        RedrawWindow(&rect,&rgn); //�����ػ�
    }

    CRect rectTrack;
    GetDlgItem(IDC_TRACK)->GetWindowRect(&rectTrack);
    ScreenToClient(rectTrack);
    //��������ĵ����Ļ����ת��Ϊ�Ի��򣨿ͻ���������
    //ScreenToClient((LPPOINT)(&point));
	
	//TCHAR szXYandHSV[256];//�����IDC_VEDIO_STITCHING�ؼ��ϵ���ĵ��HSV����
	//_stprintf(szXYandHSV,TEXT("XY:  (%d, %d)\nHSV: (%d, %d, %d,%d)"),
	//	point.x, point.y, rectTrack.left,rectTrack.top,rectTrack.right,rectTrack.bottom);
	//MessageBox(szXYandHSV,_T("XY & HSV"));
	

    //�ж��Ƿ�������IDC_TRACK����
    if((point.x>=rectTrack.left && point.x<=rectTrack.right) && 
        (point.y>=rectTrack.top && point.y<=rectTrack.bottom))
    {
        isLDownTrack = true;
        //�����IDC_VEDIO_STITCHING�ؼ��ϵ���ĵ��XY����
        pointTrack.x = point.x-rectTrack.left;
        pointTrack.y = point.y-rectTrack.top;
        if( !imageTrack )
        {
            AfxMessageBox(_T("�뿪ʼ����"));
            return;
        }
		/*      
        if( imageTrack->origin )
              pointy = 240 - pointy;

        if (imageTrack->width==640)
        {
            pointx=pointx*640/320;
            pointy=pointy*480/240;
        }
		*/
        origin.x = pointTrack.x;
        origin.y = pointTrack.y;
        selection = cvRect(pointTrack.x,pointTrack.y,0,0);//����
        isSelectObject = 1;//������ʼ����ѡȡ
    }
    CDialogEx::OnLButtonDown(nFlags, point);
}

CPoint pointMove;
void CMultiCamDlg::OnLButtonUp(UINT nFlags, CPoint point)
{
    // TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
	/*
	CvPoint origin1,point1;
	origin1.x =selection.x;
	origin1.y =selection.y;
	point1.x = selection.x+selection.width;
	point1.y = selection.y+selection.height;
	cvRectangle(imageTrack,origin,point1,cvScalar(255,0,0));
	cvSaveImage("track.jpg",imageTrack);
	*/
	/*
	TCHAR szXYandHSV[256];//�����IDC_VEDIO_STITCHING�ؼ��ϵ���ĵ��HSV����
	_stprintf(szXYandHSV,TEXT("XY:  (%d, %d,%d,%d)\nHSV: (%d, %d)\n%f"),
		origin.x, origin.y, pointMove.x,pointMove.y,selection.width, selection.height,factorWTrack);
	MessageBox(szXYandHSV,_T("XY & HSV"));
	*/
    if(isLDownTrack)//isLDownTrack
    {
        isSelectObject = 0;//ѡ�������ѡ��Ŀ���־Ϊ��
        if( selection.width > 0 && selection.height > 0 )//���ѡ�����������
            isTrackObject = -1;//���ѡ��������Ч����򿪸��ٹ���
    }
    CDialogEx::OnLButtonUp(nFlags, point);
}
void CMultiCamDlg::OnMouseMove(UINT nFlags, CPoint point)
{
    // TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
    CRect rectTrack;
    GetDlgItem(IDC_TRACK)->GetWindowRect(&rectTrack);
    ScreenToClient(rectTrack);
    //��������ĵ����Ļ����ת��Ϊ�Ի��򣨿ͻ���������
    //ScreenToClient((LPPOINT)(&point));

    //�ж��Ƿ�������IDC_VEDIO_STITCHING����
    if((point.x>=rectTrack.left && point.x<=rectTrack.right) && 
        (point.y>=rectTrack.top && point.y<=rectTrack.bottom))
    {
		//�����Զ�����
		SetCursor(AfxGetApp()->LoadCursor(IDC_CURSOR));
        if (isSelectObject)
        {
			pointMove.x = point.x-rectTrack.left;
			pointMove.y = point.y-rectTrack.top;
			/*
            if( imageTrack->origin )
			{
                pointMove.y = 480- pointMove.y;
			}
            if (imageTrack->width==640)
            {
                pointMove.x=pointMove.x*640/320;
                pointMove.y=pointMove.y*480/240;
            }
			*/
            selection.x = (MIN(pointMove.x,origin.x))*factorWTrack;
            selection.y = MIN(pointMove.y,origin.y)*factorHTrack;
            selection.width =  (CV_IABS(pointMove.x - origin.x))*factorWTrack;
            selection.height = (CV_IABS(pointMove.y - origin.y))*factorHTrack;
			/*
			selection.x = MIN(pointMove.x,origin.x);
			selection.y = MIN(pointMove.y,origin.y);
			selection.width =  selection.x +CV_IABS(pointMove.x - origin.x);//
			selection.height = selection.y +CV_IABS(pointMove.y - origin.y);// 
			*/
            selection.x = MAX( selection.x, 0 );
            selection.y = MAX( selection.y, 0 );
            selection.width = MIN( selection.width, imageTrack->width );
            selection.height = MIN( selection.height, imageTrack->height );
            //selection.width -= selection.x;
            //selection.height -= selection.y;	   
        }
    }
    CDialogEx::OnMouseMove(nFlags, point);
}

//�ж�h,s,v�Ƿ�������ķ�Χ��
bool CMultiCamDlg::CheckHSV(HSV colorMarked, HSV m_Color)
{
    int H,S,V, h,s,v;
    H = colorMarked.H;
    S = colorMarked.S;
    V = colorMarked.V;
    h = m_Color.H;
    s = m_Color.S;
    v = m_Color.V;
    if(h>=H-10 && h<=H+10)
        if(s>=S-30 && s<=S+30)
            if(v>=V-30 && v<=V+30)
                return true;
            else
                return false;
}

/*****************  ��������ʼ���׽���  ********************/
bool CMultiCamDlg::InitSocket(void)
{
    //�����׽���
    m_socket = socket(AF_INET,SOCK_DGRAM,0);
    if(INVALID_SOCKET == m_socket)
    {
        MessageBox(_T("�����׽���ʧ�ܣ�"));
        return false;
    }
    SOCKADDR_IN addrSock;
    addrSock.sin_family = AF_INET;
    addrSock.sin_port = htons(6000);
    addrSock.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

    int retval;
    //���׽���
    retval = bind(m_socket,(SOCKADDR*)&addrSock,sizeof(SOCKADDR));
    if(SOCKET_ERROR == retval)
    {
        closesocket(m_socket);
        MessageBox(_T("��ʧ�ܣ�"));
        return false;
    }
    return TRUE;
}
//�������ݵ��߳�
DWORD WINAPI CMultiCamDlg::RecvProcThread(LPVOID lpParameter)
{
    //��ȡ���̴߳��ݵ��׽��ֺʹ��ھ��
    SOCKET sock = ((RECVPARAM*)lpParameter)->sock;
    HWND hwnd = ((RECVPARAM*)lpParameter)->hwnd;
    delete lpParameter;

    SOCKADDR_IN addrFrom;
    int len = sizeof(SOCKADDR);

    char recvBuf[200];
    char tempBuf[300];
    int retval;
    while(TRUE)
    {
        //��������
        retval = recvfrom(sock,recvBuf,200,0,(SOCKADDR*)&addrFrom,&len);
        if(SOCKET_ERROR == retval)
            break;
        sprintf(tempBuf,"%s",recvBuf);
        ::PostMessage(hwnd,WM_RECVDATA,0,(LPARAM)tempBuf);
    }
    return 0;
}
//����������Ϣӳ�亯��
LRESULT CMultiCamDlg::OnRecvData(WPARAM wParam, LPARAM lParam)
{
    //ȡ�����յ�������
    CString str = _T(""); 
    str = (char*)lParam;
    CString strTemp;
    //����ѽ��ܵ�����
    GetDlgItemText(IDC_EDIT_ACCEPTCOMMENT,strTemp);
    str += "\r\n";
    strTemp += str;
    SetDlgItemText(IDC_EDIT_ACCEPTCOMMENT,strTemp);
    return 0;
}
//��������
void CMultiCamDlg::OnBnClickedBtnSend()
{
    // TODO: �ڴ���ӿؼ�֪ͨ����������
    
    DWORD dwIP; //��öԷ�IP
    ((CIPAddressCtrl*)GetDlgItem(IDC_IPADDRESS1))->GetAddress(dwIP);
    UpdateData();
    SOCKADDR_IN addrTo;
    addrTo.sin_family = AF_INET;
    addrTo.sin_port = htons(nPort);
    addrTo.sin_addr.S_un.S_addr = htonl(dwIP);

    CString strSend = _T("");
    //��ô���������
    GetDlgItemText(IDC_EDIT_SENDCOMMENT,strSend);

    //��LPCTSTR��CString strSendת��Ϊconst char* pstrSend
    const size_t strsize=(strSend.GetLength()+1)*2; //���ַ��ĳ���;
    char * pstrSend= new char[strsize]; //����ռ�;
    size_t sz=0;
    wcstombs_s(&sz,pstrSend,strsize,strSend,_TRUNCATE);
    int n=atoi((const char*)pstrSend); // �ַ����Ѿ���ԭ����CString ת������ const char*

    //��������
    sendto(m_socket,pstrSend,strSend.GetLength()+1,0,(SOCKADDR*)&addrTo,sizeof(SOCKADDR));
    //��շ��ͱ༭���е�����
    SetDlgItemText(IDC_EDIT_SENDCOMMENT,_T(""));
}


void CMultiCamDlg::OnScreenShot()
{
    // TODO: �ڴ���������������
    ScreenShot("..\\Output\\screenshot.jpg");
}
//��Ļ��ͼ
void CMultiCamDlg::ScreenShot(char* filename)
{
    CDC *pDC ;
    //CDC::FromHandle(::GetDC(GetSafeHwnd()));
    pDC =CDC::FromHandle(::GetWindowDC(GetSafeHwnd()));//��ȡ��ǰ�����Ի����DC
    //GetDlgItem(IDD_MULTICAM1_DIALOG)->GetDC();
    //HWND hwnd = ::GetForegroundWindow(); // ��õ�ǰ�����
    //CClientDC dc(this);
    //pDC = CDC::FromHandle(NULL); //��ȡ��ǰ������ĻDC
    //GetActiveWindow();

    CRect rect;
    GetClientRect(&rect);
    int Width = rect.Width()+3;
    int Height = rect.Height()+3;
    int BitPerPixel = pDC->GetDeviceCaps(BITSPIXEL);//�����ɫģʽ������λɫ��
//    int Width = pDC->GetDeviceCaps(HORZRES);        //��Ļ���
//    int Height = pDC->GetDeviceCaps(VERTRES);       //��Ļ�߶�

    CDC memDC;//�ڴ�DC
    memDC.CreateCompatibleDC(pDC);

    CBitmap memBitmap, *oldmemBitmap;//��������Ļ���ݵ�bitmap
    memBitmap.CreateCompatibleBitmap(pDC, Width, Height);

    oldmemBitmap = memDC.SelectObject(&memBitmap);//��memBitmapѡ���ڴ�DC
    memDC.BitBlt(0, 0, Width, Height, pDC, 0, 0, SRCCOPY);//������Ļͼ���ڴ�DC

    //���´��뱣��memDC�е�λͼ���ļ�
    BITMAP bmp;
    memBitmap.GetBitmap(&bmp);//���λͼ��Ϣ

    FILE *fp = fopen(filename, "w+b");

    BITMAPINFOHEADER bih = {0};//λͼ��Ϣͷ
    bih.biBitCount = bmp.bmBitsPixel;//ÿ�������ֽڴ�С
    bih.biCompression = BI_RGB;
    bih.biHeight = bmp.bmHeight;//�߶�
    bih.biPlanes = 1;
    bih.biSize = sizeof(BITMAPINFOHEADER);
    bih.biSizeImage = bmp.bmWidthBytes * bmp.bmHeight;//ͼ�����ݴ�С
    bih.biWidth = bmp.bmWidth;//���

    BITMAPFILEHEADER bfh = {0};//λͼ�ļ�ͷ
    bfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);//��λͼ���ݵ�ƫ����
    bfh.bfSize = bfh.bfOffBits + bmp.bmWidthBytes * bmp.bmHeight;//�ļ��ܵĴ�С
    bfh.bfType = (WORD)0x4d42;
    fwrite(&bfh, 1, sizeof(BITMAPFILEHEADER), fp);//д��λͼ�ļ�ͷ
    fwrite(&bih, 1, sizeof(BITMAPINFOHEADER), fp);//д��λͼ��Ϣͷ
    byte * p = new byte[bmp.bmWidthBytes * bmp.bmHeight];//�����ڴ汣��λͼ����
    GetDIBits(memDC.m_hDC, (HBITMAP) memBitmap.m_hObject, 0, Height, p, 
        (LPBITMAPINFO) &bih, DIB_RGB_COLORS);//��ȡλͼ����
    fwrite(p, 1, bmp.bmWidthBytes * bmp.bmHeight, fp);//д��λͼ����
    delete [] p;
    fclose(fp);
    memDC.SelectObject(oldmemBitmap);
}

CvScalar CMultiCamDlg::hsv2rgb(float hue)
{
    int rgb[3], p, sector;
    static const int sector_data[][3]={{0,2,1}, {1,2,0}, {1,0,2}, {2,0,1}, {2,1,0}, {0,1,2}};
    hue *= 0.033333333333333333333333333333333f;
    sector = cvFloor(hue);
    p = cvRound(255*(hue - sector));
    p ^= sector & 1 ? 255 : 0;

    rgb[sector_data[sector][0]] = 255;
    rgb[sector_data[sector][1]] = 0;
    rgb[sector_data[sector][2]] = p;

    return (cvScalar(rgb[2], rgb[1], rgb[0],0));//���ض�Ӧ����ɫֵ
}

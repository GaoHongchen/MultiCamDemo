
// MultiCamDlg.h : ͷ�ļ�
//

#pragma once
#include "atltypes.h"

#pragma region �������ӵ��ļ��ͺ궨��
#include <opencv2\opencv.hpp>   
#pragma endregion

// CMultiCamDlg �Ի���
class CMultiCamDlg : public CDialogEx
{
// ����
public:
	CMultiCamDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_MULTICAM1_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnClickedOpenCam1();
    afx_msg void OnClickedCloseCam1();
    afx_msg void OnTimer(UINT_PTR nIDEvent);
    afx_msg void OnClickedOpenCam2();
    afx_msg void OnClickedCloseCam2();
//    afx_msg void OnBnClickedGrabFrame1();
    afx_msg void OnClickedGrabFrame2();
    afx_msg void OnBnClickedVideoStitch();
    UINT frameCount1;
    UINT frameCount2;
    afx_msg void OnClickedGrabFrame();
    afx_msg void OnBnClickedEdgeDetect();
    virtual BOOL PreTranslateMessage(MSG* pMsg);
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    bool isLDownCam;
    afx_msg void OnClickedGrabFrame1();
    bool isVedioStitching;
    afx_msg void OnBnClickedCornerDetect();
    bool isDetectEdge;
    bool isDetectCorner;
    afx_msg void OnBnClickedBeginLocate();
    CPoint pointCam;
    CPoint pointXY;
    CPoint pointTrack;

    struct HSV
    {
        int H,S,V;
    }colorMarked,m_Color;

    bool CheckHSV(HSV colorMarked, HSV m_Color);
    static DWORD WINAPI OpenCam1Thread(LPVOID lpParam);
    static DWORD WINAPI OpenCam2Thread(LPVOID lpParam);   
    afx_msg void OnBnClickedBtnSend();
    bool InitSocket(void);
private:
    SOCKET m_socket;
public:
    static DWORD WINAPI RecvProcThread(LPVOID lpParameter);
    LRESULT OnRecvData(WPARAM wParam, LPARAM lParam);
    UINT nPort;
    void ScreenShot(char* filename);
    afx_msg void OnScreenShot();
    void ScreenVideo(void);
    afx_msg void OnScreenVideo();
    afx_msg void OnBnClickedBeginTrack();
    CvScalar hsv2rgb( float hue );
    int isTrackObject;
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
    bool isLDownTrack;
    bool isSelectObject;
	int widthDlg;
	int heightDlg;
};

struct RECVPARAM
{
    SOCKET sock;    //�Ѵ������׽���
    HWND hwnd;      //�Ի�����
};

#define WM_RECVDATA WM_USER+1



// MusicDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "Music.h"
#include "MusicDlg.h"
#include "afxdialogex.h"
#include "tchar.h"
//#include <stdio.h>
#include "winsock2.h"
#include "math.h"
//#pragma comment(lib,"ws2_32.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#endif
//char sendbuf[3];

// 用于应用程序“关于”菜单项的 CAboutDlg  对话框
char sendbuf[32],sendbuf1[32];
int getdata[6],x1,gx,gy,gz;
float x,y,z,anglexz;
int colour=0;
float xx1=50,yy1=100,yy2=90,xx2=0,cxx,cyy;
//char music[22]={'b','a','d','d','a','b','b','d','a','b','a','b','a','d','d','a','b','b','d','a','a','b'};
SOCKET m_socket;
ofstream file;
int frameNumber=0,status=0;
int tempUpData[5];
double AvgFrame[3];
bool StartGap=false;
////////////////////////////////
/*
float Gyro_y;       
float Angle_gy;      
float Accel_x;	 
float Angle_ax;   
float Angle1;        
float angle2;
float Temp_max=0,Temp_min=100;
float Angle;         
char value;	

float  Q_angle=0.0008;  
float  Q_gyro=0.003;
float  R_angle=0.5;
float  dt=0.01;	                  
char   C_0 = 1;
float  Q_bias, Angle_err;
float  PCt_0, PCt_1, E;
float  K_0, K_1, t_0, t_1;
float  Pdot[4] ={0,0,0,0};
float  PP[2][2] = { { 1, 0 },{ 0, 1 } };
void Kalman_Filter(float Accel,float Gyro);*/
////////////////////////////////

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
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


// CMusicDlg 对话框




CMusicDlg::CMusicDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CMusicDlg::IDD, pParent)
	, m_Edit_X(_T(""))
	, m_Edit_Y(_T(""))
	, m_Edit_Z(_T(""))
	, Gyo_X(_T(""))
	, Gyo_Y(_T(""))
	, Gyo_Z(_T(""))
	, Angle_X(_T(""))
	, Angle_Y(_T(""))
	, Angle_Z(_T(""))
	, Angular_X(_T(""))
	, Angular_Y(_T(""))
	, Angular_Z(_T(""))
	, Number(_T(""))
	, NowWay(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMusicDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_MSCOMM1, m_mscomm);
	DDX_Text(pDX, IDC_EDIT1, m_Edit_X);
	DDX_Text(pDX, IDC_EDIT2, m_Edit_Y);
	DDX_Text(pDX, IDC_EDIT3, m_Edit_Z);
	DDX_Text(pDX, IDC_EDIT4, Gyo_X);
	DDX_Text(pDX, IDC_EDIT5, Gyo_Y);
	DDX_Text(pDX, IDC_EDIT6, Gyo_Z);
	DDX_Text(pDX, IDC_EDIT7, Angle_X);
	DDX_Text(pDX, IDC_EDIT8, Angle_Y);
	DDX_Text(pDX, IDC_EDIT9, Angle_Z);
	DDX_Text(pDX, IDC_EDIT10, Angular_X);
	DDX_Text(pDX, IDC_EDIT11, Angular_Y);
	DDX_Text(pDX, IDC_EDIT12, Angular_Z);
	DDX_Text(pDX, IDC_EDIT13, Number);
	DDX_Text(pDX, IDC_EDIT14, NowWay);
}

BEGIN_MESSAGE_MAP(CMusicDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON1, &CMusicDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CMusicDlg::OnBnClickedButton2)
	ON_EN_CHANGE(IDC_EDIT1, &CMusicDlg::OnEnChangeEdit1)
	ON_EN_CHANGE(IDC_EDIT2, &CMusicDlg::OnEnChangeEdit2)
	ON_EN_CHANGE(IDC_EDIT3, &CMusicDlg::OnEnChangeEdit3)
	ON_BN_CLICKED(IDC_BUTTON3, &CMusicDlg::OnBnClickedButton3)
END_MESSAGE_MAP()


// CMusicDlg 消息处理程序

BOOL CMusicDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
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

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CMusicDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CMusicDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CMusicDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CMusicDlg::OnBnClickedButton1()
{
// TODO: 在此添加控件通知处理程序代码
	
	file.open("hello.txt"); 
	if(m_mscomm.get_PortOpen()) //如果串口是打开的，则行关闭串口
	{
		m_mscomm.put_PortOpen(FALSE);
	}
	m_mscomm.put_CommPort(5); //选择COM1
	m_mscomm.put_InBufferSize(1024); //接收缓冲区
	m_mscomm.put_OutBufferSize(1024);//发送缓冲区
	m_mscomm.put_InputLen(0);//设置当前接收区数据长度为0,表示全部读取
	m_mscomm.put_InputMode(1);//以二进制方式读写数据
	m_mscomm.put_RThreshold(1);//接收缓冲区有1个及1个以上字符时，将引发接收数据的OnComm
	m_mscomm.put_Settings(_T("9600,n,8,1"));//波特率9600无检验位，8个数据位，1个停止位

	/**************************2012network**********/



	if(!m_mscomm.get_PortOpen())//如果串口没有打开则打开
	{
		m_mscomm.put_PortOpen(TRUE);//打开串口
		AfxMessageBox(_T("Start the music"));
	}
	else
	{
		m_mscomm.put_OutBufferCount(0);
		AfxMessageBox(_T("Sorry,I forget tools"));	
	}

			/**************************2012network**********/
	WSADATA wsaData;
	int iResult = WSAStartup( MAKEWORD(2,2), &wsaData );
	if ( iResult != NO_ERROR )
	AfxMessageBox(_T("Error at WSAStartup()n"));

	// Create a socket.
	//SOCKET m_socket;
	m_socket = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );

	if ( m_socket == INVALID_SOCKET ) {
	AfxMessageBox(_T( "Error at socket(): %ldn", WSAGetLastError() ));
	WSACleanup();
	return;
	}

	// Connect to a server.
	sockaddr_in clientService;

	clientService.sin_family = AF_INET;
	//clientService.sin_addr.s_addr = inet_addr( "140.0.0.1" );
	clientService.sin_addr.s_addr = inet_addr( "127.0.0.1" );
	clientService.sin_port = htons( 9999 );

	if ( connect( m_socket, (SOCKADDR*) &clientService, sizeof(clientService) ) == SOCKET_ERROR)
	{
	AfxMessageBox(_T( "Failed to connect.n" ));
	WSACleanup();
	return;
	}
	//Kalman_x =new Kalman;
	//Kalman_y =new Kalman;

}


void CMusicDlg::OnBnClickedButton2()
{
// TODO: 在此添加控件通知处理程序代码
	m_mscomm.put_PortOpen(FALSE);//关闭串口
	AfxMessageBox(_T("See u next time"));
}


void CMusicDlg::OnEnChangeEdit1()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
}


void CMusicDlg::OnEnChangeEdit2()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
}


void CMusicDlg::OnEnChangeEdit3()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
}
void CMusicDlg::OnBnClickedButton3(void)
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
}
BEGIN_EVENTSINK_MAP(CMusicDlg, CDialogEx)
	ON_EVENT(CMusicDlg, IDC_MSCOMM1, 1, CMusicDlg::OnCommMscomm1, VTS_NONE)
END_EVENTSINK_MAP()


void CMusicDlg::OnCommMscomm1()
{
	// TODO: 在此处添加消息处理程序代码
	static unsigned int cnt=0,cnt1=1,mcnt=0,select=0;
	VARIANT variant_inp;
	COleVariant myVar;
	COleSafeArray safearray_inp;
	long len,k;
	unsigned char b;
	int a;
	double cha;
	unsigned int p,q;
	DWORD dwHex;
	unsigned int data[32]={0};
	byte data1[2];
	float jiao;
	byte rxdata[1024]; //设置 BYTE 数组
	CString strtemp,strtemp1,strtemp2;
	unsigned int j;
	double tempX=0,tempY=0;
	int bytesSent; 
	int bytesRecv = SOCKET_ERROR;


	if(m_mscomm.get_CommEvent()==2) 
	{	
			myVar.Attach (m_mscomm.get_Input());
			safearray_inp=myVar;
			len=safearray_inp.GetOneDimSize(); 

			for(k=0;k<len;k++)
			{
				safearray_inp.GetElement(&k,rxdata+k); 
			}
			for(k=0;k<len;k++) 
			{		
				cnt++;
				if(cnt==1)
				{
					sendbuf[0]=(unsigned char)*(rxdata+k);
					select=1;

				}
				sendbuf[cnt-1]=(unsigned char)*(rxdata+k);
				if(cnt==14)
				{
					getdata[0]=(sendbuf[1]<<8)+sendbuf[2];
					getdata[1]=(sendbuf[3]<<8)+sendbuf[4];
					getdata[2]=(sendbuf[5]<<8)+sendbuf[6];
					getdata[3]=(sendbuf[7]<<8)+sendbuf[8];
					getdata[4]=(sendbuf[9]<<8)+sendbuf[10];
					getdata[5]=(sendbuf[11]<<8)+sendbuf[12];
					for(int i=1;i<13;i++)
					{
						if(sendbuf[i]>=0)
						{
							sendbuf1[2*i]=0;
						}
						else
						{
							sendbuf1[2*i]=1;
						}
						sendbuf1[2*i-1]=abs(sendbuf[i]);
					}
						
					strtemp.Format(_T("x=%d"),getdata[0]);
					m_Edit_X=strtemp;

					strtemp.Format(_T("y=%d"),getdata[1]);
					m_Edit_Y=strtemp;	
						
					strtemp.Format(_T("z=%d"),getdata[2]);
					m_Edit_Z=strtemp;

					strtemp.Format(_T("G_x=%d"),getdata[3]);
					Gyo_X=strtemp;

					strtemp.Format(_T("G_y=%d"),getdata[4]);
					Gyo_Y=strtemp;	
						
					strtemp.Format(_T("G_z=%d"),getdata[5]);
					Gyo_Z=strtemp;
					
					float tempX,tempXGyo,tempY,tempYGyo;

					tempX = (float)(getdata[0]+129) /16384;   
					tempX = tempX*1.1*180/3.14;    
					tempXGyo = -(float)(getdata[4] -19)/16.4;         
 					
					tempY = (float)(getdata[1]+129) /16384;   
					tempY = tempX*1.1*180/3.14;    
					tempYGyo = -(float)(getdata[3] -19)/16.4;         
 					
					Kalman_x.Kalman_Filter(tempX,tempXGyo);

					Kalman_y.Kalman_Filter(tempY,tempYGyo);
					
	

					strtemp.Format(_T("%f"),Kalman_x.Angle);
					Angle_X=strtemp;	
						
					strtemp.Format(_T("%f"),Kalman_y.Angle);
					Angle_Y=strtemp;

					if(tempX>2000)
						Angle_Z="left";
					else if(tempX<-2000)
						Angle_Z="right";
					else 
						Angle_Z="no";

					if(Kalman_x.Gyro_y>250)
						Angular_Z="left";
					else if(Kalman_x.Gyro_y<-250)
						Angular_Z="right";
					else 
						Angular_Z="no";

					strtemp.Format(_T("%f"),Kalman_x.Gyro_y);
					Angular_X=strtemp;
					strtemp.Format(_T("%f"),Kalman_y.Gyro_y);
					Angular_Y=strtemp;
			
					if(getdata[3]<0)
						getdata[3]+=270;
					if(getdata[4]<0)
						getdata[4]+=270;
					if(getdata[5]<0)
						getdata[5]+=270;

					strtemp.Format(_T("%d"),200-frameNumber);
					Number=strtemp;
					
					//キ菲てА
					if(frameNumber>2)
					{
						int IndexofDataSet=frameNumber%3;
						AvgFrame[IndexofDataSet]=getdata[3];
						getdata[3]=(AvgFrame[0]+AvgFrame[1]+AvgFrame[2])/3.0;
					}
					else
					{
						AvgFrame[frameNumber]=getdata[3];
					}

					if(frameNumber>=200)
					{
						int IndexofDataSet=frameNumber-200;
						
						if(IndexofDataSet>4)
						{
							for(int i=0;i<4;i++)
							{
								tempUpData[i]=tempUpData[i+1];
							}
							tempUpData[4]=getdata[3];
						}
						else
							tempUpData[IndexofDataSet]=getdata[3];

						if(getdata[3]<0)
							getdata[3]*=-1;

						if(tempUpData[0]<tempUpData[1] && tempUpData[1]<tempUpData[2] && tempUpData[2]<tempUpData[3] && tempUpData[3]<tempUpData[4] &&tempUpData[0]>0)
						{
							StartGap=true;
						}
						else if(tempUpData[0]>tempUpData[1]&&tempUpData[1]>tempUpData[2]&&tempUpData[2]>tempUpData[3]&&tempUpData[3]>tempUpData[4])
						{
							if(StartGap)
								StartGap=false;
						}

					   
						if(getdata[3]>800)
						{
							NowWay="OPEN";
							status=1;
						}
						else
						{
							NowWay="CLOSE";
							status=-1;
						}
						sendbuf1[0]=status;
						bytesSent = send( m_socket, sendbuf1, 1, 0 );
						
						
						file<<frameNumber<<","<<status<<","<<getdata[3]<<","<<getdata[4]<<","<<getdata[5]<<","<<getdata[0]<<","<<getdata[1]<<","<<getdata[2]<<","<<Kalman_x.Gyro_y<<","<<Kalman_y.Gyro_y<<"\n";

					}
					
					
				}
				
				if(cnt==32)
				{
					
					//file<<"x="<<Kalman_x.Gyro_y<<" y="<<Kalman_y.Gyro_y<<"\n";
					frameNumber++;
					cnt=0;					
				}
		}
		
	UpdateData(FALSE); //更新编辑框内容
}

}

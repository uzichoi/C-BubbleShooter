// BubbleShooter.c
// 2025 프로그래밍랩[A] 텀프로젝트

#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <conio.h>
#include <Windows.h>
#include <time.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")

#define ESC	0x1b	// ESC key 값
#define MAX_ROWS 30
#define MAX_COLS 10

int bubbleMap[MAX_ROWS][MAX_COLS];	// 버블의 색상을 저장하는 2차원 배열
int visited[MAX_ROWS][MAX_COLS];	// 방문했는지 확인하는 2차원 배열
int radius = 18;				// 반지름
int space = 1;					// 버블간 간격
int startX = 214, startY = 60;	// 게임판 그릴 때의 좌우 여백
int screenWidth = 600;			// 스크린 너비
int screenHeight = 850;			// 스크린 높이
int activeBubble = 0;	// 현재 발사 중인 버블의 존재 여부
int totalScore = 0;		//	총점
int combo = 0;			// 최대
int maxCombo = 0;		// 최대 콤보
int comboTime = 3000;	// 콤보 유지 시간 (3초)
int lastRemoveTime = 0;	// 마지막으로 버블 제거한 시각
int minutes = 0;		// 생존 시간 (분 단위)
int seconds = 0;		// 생존 시간 (초 단위)
int availableColors = 5;	// 사용 가능한 색상 수
clock_t gameStartTime;		// 게임 시작 시간

// 색상 정의 (실제 RGB 색상값을 담은 테이블)
COLORREF colorList[] = {	// COLORREF = GDI에서 색상 값을 표현하기 위해 사용하는 정수 자료형
	// 0-5: 버블 색상 (트라이어딕 기반)
	RGB(255, 60, 60),     // 0: 코랄 레드
	RGB(60, 120, 255),    // 1: 로얄 블루  
	RGB(255, 200, 40),    // 2: 골든 옐로우 (가시성 개선)
	RGB(255,140,140),	  // 3: 라이트 레드
	RGB(100,200,255),   // 4: 라이트 블루
	RGB(255, 220, 80),    // 5: 라이트 옐로우

	// 6-10: 게임 영역 (중성 그레이 + 블루 포인트)
	RGB(150,150,155),   // 6: 게임박스 윤곽선 (쿨 그레이)
	RGB(100, 220, 240),	  // 7: 조준선 연한 시안 계열
	RGB(180,180,180),   // 8: 데드라인 (중간 그레이)
	RGB(20, 22, 28),      // 9: 게임박스 내부 (다크 네이비)
	RGB(35,38,45),      // 10: 게임박스 외부 (미디엄 네이비)

	// 11-14: UI 요소 (다크 테마 + 블루 액센트)
	RGB(25, 28, 35),      // 11: 메뉴바 배경 (다크 네이비)
	RGB(200, 210, 220),   // 12: 메뉴바 텍스트 (라이트 그레이)
	RGB(15, 17, 22),      // 13: 게임 종료 배경 (딥 네이비)
	RGB(255, 60, 60),      // 14: 게임 종료 메시지 (메인 레드 재사용)
	RGB(80,160,240),		// 15: 요소간 구분 미드 블루톤

	// 16-19: 시작 화면 타이틀 애니메이션용 시퀀스 컬러 (트라이어딕 & 보조)
	RGB(255,  60,  60),   // 16: 타이틀 색상1 – 코랄 레드 (강렬한 첫인상)
	RGB(60, 120, 255),   // 17: 타이틀 색상2 – 로얄 블루 (시원하고 명확한 대비)
	RGB(255, 200,  40),   // 18: 타이틀 색상3 – 골든 옐로우 (밝고 경쾌한 포인트)
	RGB(100, 200, 255),   // 19: 타이틀 색상4 – 라이트 블루 (버블 컬러와 조화되는 부드러움)
	RGB(255, 255, 255)		// 20: 흰색
};

// 발사용 버블 구조체
typedef struct {
	double x, y;	// 현재 위치
	double dx, dy;	// 이동 방향 단위 벡터
	int colorIndex;	// 색상 인덱스
	int active;		// 활성 여부 (0: 비활성, 1: 발사 중)
} Bubble;

// ===== 함수 원형 선언 =====
void gotoxy(int x, int y);
void textcolor(int fg_color, int bg_color);
void removeCursor(void);
void showCursor(void);
void cls(int text_color, int bg_color);
void printStartScreen(HDC hdc, HDC memDC, int screenWidth, int screenHeight);
void initBubbles();
void addNewLine();
int isGameOver(int y1, int y2);
void drawAllBubbles(HDC hdc);
void drawShootingBubble(HDC hdc, Bubble* pb);
void drawCrosshair(HDC memDC, Bubble* pb, int x1, int x2, int y1, int y2);
void drawDeadLine(HDC memDC, int x1, int x2, int y1, int y2);
void drawGameBox(HDC hdc, int x1, int y1, int x2, int y2);
void calculateGameBox(int* x1, int* y1, int* x2, int* y2);
void createShootingBubble(Bubble* pb);
void setShootDirection(Bubble* pb);
void startShooting(HDC memDC, Bubble* pb);
int checkCollision(Bubble* pb, int row, int col);
void placeBubble(Bubble* pb, int row, int col);
void manageCollisions(Bubble* pb);
void moveBubble(Bubble* pb, int x1, int x2);
void updateShootingBubble(Bubble* pb, int x1, int x2);
int countConnectedBubbles(int row, int col, int targetColor, int bubbleList[][2], int* bubbleCount);
void removeBubbles(int bubbleList[][2], int bubbleCount);
int removeConnectedBubbles();
void visitAllConnectedBubbles(int row, int col);
int removeDisconnectedBubbles();
int howManyConnected(int row, int col, int testColor);
int chooseColor(int row, int col);
int countScore(int removedBubbles);
void drawTopMenuBar(HDC hdc);
void enableConsoleUTF8();
void gotoConsoleXY(int x, int y);
void writeConsoleText(int x, int y, WORD color, const wchar_t* wstr, DWORD len);
void printGameOver(HDC hdc, HDC memDC, int width, int height);
void resetGame(int* currentLine, clock_t* lastAddTime, Bubble* pb);
void drawBottomMenuBar(HDC memDC, int x1, int y2, int x2);

// ===== main() 함수 =====
int main() {
	HWND hwnd = GetConsoleWindow();  // 현재 실행 중인 콘솔의 핸들(HWWD) 얻기
	HDC hdc = GetDC(hwnd);           // hwnd에 그림 그릴 수 있는 권한(HDC) 얻기
	HDC memDC = CreateCompatibleDC(hdc);	// 더블 버퍼링 (메모리 DC 및 백버퍼 비트맵 생성)
	HBITMAP memBitmap = CreateCompatibleBitmap(hdc, screenWidth, screenHeight);	// 600x850 크기의 메모리용 비트맵 생성 (더블 버퍼링용 백버퍼)
	HBITMAP originalBitmap = (HBITMAP)SelectObject(memDC, memBitmap);  // 기존 비트맵 저장

	removeCursor();
	printStartScreen(hdc, memDC, screenWidth, screenHeight);	// 시작 화면 출력

	Bubble b;		// Bubble 구조체 타입의 슈팅 버블 b 생성
	b.active = 0;	// 초기에는 비활성(0) 상태
	srand((unsigned)time(NULL));	// 난수 seed값 설정
	b.colorIndex = rand() % availableColors;	// 슈팅 버블의 색상을 랜덤한 색상으로 초기화	

	// 게임 초기 설정 
	int x1 = 0, y1 = 0, x2 = 0, y2 = 0;		// 게임 박스 좌표
	int currentLine = 5;					// 현재 라인 수: 5줄
	clock_t lastAddTime = clock();			// 버블을 마지막으로 추가한 시각

	calculateGameBox(&x1, &y1, &x2, &y2);	// 게암 박스 좌표 계산
	initBubbles();			// 상단 5줄 버블 랜덤 색으로 매핑
	enableConsoleUTF8();	// 콘솔에 UTF-8로 쓸 수 있도록 

	// 게임 엔진 시작
	while (1) {
		clock_t now = clock();	// 현재 시각 저장

		if (isGameOver(y1, y2)) {
			printGameOver(hdc, memDC, screenWidth, screenHeight);
			break;	// "GAME OVER" 메시지 출력 후 while문 탈출
		}

		if (now - lastAddTime >= 10000 && currentLine < 14) {	// 3초 간격으로 1줄 추가, 최대 13줄
			addNewLine();
			currentLine++;			// 현재 라인 번호 증가
			lastAddTime = now;		// 마지막으로 라인 추가한 시각을 now로 갱신
		}

		if (kbhit()) {		// 키 입력 처리
			int key = getch();
			if (key == ' ') {			// Space 키 입력 시
				if (b.active == 0) {
					//PlaySound(TEXT("shoot.wav"), NULL, SND_FILENAME | SND_ASYNC);
					startShooting(memDC, &b);	// 버블 발사 
				}
			}
			if (key == 'R' || key == 'r') {
				resetGame(&currentLine, &lastAddTime, &b);
				continue;   // 루프 재시작
			}
			if (key == ESC) {	// ESC 키 입력 시 게임 종료
				printGameOver(hdc, memDC, screenWidth, screenHeight);	// "GAME OVER" 메시지 출력
				break;			// while문 탈출
			}
		}

		// ====== 더블 버퍼링  렌더링 시작 =====
		// 1-1. 메모리 DC 전체를 배경색으로 초기화
		RECT backgroundRect = { 0, 0, 600, 1000 };		// left, top, right, bottom
		HBRUSH outerBrush = CreateSolidBrush(colorList[10]);	 // 22: LIGHT GRAY
		FillRect(memDC, &backgroundRect, outerBrush);
		DeleteObject(outerBrush);

		// 1-2. 박스 내부 배경색 설정
		RECT boxRect = { x1, y1, x2, y2 };
		HBRUSH innerBrush = CreateSolidBrush(colorList[9]);
		FillRect(memDC, &boxRect, innerBrush);
		DeleteObject(innerBrush);

		// 2. 게임 요소들을 메모리 DC에 그리기
		drawGameBox(memDC, x1, y1, x2, y2); // 게임 테두리 그리기. left, top, right, bottom
		drawAllBubbles(memDC);			// 모든 버블 그리기
		drawDeadLine(memDC, x1, x2, y1, y2);		// 데드라인 그리기
		drawShootingBubble(memDC, &b);	// 슈팅 버블 그리기
		drawCrosshair(memDC, &b, x1, y1, x2, y2);		// 조준선 그리기
		drawTopMenuBar(memDC);
		drawBottomMenuBar(memDC, x1, y2, x2);

		// 3. 메모리 DC의 내용을 실제 화면 CD로 한 번에 복사 (깜빡임 방지)
		BitBlt(hdc, 0, 0, screenWidth, screenHeight, memDC, 0, 0, SRCCOPY);
		// ===== 더블 버퍼링 렌더링 끝 =====

		now = clock();
		if (now - lastRemoveTime > comboTime && combo > 0) {
			combo = 0;  // 콤보 리셋
		}

		// 슈팅 버블 물리 업데이트
		updateShootingBubble(&b, x1, x2);

		// 버블이 3개 이상 맞닿으면 터뜨리기
		if (activeBubble == 0) {	// 활성 버블이 없을 때만 (슈팅 버블이 착지한 이후)
			int removedBubbles = 0;
			int totalRemoved = 0;

			// 연결된 버블들을 반복적으로 제거 (연쇄 반응 처리)
			do {
				removedBubbles = removeConnectedBubbles();
				totalRemoved += removedBubbles;

				// 떠있는 버블들도 제거
				if (removedBubbles > 0) {
					int floatingRemoved = removeDisconnectedBubbles();
					totalRemoved += floatingRemoved;
				}

			} while (removedBubbles > 0); // 더 이상 제거할 버블이 없을 때까지 반복
		}

		Sleep(8);  // 8ms
	}

	// 리소스 정리 (메모리 누수 방지)
	SelectObject(memDC, originalBitmap);  // 원래 비트맵 복원
	DeleteObject(memBitmap);         // 생성한 비트맵 삭제
	DeleteDC(memDC);                 // 메모리 DC 삭제
	ReleaseDC(hwnd, hdc);            // 화면 DC 해제
	showCursor();					 // 커서 다시 표시

	return 0;
}


// ===== 함수 구현부 =====
// 시작 화면(제목) 출력하는 함수
void printStartScreen(HDC hdc, HDC memDC, int screenWidth, int screenHeight) {
	// 1. 사용할 타이틀 색상 (버블 색과 동일)
	COLORREF startColors[] = {
		colorList[0], colorList[1], colorList[2],
		colorList[3], colorList[4], colorList[5]
	};
	const int nColors = sizeof(startColors) / sizeof(startColors[0]);

	// 2. 폰트 생성
	HFONT hFontTitle = CreateFontW(
		120, 0, 0, 0,
		FW_HEAVY,   // Impact체의 무게감을 살리기 위해 Heavy
		FALSE, FALSE, FALSE,
		ANSI_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS,
		ANTIALIASED_QUALITY, FF_DONTCARE, L"Impact"
	);
	HFONT hFontPrompt = CreateFontW(
		28, 0, 0, 0,
		FW_NORMAL,
		FALSE, FALSE, FALSE,
		ANSI_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS,
		ANTIALIASED_QUALITY, FF_DONTCARE, L"Consolas"
	);

	// 3. 타이밍 변수
	DWORD lastColorChange = GetTickCount64();	// 마지막 색상 변경
	DWORD lastPromptChange = lastColorChange;	// 마지막 프롬프트 변경
	int colorIndex = 0;		// 타이틀, 프롬프트의 색상 인덱스
	int promptVisible = 1;	// 프롬프트의 가시 여부

	// 4. 루프: SPACE 누를 때까지
	while (1) {
		DWORD now = GetTickCount64();

		// 4-1. 0.5초마다 타이틀 색 변경
		if (now - lastColorChange >= 500) {
			colorIndex = (colorIndex + 1) % nColors;	// 인덱스 증가. 최대 5
			lastColorChange = now;
		}

		// 4-2. 0.25초마다 프롬프트 깜빡임
		if (now - lastPromptChange >= 250) {
			promptVisible = promptVisible > 0 ? 0 : 1;	// 가시성 여부 반전
			lastPromptChange = now;
		}

		// 4-3. 키 입력 확인(스페이스만)
		if (kbhit()) {
			int c = getch();
			if (c == ' ') {
				Sleep(70);	// 아주 잠시 Sleep. 시각적 안정감
				break;   // SPACE 입력 시 루프 탈출
			}
		}

		// 5. 배경 클리어
		RECT backgroundRect = { 0,0, screenWidth, screenHeight };
		HBRUSH brush = CreateSolidBrush(colorList[9]);  // 다크 네이비
		FillRect(memDC, &backgroundRect, brush);
		DeleteObject(brush);

		// 6. 문자 간격 2px로 설정
		SetTextCharacterExtra(memDC, 2);

		// 7. 타이틀 그리기
		SelectObject(memDC, hFontTitle);	// hFontTitle 선택
		SetBkMode(memDC, TRANSPARENT);		// 배경색 없애고 투명하게

		// 7-1. 그림자
		SetTextColor(memDC, colorList[12]);
		RECT s1 = { 0, screenHeight / 5 + 5, screenWidth, screenHeight / 5 + 5 + 120 };
		DrawTextW(memDC, L"Bubble", -1, &s1, DT_CENTER | DT_SINGLELINE);
		RECT s2 = { 0, screenHeight * 3 / 10 + 5, screenWidth, screenHeight * 3 / 10 + 5 + 120 };
		DrawTextW(memDC, L"Shooter", -1, &s2, DT_CENTER | DT_SINGLELINE);

		// 7-2. 본문 타이틀
		SetTextColor(memDC, startColors[colorIndex]);
		RECT t1 = { 0, screenHeight / 5, screenWidth, screenHeight / 5 + 120 };
		DrawTextW(memDC, L"Bubble", -1, &t1, DT_CENTER | DT_SINGLELINE);
		RECT t2 = { 0, screenHeight * 3 / 10, screenWidth, screenHeight * 3 / 10 + 120 };
		DrawTextW(memDC, L"Shooter", -1, &t2, DT_CENTER | DT_SINGLELINE);

		// 8. 프롬프트 (Press SPACE to start)
		if (promptVisible) {
			SelectObject(memDC, hFontPrompt);
			SetTextColor(memDC, startColors[(colorIndex + 3) % nColors]);  // 대비되는 색
			RECT p = { 0, screenHeight * 2 / 3, screenWidth, screenHeight * 2 / 3 + 50 };
			DrawTextW(memDC, L"Press SPACE to start", -1, &p, DT_CENTER | DT_SINGLELINE);
		}

		// 9. 메모리 버퍼의 내용을 한 번에 출력
		BitBlt(hdc, 0, 0, screenWidth, screenHeight, memDC, 0, 0, SRCCOPY);

		// 10. 약간 대기 
		Sleep(16);
	}

	// 11. 리소스 정리
	DeleteObject(hFontTitle);
	DeleteObject(hFontPrompt);

	// 12. 클리어 (바로 게임 화면 전환 준비)
	RECT background2 = { 0,0, screenWidth, screenHeight };
	HBRUSH brush2 = CreateSolidBrush(colorList[10]);  // 미디엄 네이비
	FillRect(memDC, &background2, brush2);
	DeleteObject(brush2);
	BitBlt(hdc, 0, 0, screenWidth, screenHeight, memDC, 0, 0, SRCCOPY);
}

void gotoxy(int x, int y)	//내가 원하는 위치로 커서 이동
{
	COORD pos;				// Windows.h 에 정의
	pos.X = x;
	pos.Y = y;
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}

void textcolor(int fg_color, int bg_color)
{
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), fg_color | bg_color << 4);
}

void removeCursor(void) {	// 커서를 안보이게 한다
	CONSOLE_CURSOR_INFO curInfo;
	GetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &curInfo);
	curInfo.bVisible = 0;
	SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &curInfo);
}

void showCursor(void) {		// 커서를 보이게 한다
	CONSOLE_CURSOR_INFO curInfo;
	GetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &curInfo);
	curInfo.bVisible = 1;
	SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &curInfo);
}

void cls(int text_color, int bg_color)	// 화면 지우기
{
	char cmd[100];
	system("cls");
	sprintf(cmd, "COLOR %x%x", bg_color, text_color);
	system(cmd);
}

// 초기 상단 5줄 버블의 색을 랜덤으로 채우는 함수
void initBubbles() {
	srand((unsigned)time(NULL));	// 랜덤 초기화

	int row, col;

	for (row = 0; row < MAX_ROWS; row++) {
		for (col = 0; col < MAX_COLS; col++) {
			bubbleMap[row][col] = -1;
		}
	}
	// 상단 5줄 랜덤한 색상으로 채우기
	for (row = 0; row < 5; row++) {
		for (col = 0; col < MAX_COLS; col++) {
			bubbleMap[row][col] = chooseColor(row, col);
		}
	}
}

// 새로운 한 줄을 추가하기 위해 bubbleMap을 아래로 미는 함수
void addNewLine() {
	for (int row = MAX_ROWS - 2; row >= 0; row--) {
		for (int col = 0; col < MAX_COLS; col++) {	// 아래로 한 줄씩 밀어 갱신
			bubbleMap[row + 1][col] = bubbleMap[row][col];	// 랜덤한 색으로 매핑
		}
	}
	// 최상단(0행) 한 줄만 최초 한 번에 새 랜덤 색으로 채우기
	for (int col = 0; col < MAX_COLS; col++) {
		bubbleMap[0][col] = chooseColor(0, col);
	}
}

// 게임 종료 여부를 판단하는 함수
int isGameOver(int y1, int y2) {
	int diameter = radius * 2;
	int deadLine = y2 - ((diameter + space) * 3);		// 박스 하단 y좌표에서 버블 3개 만큼의 높이만큼 위쪽

	for (int row = 0; row < MAX_ROWS; row++) {
		for (int col = 0; col < MAX_COLS; col++) {
			if (bubbleMap[row][col] != -1) {	// 빈칸이 아니면
				int bubbleY = row * (diameter + space) + y1 + diameter;	// 현재 버블의 y좌표 계산
				if (bubbleY >= deadLine) { // 데드라인을 넘어서면
					//PlaySound(TEXT("gameover.wav"), NULL, SND_FILENAME | SND_ASYNC);
					return 1;	// 1(게임 오버) 반환
				}
			}
		}
	}
	return 0;
}

// 모든 버블을 HDC(hdc)에 그리는 함수
void drawAllBubbles(HDC hdc) {
	int diameter = radius * 2;	// 지름
	int row, col;

	// 버블 테두리용 펜 생성
	HPEN outlinePen = CreatePen(PS_SOLID, 3, colorList[9]);
	HGDIOBJ oldPen = SelectObject(hdc, outlinePen);

	for (row = 0; row < MAX_ROWS; row++) {
		for (col = 0; col < MAX_COLS; col++) {
			int colorIndex = bubbleMap[row][col];
			if (colorIndex < 0) continue;	// -1(빈칸)인 경우 건너뜀

			int x = col * (diameter + space) + startX;	// 버블의 x좌표 계산
			int y = row * (diameter + space) + startY;	// 버블의 y좌표 계산

			COLORREF color = colorList[colorIndex];
			HBRUSH brush = CreateSolidBrush(color);		// 색상을 채운 브러쉬 생성
			HGDIOBJ oldBrush = SelectObject(hdc, brush);	// 브러쉬 선택하고 기존 브러쉬 저장

			Ellipse(hdc, x, y, x + diameter, y + diameter);		// (x,y)에 원 생성

			SelectObject(hdc, brush);		// 이후 hdc에 그릴 모든 도형을 brush로 채우도록 설정
			DeleteObject(brush);						// 메모리 누수 방지 위해 사용한 붓 삭제
		}
	}
}

// 슈팅 버블을 그리는 함수
void drawShootingBubble(HDC hdc, Bubble* pb) {
	// 활성/비활성 상관없이 언제나 작도해야 함
	int diameter = radius * 2;		// 버블의 지름

	// 대기 상태일 때 기본 위치와 색상 설정
	if (pb->active == 0) {
		// 기본 대기 위치 설정 (화면 하단 중앙)
		double waitingX = 4.5 * (diameter + space) + startX;
		double waitingY = 700;

		// 기본 색상 설정 (랜덤 색상이 아직 없다면 0번 색상을 디폴트로 사용)
		int colorIndex = (pb->colorIndex >= 0) ? pb->colorIndex : 0;

		COLORREF color = colorList[colorIndex];
		HBRUSH brush = CreateSolidBrush(color);
		HGDIOBJ oldBrush = SelectObject(hdc, brush);

		// 버블 테두리용 펜 생성
		HPEN outlinePen = CreatePen(PS_SOLID, 3, colorList[7]);
		HGDIOBJ oldPen = SelectObject(hdc, outlinePen);

		Ellipse(hdc, waitingX, waitingY, waitingX + diameter, waitingY + diameter);

		SelectObject(hdc, oldPen);
		DeleteObject(outlinePen);
		SelectObject(hdc, oldBrush);
		DeleteObject(brush);
		return;
	}

	// 발사 중인 상태일 때 현재 위치에 그리기
	COLORREF color = colorList[pb->colorIndex];
	HBRUSH brush = CreateSolidBrush(color);
	HGDIOBJ oldBrush = SelectObject(hdc, brush);

	// 버블 테두리용 펜 생성
	HPEN outlinePen = CreatePen(PS_SOLID, 2, colorList[9]);
	HGDIOBJ oldPen = SelectObject(hdc, outlinePen);

	Ellipse(hdc, pb->x, pb->y, pb->x + diameter, pb->y + diameter);

	SelectObject(hdc, oldPen);
	DeleteObject(outlinePen);
	SelectObject(hdc, oldBrush);
	DeleteObject(brush);
}

// 조준선(crosshair)을 그리는 함수
void drawCrosshair(HDC memDC, Bubble* pb, int x1, int x2, int y1, int y2) {
	// 발사 중이거나 활성 버블이 있으면 조준선 표시하지 않음. 함수 종료
	if (pb->active == 1 || activeBubble == 1) return;

	POINT mouse;	// 마우스 좌표를 가져오기 위한 POINT 구조체 선언 (x, y)
	GetCursorPos(&mouse);	// 현재 마우스의 화면상 절대 좌표를 가져와 mouse 변수에 저장
	ScreenToClient(GetConsoleWindow(), &mouse);	// 화면 절대 좌표를 콘솔 창 기준의 상대 좌표로 변환 (콘솔 창 내에서의 위치로 변환)

	// 조준선 작도 위치를 게임 박스 하단 중앙으로 고정
	int diameter = radius * 2;
	double bubbleCenterX = 4.5 * (diameter + space) + startX + radius;
	double bubbleCenterY = 700;

	// 아래쪽 발사 방지 (위쪽만 허용)
	if (mouse.y >= bubbleCenterY) {		// 마우스가 버블보다 아래에 있으면
		mouse.y = (int)(bubbleCenterY - 50);	// 강제로 위쪽으로 조정
	}

	// 점선 스타일의 조준선
	HPEN crosshair = CreatePen(PS_DASH, 2, colorList[7]);	// 점선, 순백색, 두께 2인 펜 생성
	HGDIOBJ oldPen = SelectObject(memDC, crosshair);			// 새로운 펜으로 교체하고 기존 펜 저장

	MoveToEx(memDC, bubbleCenterX, bubbleCenterY, NULL);		// 조준선의 시작 위치 설정
	LineTo(memDC, mouse.x, mouse.y);	// 버블 중심부터 현재 마우스 위치(x, y)까지 조준선 작도

	SelectObject(memDC, oldPen);	// oldPen으로 복귀
	DeleteObject(crosshair);		// 메모리 누수 방지 위해 해제
}


// 데드라인을 그리는 함수
void drawDeadLine(HDC memDC, int x1, int x2, int y1, int y2) {
	int diameter = radius * 2;
	int deadLineY = y2 - (diameter * 2 + 10);

	HPEN deadLine = CreatePen(PS_SOLID, 2, colorList[8]);
	HGDIOBJ oldPen = SelectObject(memDC, deadLine);			// 새로운 펜으로 교체하고 기존 펜 저장

	MoveToEx(memDC, (x1 + 5), deadLineY, NULL); // 시작점 설정 (테두리 두께 10 고려)
	LineTo(memDC, (x2 - 7), deadLineY);	// 데드라인 작도

	SelectObject(memDC, oldPen);	// oldPen으로 복귀
	DeleteObject(deadLine);		// 메모리 누수 방지 위해 해제
}

// GDI 방식으로 박스를 그리는 함수 (게임 영역 테두리용)
void drawGameBox(HDC hdc, int x1, int y1, int x2, int y2) {
	HPEN hPen = CreatePen(PS_SOLID, 10, colorList[6]);
	HGDIOBJ oldPen = SelectObject(hdc, hPen);	// 현재 펜을 새 펜으로 교체. 이전에 사용되던 펜을 저장하는 변수 oldPen

	Rectangle(hdc, x1, y1, x2, y2); 	// 사각형 그리기
	SelectObject(hdc, oldPen);	// 새 펜을 해제하고 oldPen(기존 펜)을 다시 적용

	DeleteObject(hPen);
}

// 게임 영역의 박스 좌표를 계산하는 함수
void calculateGameBox(int* x1, int* y1, int* x2, int* y2) {
	int margin = 10;			// 게임 영역 주변에 10px 여백 설정
	int diameter = radius * 2;  // 버블의 지름 = 36

	// 실제 게임 영역 계산
	int gameAreaWidth = MAX_COLS * (diameter + space);	// 너비: 10 * 37 = 370
	int gameAreaHeight = MAX_ROWS * (diameter + space);	// 높이: 30 * 37 = 1110
	int shooterY = 700 - (diameter + margin);			// 슈팅 버블의 y좌표

	// 게임 박스를 화면 정중앙에 배치. startX, startY 갱신
	startX = (screenWidth - gameAreaWidth) / 2;

	int menuBarHeight = 130;	// 상단 메뉴바 높이
	int topMargin = 30;		// 메뉴바와 게임 박스 사이 여백

	startY = menuBarHeight + topMargin;

	// 최종 좌표 계산. 박스의 모퉁이 
	*x1 = startX - margin;                    // 214 - 10 = 204
	*y1 = startY - margin;                    // 60 - 10 = 50  
	*x2 = startX + gameAreaWidth + margin;    // 214 + 370 + 10 = 604
	*y2 = shooterY + (diameter + radius) + margin;       // 700 + 54 + 10 = 762
}

// 슈팅 버블을 생성하는 함수
void createShootingBubble(Bubble* pb) {
	int diameter = 2 * radius;

	// 위치 설정
	pb->x = 4.5 * (diameter + space) + startX;
	pb->y = 700;  // 화면 하단에 고정

	// 상태 설정 (활성화)
	pb->active = 1;
	activeBubble = 1;

	//  방향 벡터 설정 (기본값: 위쪽)
	pb->dx = 0;
	pb->dy = -1;
}

// 마우스 기반으로 발사 방향 설정하는 함수
void setShootDirection(Bubble* pb) {
	POINT mouse;
	GetCursorPos(&mouse);
	ScreenToClient(GetConsoleWindow(), &mouse);

	// 사용자 기준 버블 중심 좌표
	double bubbleCenterX = pb->x + radius;
	double bubbleCenterY = pb->y + radius;
	double xVector = mouse.x - bubbleCenterX;
	double yVector = mouse.y - bubbleCenterY;
	double length = sqrt(xVector * xVector + yVector * yVector);

	if (length > 0) {
		pb->dx = xVector / length;
		pb->dy = yVector / length;
	}
	// 위쪽 방향만 허용 (아래쪽 발사 방지)
	if (pb->dy > 0) {
		pb->dy = -pb->dy;
	}
}

// Space 키 입력 시 발사 시작
void startShooting(HDC memDC, Bubble* pb) {
	if (pb->active == 1) return;  // 이미 발사 중이면 무시

	createShootingBubble(pb);	// 슈팅 버블 생성
	setShootDirection(pb);		// 슈팅 버블의 방향 설정
}

// 버블의 충돌을 검사하는 함수
int checkCollision(Bubble* pb, int row, int col) {
	int diameter = radius * 2;

	double xCenter = pb->x + radius;
	double yCenter = pb->y + radius;
	double xBubble = col * (diameter + space) + startX + radius;
	double yBubble = row * (diameter + space) + startY + radius;

	double dist = sqrt(pow(xCenter - xBubble, 2) + pow(yCenter - yBubble, 2));

	return (dist <= diameter) ? 1 : 0;	// 거리가 지름 이하이면 true, 초과이면 false 반환
}

// 버블을 배치하는 함수
void placeBubble(Bubble* pb, int row, int col) {
	int diameter = radius * 2;

	// 격자 위치로 정렬
	pb->x = col * (diameter + space) + startX;
	pb->y = row * (diameter + space) + startY;

	// 배열에 추가
	bubbleMap[row][col] = pb->colorIndex;

	// 비활성화
	pb->active = 0;
	activeBubble = 0;

	// 다음 슈팅 버블의 색생 미리 지정
	pb->colorIndex = rand() % availableColors;
}

// 버블의 충돌들을 관리하는 함수
void manageCollisions(Bubble* pb) {
	if (pb->active == 0) return;		// 비활성 상태이면 함수 종료

	// 전체 버블 배열을 스캔하여 충돌 검사
	for (int row = 0; row < MAX_ROWS; row++) {
		for (int col = 0; col < MAX_COLS; col++) {
			if (bubbleMap[row][col] < 0) continue;  // 빈칸(-1) 건너뛰기

			if (checkCollision(pb, row, col)) {		// 충돌했으면 충돌한 버블 아래쪽에 배치시킨다.
				int targetRow = row + 1;			// 타켓 열 = 현재 열 + 1		

				// 1. 위쪽 빈 공간 탐색
				if (targetRow < MAX_ROWS && bubbleMap[targetRow][col] == -1) {
					// 조건: targetRow < MAX_ROWS -> 게임판 하단 경계 체크
					// 조건: bubbleMap[targetRow][col] == -1 -> 빈 공간 확인
					placeBubble(pb, targetRow, col);
					return;
				}

				// 2. 위쪽이 막혀있으면 옆자리 탐색 (같은 행에서 좌우 빈 공간 찾기)
				else {
					for (int c = col - 1; c <= col + 1; c++) {
						if (c >= 0 && c < MAX_COLS && bubbleMap[row][c] == -1) {
							placeBubble(pb, row, c);
							return;
						}
					}
				}

				// 3. 아래쪽에서 빈 공간 탐색
				for (int c = 0; c < MAX_COLS; c++) {
					if (bubbleMap[0][c] == -1) {
						placeBubble(pb, 0, c);
						return;
					}
				}

				return;
			}
		}
	}
}

// 버블을 이동시키고 충돌을 처리하는 함수
void moveBubble(Bubble* pb, int x1, int x2) {	// x1 = 276, x2 = 666
	if (pb->active == 0) return;

	double speed = 15.0;
	int margin = 10;
	int diameter = radius * 2;

	// 위치 업데이트
	pb->x += pb->dx * speed;
	pb->y += pb->dy * speed;

	// 벽 충돌 처리
	// 1. 왼쪽 벽에 닿는 경우
	if (pb->x <= x1) {
		pb->x = x1;		// 위치 보정
		pb->dx *= -1;
	}
	// 2. 오른쪽 벽에 닿는 경우
	if ((pb->x + diameter) >= x2) {		// pb->x는 버블의 좌측 상단 모서리의 좌표이므로, 지름을 더해 주어야 올바른 경계 충돌 판정이 가능하다.
		pb->x = x2 - diameter;			// 위치 보정
		pb->dx *= -1;
	}
}

// 슈팅 버블의 상태를 업데이트하는 함수
void updateShootingBubble(Bubble* pb, int x1, int x2) {
	if (pb->active == 0) return;	// 비활성 상태이면 함수 종료

	moveBubble(pb, x1, x2);			// 이동
	manageCollisions(pb);		// 충돌 처리
}

// 깊이 우선 탐색(DFS)를 이용해 연결된 같은 색상의 버블 개수를 세는 함수
int countConnectedBubbles(int row, int col, int targetColor, int bubbleList[][2], int* total) {
	// 특정 위치에서 시작해 같은 색상으로 연결된 버블의 개수를 세고, 좌표를 기록한다.
	// 매개변수: 시작 위치(row, col), 찾을 색상, 버블의 좌표를 저장할 배열, 버블의 개수


	// 배열 경계 벗어나면 탐색 중단
	if (row < 0 || row >= MAX_ROWS || col < 0 || col >= MAX_COLS) {
		return 0;
	}

	// 이미 방문 or 빈 공간 or 타켓 색상과 다르면 탐색 중단
	if (visited[row][col] || bubbleMap[row][col] == -1 || bubbleMap[row][col] != targetColor) {
		return 0;
	}

	// step2: 현재 위치 처리
	visited[row][col] = 1;	// 현재 위치 1(방문 완료)로 마킹
	bubbleList[*total][0] = row;	// 행 좌표 저장
	bubbleList[*total][1] = col;	// 열 좌표 저장
	(*total)++;				// 총 개수 증가

	int count = 1;			// 현재 버블 1개부터 카운트 시작

	// 상하좌우 4방향 탐색
	count += countConnectedBubbles(row - 1, col, targetColor, bubbleList, total); // 위
	count += countConnectedBubbles(row + 1, col, targetColor, bubbleList, total); // 아래
	count += countConnectedBubbles(row, col - 1, targetColor, bubbleList, total); // 왼쪽
	count += countConnectedBubbles(row, col + 1, targetColor, bubbleList, total); // 오른쪽

	return count;	// 연결된 버블의 개수 반환
}

// 버블들을 게임 박스에서 제거하는 함수 
void removeBubbles(int bubbleList[][2], int total) {
	for (int i = 0; i < total; i++) {	// 제거할 모든 버블 개수만큼 반복
		int row = bubbleList[i][0];
		int col = bubbleList[i][1];
		bubbleMap[row][col] = -1; // 빈 공간으로 마킹
	}
}

// 3개 이상 연결된 버블들을 찾아서 제거하는 메인 함수
int removeConnectedBubbles() {
	int totalRemoved = 0; // 현재 turn에 제거된 버블의 총 개수

	// visited 배열 초기화
	for (int i = 0; i < MAX_ROWS; i++) {
		for (int j = 0; j < MAX_COLS; j++) {
			visited[i][j] = 0;	// 미방문(0) 상태
		}
	}

	// 전체 게임판을 스캔하면서 연결된 버블 그룹 찾기
	for (int row = 0; row < MAX_ROWS; row++) {
		for (int col = 0; col < MAX_COLS; col++) {
			// 빈 공간이거나 이미 방문한 곳이면 건너뛰기
			if (bubbleMap[row][col] == -1 || visited[row][col]) {
				continue;
			}

			int targetColor = bubbleMap[row][col];	// 현재 위치의 버블 색상 저장
			int bubbleList[MAX_ROWS * MAX_COLS][2]; // 연결된 버블들의 좌표를 저장할 배열
			int bubbleCount = 0;	// 실제로 저장된 버블의 개수
			int connectedBubbles = countConnectedBubbles(row, col, targetColor, bubbleList, &bubbleCount);			// connectedBubbles는 같은 색상의 버블 개수를 저장하는 변수

			// 5개 이상 연결되어 있으면 제거
			if (connectedBubbles >= 3) {
				//PlaySound(TEXT("pop.wav"), NULL, SND_FILENAME | SND_ASYNC);
				removeBubbles(bubbleList, bubbleCount);		// -1로 마킹
				totalRemoved += connectedBubbles;			// 제거된 총 버블 수 갱신
				if (maxCombo < combo) {	// 최대 콤보 수 갱신
					maxCombo = combo;
				}
			}
		}
	}

	if (totalRemoved > 0) {
		countScore(totalRemoved);  // 점수 업데이트 호출
	}

	return totalRemoved;
}

// 상단과 연결된 버블들을 재귀적으로 방문 처리하는 함수
void visitAllConnectedBubbles(int row, int col) {
	if (row < 0 || row >= MAX_ROWS || col < 0 || col >= MAX_COLS) return;	// 경계 체크
	if (visited[row][col] || bubbleMap[row][col] == -1) return;		// 이미 방문했거나 빈 공간이면 탐색 중단

	visited[row][col] = 1;		// 현재 위치 방문 처리

	// 상하좌우 4방향으로 연결된 모든 버블 방문 처리
	visitAllConnectedBubbles(row - 1, col); // 위
	visitAllConnectedBubbles(row + 1, col); // 아래
	visitAllConnectedBubbles(row, col - 1); // 왼쪽
	visitAllConnectedBubbles(row, col + 1); // 오른쪽
}

// 천장과 연결되지 않은(떠있는) 버블을 제거하는 함수
int removeDisconnectedBubbles() {
	int removed = 0;	// 제거된 버블의 개수

	// visited 배열 초기화
	for (int i = 0; i < MAX_ROWS; i++) {
		for (int j = 0; j < MAX_COLS; j++) {
			visited[i][j] = 0;
		}
	}

	// 천장(첫 번째 줄)과 연결된 모든 버블들을 방문 처리
	for (int col = 0; col < MAX_COLS; col++) {
		if (bubbleMap[0][col] != -1 && !visited[0][col]) {	// 천장에 버블이 있고, 아직 확인하지 않았으면
			visitAllConnectedBubbles(0, col);	// 방문 처리
		}
	}

	// 천장을 제외한 나머지 영역에서 방문하지 않은 버블들 탐색
	for (int row = 1; row < MAX_ROWS; row++) { // 천장 제외
		for (int col = 0; col < MAX_COLS; col++) {
			if (bubbleMap[row][col] != -1 && !visited[row][col]) {
				bubbleMap[row][col] = -1; // 제거
				removed++;		// 제거한 버블 수 증가
			}
		}
	}

	return removed;		// 제거한 총 버블의 개수 리턴
}


// 특정 위치에서 같은 색상으로 연결된 버블 개수를 세는 함수 (실제 배치 전 시뮬레이션 용도)
int howManyConnected(int row, int col, int testColor) {
	// visited 배열 초기화 (모두 미방문 상태로)
	for (int i = 0; i < MAX_ROWS; i++) {
		for (int j = 0; j < MAX_COLS; j++) {
			visited[i][j] = 0;
		}
	}

	// 임시로 해당 위치에 색상 설정
	int originalColor = bubbleMap[row][col];	// 원본 상태 저장
	bubbleMap[row][col] = testColor;			// 임시로 테스트 색상 설정

	// 가상으로 배치한 상태에서 연결된 개수 계산
	int bubbleList[MAX_ROWS * MAX_COLS][2];
	int bubbleCount = 0;
	int connectedCount = countConnectedBubbles(row, col, testColor, bubbleList, &bubbleCount);

	bubbleMap[row][col] = originalColor;	// 원래 색상으로 되돌리기

	return connectedCount;	// 예상한 연결 개수 리턴
}

// 안전한(3개 이상 연결되지 않는) 색상을 선택하는 함수
int chooseColor(int row, int col) {
	int attempts = 0;	// 시도 횟수
	int maxAttempts = availableColors * 3; // 최대 시도 횟수

	// 1. 랜덤 시도로 안전한 색상 탐색
	while (attempts < maxAttempts) {
		int testColor = rand() % availableColors;	// 랜덤한 색상 선택

		// 해당 색상으로 배치했을 때 연결되는 개수 예측
		if (howManyConnected(row, col, testColor) < 3) {	// 5개 미만이면 안전하다고 판단
			return testColor; // 그 색상 반환
		}

		attempts++;	// 시도 횟수 증가
	}

	// 2. 모든 시도가 실패하면 마지막 수단으로 가장 적게 연결되는 색상 선택
	int bestColor = 0;		// 최선의 색상
	int minConnections = MAX_ROWS * MAX_COLS;

	for (int color = 0; color < availableColors; color++) {		// 가능한 색상 가짓수 순회하며 하나씩 테스트
		int connections = howManyConnected(row, col, color);
		if (connections < minConnections) {	// 더 적게 연결되는 색상 발견 시
			minConnections = connections;	// 최소 연결 갱신
			bestColor = color;				// 최선의 색상 갱신
		}
	}

	return bestColor;
}

// 총점을 계산하는 함수
int countScore(int removedBubbles) {
	clock_t current = clock();

	if (current - lastRemoveTime <= comboTime) combo++;	// 3초 내에 연속 제거하면 콤보 중가
	else combo = 1;		// 콤보 리셋

	int baseScore = removedBubbles * 100;	// 버블당 100점
	int finalScore = baseScore * combo;		// 기본 점수 * 콤보 배수

	totalScore += finalScore;	// 점수 합 계산
	lastRemoveTime = current;	// 마지막으로 버블 제거한 시각 = 현재 시각으로 갱신
}

// 게임 박스 상단에 메뉴 바를 그리는 함수
void drawTopMenuBar(HDC memDC) {
	// 1. 메뉴바 픽셀 배경 그리기
	RECT menuRect = { 0, 0, 600, 130 };		// 메뉴바 전체 영역 정의
	HBRUSH menuBrush = CreateSolidBrush(colorList[11]);	// 배경 브러쉬 생성
	FillRect(memDC, &menuRect, menuBrush);	// 영역 채우기
	DeleteObject(menuBrush);	// 브러쉬 해제

	// 2. GDI로 “SCORE / COMBO / TIME” 문자열을 계산 후 가운데 정렬
	// 2-1. 현재 경과 시간 계산
	clock_t current = clock();
	int survivalTime = (int)((current - gameStartTime) / CLOCKS_PER_SEC);	// 생존 시간 = (현재 시각 - 스타트 시각)
	minutes = survivalTime / 60;
	seconds = survivalTime % 60;

	// 2-2. 각 문자를 '와이드 문자열'로 포맷팅
	wchar_t scoreW[64];	// 현재 점수
	wchar_t comboW[32];	// 현재 콤보 수
	wchar_t timeW[16];	// 분:초 형식의 생존 시간

	swprintf(scoreW, 64, L"SCORE : %d", totalScore);
	swprintf(comboW, 32, L"COMBO : x%d", combo);
	swprintf(timeW, 16, L"%02d:%02d", minutes, seconds);

	// 2-3. 원하는 폰트와 크기 생성
	HFONT hFont = CreateFontW(	// Consolas 폰트, 크기 24, 굵게 설정으로 폰트 생성
		24, 0, 0, 0,
		FW_BOLD,
		FALSE, FALSE, FALSE,
		ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Consolas");
	HFONT oldFont = (HFONT)SelectObject(memDC, hFont);	// 현재 폰트를 새 폰트로 교체, 이전 폰트 저장

	// 2-4. 가독성을 위해 색 지정 
	SetTextColor(memDC, colorList[12]);
	SetBkMode(memDC, TRANSPARENT);		// 배경색이 비치도록 텍스트의 배경을 투명하게 설정	

	// 2-5. 문자열 별로 크기 구해서 가운데 맞춤(x, y)
	// 현재 폰트로 렌더링될 때의 픽셀 크기 계산
	SIZE sizeScore, sizeCombo, sizeTime;
	GetTextExtentPoint32W(memDC, scoreW, (int)wcslen(scoreW), &sizeScore);
	GetTextExtentPoint32W(memDC, comboW, (int)wcslen(comboW), &sizeCombo);
	GetTextExtentPoint32W(memDC, timeW, (int)wcslen(timeW), &sizeTime);

	// 2-6. 3등분하여 각 문자열의 위치 계산 (너비: 600px, 높이: 130px)
	int sectionWidth = 600 / 3; // 메뉴바 3등분 -> 각 섹션의 너비는 200px

	// 각 섹션 내에서 텍스트를 가운데 정렬하기 위하여 X 좌표 계산
	int scoreSectionX = (sectionWidth - sizeScore.cx) / 2;                    // 0~200px 구역
	int comboSectionX = sectionWidth + ((sectionWidth - sizeCombo.cx) / 2);   // 200~400px 구역  
	int timeSectionX = 2 * sectionWidth + ((sectionWidth - sizeTime.cx) / 2); // 400~600px 구역

	// 수직 중앙 정렬을 위한 Y 좌표 계산 (메뉴바 높이 130px 기준)
	int textY = (130 - sizeScore.cy) / 2;

	// 2-7. 실제 출력
	TextOutW(memDC, scoreSectionX, textY, scoreW, (int)wcslen(scoreW));	// 점수 표시
	TextOutW(memDC, comboSectionX, textY, comboW, (int)wcslen(comboW));	// 콤보 수 표시
	TextOutW(memDC, timeSectionX, textY, timeW, (int)wcslen(timeW));	// 생존시간 표시

	// 2-8. 메모리 해제
	SelectObject(memDC, oldFont);
	DeleteObject(hFont);
}

void enableConsoleUTF8() {
	// < 콘솔을 UTF-8 모드로 설정하는 이유 >
	// 1) Windows 기본 콘솔은 ANSI(멀티바이트) 코드 페이지를 사용하므로 한글/유니코드 출력 시 깨짐이 발생할 수 있다.
	// 2) SetConsoleOutputCP(CP_UTF8) 호출 후 WriteConsoleW()를 사용하면 
	//	 내부적으로 UTF-8 -> UTF-16 변환 없이 바로 유니코드 코드 포인트를 전달하므로,
	//   한글 및 기타 유니코드 문자도 정확히 표시된다.
	// 3) 단, 콘솔 속성에서 유니코드를 지원하는 글꼴(예: Consolas, Lucida Console 등)으로 
	//   설정해 두어야 제대로 보인다.

	// 출력용 코드 페이지를 UTF-8(65001)로 설정
	SetConsoleOutputCP(CP_UTF8);

	// 콘솔 모드에서 유니코드 출력 허용
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	DWORD mode = 0;

	GetConsoleMode(hOut, &mode);
	SetConsoleMode(hOut, mode | ENABLE_PROCESSED_OUTPUT | ENABLE_WRAP_AT_EOL_OUTPUT);	// ENABLE_PROCESSED_OUTPUT: 콘솔 처리 출력, ENABLE_WRAP_AT_EOL_OUTPUT: 끝까지 가면 줄바꿈

	// < 콘솔에 텍스트를 출력할 때 와이드 문자열을 쓰는 이유 >
	// 1) ANSI 출력(printf, WriteConsoleA)은 현재 콘솔 코드 페이지(CP)에 의존하며,
	//    한글이나 다국어 문자가 깨질 수 있다.
	// 2) WriteConsoleW()는 UTF-16 기반으로 유니코드 코드 포인트를 직접 보내므로,
	//    콘솔 코드 페이지 설정과 상관없이 한글 등 모든 유니코드 문자를 정확히 찍어 준다.
	// 3) 문자열 길이를 wcslen()으로 계산하고, 반드시 Consolas·Lucida Console 등
	//    유니코드를 지원하는 폰트를 콘솔 속성에서 선택해 두어야 한다.
	// 4) ANSI/Unicode 혼용 시 인코딩 오류가 발생하므로, 같은 함수 내에서는
	//    와이드 출력 방식만 사용하도록 통일한다.
}

void gotoConsoleXY(int x, int y) {
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);	// 윈도우 콘솔의 출력 스트림 가져오기
	COORD pos = { (int)x, (int)y };		// short형 x, y를 멤버로 갖는 구조체 COORD
	SetConsoleCursorPosition(hOut, pos);	// 화면 위에서 커서 이동시키는 API. 콘솔 윈도우의 현재 출력 위치를 (x, y) 셀로 변경하는 역할
}

void writeConsoleText(int x, int y, WORD color, const wchar_t* wstr, DWORD len) {
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);	// 표준 입출력용 객체의 핸들 받아와서 hOut에 저장

	SetConsoleTextAttribute(hOut, color);	// 색상 속성 설정. hOut이 가리키는 출력 버퍼(화면)에 문자 색상과 배경색 속성 적용
	gotoConsoleXY(x, y);	// 콘솔의 현재 커서 위치를 x, y로 이동

	DWORD written = 0;		// 실제 출력한 글자의 개수를 저장하기 위한 변수. 0으로 초기화
	WriteConsoleW(hOut, wstr, len, &written, NULL);	// 콘솔에 유니코드(wcahr_t) 문자열을 출력하기 위한 윈도우 API 함수 사용

	// WriteConsoleW 호출 횟수가 많으면 콘솔 셀 단위 깜빡임이 생길 수 있으므로, 
	// 이를 방지하기 위해 main()에서 더블 버퍼링 방식을 사용한다.
}


// 화면에 "GAME OVER" 메시지와 하단 통계(총점수, 최대 콤보, 생존 시간)를 깜빡이며 출력하는 함수
void printGameOver(HDC hdc, HDC memDC, int screenWidth, int screenHeight) {
	// 1. 전체 배경을 딥 네이비(colorList[13])로 채우기
	RECT backgroundRect = { 0, 0, screenWidth, screenHeight };
	HBRUSH bgBrush = CreateSolidBrush(colorList[13]);
	FillRect(memDC, &backgroundRect, bgBrush);
	DeleteObject(bgBrush);

	// 2-1. 큰 글꼴 생성: Impact, 높이 100, ExtraBold
	HFONT hFontBig = CreateFontW(
		100, 0, 0, 0, FW_EXTRABOLD, FALSE, FALSE, FALSE,
		ANSI_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS,
		ANTIALIASED_QUALITY, FF_DONTCARE, L"Impact"
	);
	// 2-2. 작은 글꼴 생성: Impact, 높이 24, Normal
	HFONT hFontSmall = CreateFontW(
		24, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
		ANSI_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS,
		ANTIALIASED_QUALITY, FF_DONTCARE, L"Impact"
	);

	// 2-3. 통계 문자열 포맷
	wchar_t statsText[128];
	swprintf(
		statsText, 128,
		L"SCORE : %d    MAX COMBO : x%d    TIME : %02d:%02d",
		totalScore, maxCombo, minutes, seconds
	);

	// 2-4. 타이머 초기화 (5초 동안 깜빡임)
	DWORD startTime = GetTickCount64();
	DWORD lastBlink = startTime;
	int visible = 1;	// 가시성 여부

	// 3. 루프: 5초 동안 0.35초 간격으로 깜빡이며 출력
	while (GetTickCount64() - startTime < 15000) {
		DWORD now = GetTickCount64();
		// 3-1. 0.5초마다 visible 토글
		if (now - lastBlink >= 350) {
			visible = visible > 0 ? 0 : 1;
			lastBlink = now;
		}

		// 3-2. 배경 클리어
		HBRUSH clr = CreateSolidBrush(colorList[13]);
		FillRect(memDC, &backgroundRect, clr);
		DeleteObject(clr);

		if (visible) {
			// 3-3. 그림자 효과
			SelectObject(memDC, hFontBig);
			SetTextColor(memDC, RGB(0, 195, 195));
			SetBkMode(memDC, TRANSPARENT);
			RECT shadowRect = { 0, 10, screenWidth, screenHeight - 120 + 10 };
			DrawTextW(memDC, L"GAME OVER !", -1, &shadowRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

			// 3-4. 본문 타이틀
			SetTextColor(memDC, colorList[14]);
			RECT titleRect = { 0, 10, screenWidth, screenHeight - 120 };
			DrawTextW(memDC, L"GAME OVER !", -1, &titleRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
		}

		// 3-5. 하단 통계 (깜빡임 없이)
		SelectObject(memDC, hFontSmall);
		SetTextColor(memDC, colorList[12]);
		SetBkMode(memDC, TRANSPARENT);
		RECT statsRect = { 0, screenHeight - 800, screenWidth - 60, screenHeight };
		DrawTextW(memDC, statsText, -1, &statsRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

		// 3-6. 백버퍼 → 실제 화면
		BitBlt(hdc, 0, 0, screenWidth, screenHeight, memDC, 0, 0, SRCCOPY);

		// 3-7. 약간 대기 
		Sleep(16);
	}

	// 4-1. 작은 글꼴 해제
	DeleteObject(SelectObject(memDC, hFontSmall));

	// 4-2. 큰 글꼴 해제
	DeleteObject(SelectObject(memDC, hFontBig));
}


// 게임 상태 초기화 함수
void resetGame(int* currentLine, clock_t* lastAddTime, Bubble* pb) {
	// 1. 버블 맵 초기화 (상단 5줄 랜덤 색)
	initBubbles();

	// 2. 점수, 콤보, 시간 리셋
	totalScore = 0;
	combo = 0;
	lastRemoveTime = 0;
	gameStartTime = clock();

	// 3. 현재 라인 수 및 버블 추가 타이머 리셋
	*currentLine = 5;
	*lastAddTime = clock();

	// 4. 발사 중 버블 초기화
	pb->active = 0;
	pb->colorIndex = rand() % availableColors;

	// 5. 콘솔 커서 숨김 (게임 재시작 시 필요)
	removeCursor();
}

// 게임 박스 하단에 RESET / EXIT 메시지를 출력하는 함수
void drawBottomMenuBar(HDC memDC, int x1, int y2, int x2) {
	// 1. 출력할 문자열 준비
	wchar_t resetW[] = L"RESET : Press R or r";
	wchar_t exitW[] = L"EXIT : Press ESC";

	// 2. 폰트 생성 (Consolas, 20pt, 보통 굵기)
	HFONT hFontPrompt = CreateFontW(
		20, 0, 0, 0,
		FW_NORMAL,
		FALSE, FALSE, FALSE,
		ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		ANTIALIASED_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Consolas"
	);
	HFONT oldFont = (HFONT)SelectObject(memDC, hFontPrompt);

	// 3. 텍스트 배경 투명
	SetBkMode(memDC, TRANSPARENT);

	// 4. 문자열 크기 계산
	SIZE sizeReset, sizeExit;
	GetTextExtentPoint32W(memDC, resetW, (int)wcslen(resetW), &sizeReset);
	GetTextExtentPoint32W(memDC, exitW, (int)wcslen(exitW), &sizeExit);

	// 5. 수직 그리드
	int promptY = y2 + 48;
	if (promptY + sizeReset.cy > screenHeight) {
		// 화면을 벗어나지 않도록 약간 올림
		promptY = screenHeight - sizeReset.cy - 10;
	}

	// 6. RESET/EXIT 각각 색상 지정
	// RESET : 옐로우 포인트,  EXIT  : 레드 포인트
	SetTextColor(memDC, colorList[5]);	// (둘 다 한 줄에 좌우로 나란히 표시하므로, 각각 따로 색상을 지정하며 출력)

	// 7. X 좌표 계산 (게임박스 x1~x2 영역의 가운데 기준)
	int boxWidth = x2 - x1;
	// RESET은 박스 중앙( x1 + boxWidth/2 )보다 왼쪽으로 sizeReset.cx + 55px 만큼 이동해야 한다.
	int centerX = x1 + (boxWidth / 2);
	int resetX = centerX - sizeReset.cx - 20;
	int exitX = centerX + 55;

	// 8. 실제 텍스트 출력
	TextOutW(memDC, resetX, promptY, resetW, (int)wcslen(resetW));

	// EXIT를 그리기 전에 색상만 변경
	SetTextColor(memDC, colorList[0]);
	TextOutW(memDC, exitX, promptY, exitW, (int)wcslen(exitW));

	// 9. 폰트 복구 및 해제
	SelectObject(memDC, oldFont);
	DeleteObject(hFontPrompt);
}
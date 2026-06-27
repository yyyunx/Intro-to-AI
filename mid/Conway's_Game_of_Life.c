#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// 如果是 Windows 平台
#ifdef _WIN32
#include <windows.h>
#define SLEEP(ms) Sleep(ms)  // Windows 的 sleep 函式

// gotoxy：移動游標到 (x, y)
void gotoxy(int x, int y) {
    COORD pos = {x, y};
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}
#else
// 如果是 Linux 或 macOS 平台
#include <unistd.h>
#define SLEEP(ms) usleep((ms) * 1000)  // usleep 單位為微秒

void gotoxy(int x, int y) {
    printf("\033[%d;%dH", y + 1, x + 1);
    fflush(stdout);
}
#endif

#define ROW 40
#define COL 70
#define DELAY 250

void initBoard(int board[ROW][COL], int mode);
void displayBoard(int board[ROW][COL], int generation);
int countNeighbors(int board[ROW][COL], int x, int y);
void updateBoard(int board[ROW][COL]);

int main() {
    int board[ROW][COL];
    int mode = 0;

    printf("===== 康威生命遊戲 (Conway's Game of Life) =====\n");
    printf("請選擇模式：\n");
    printf("1. 自訂初始細胞\n");
    printf("2. 隨機模式（起點隨機）\n");
    printf("3. 蜂巢\n");
    printf("4. 震盪器\n");
    printf("5. 滑翔機槍\n");
    printf("選擇：");
    scanf("%d", &mode);
    getchar(); // 清除換行符號

    system("cls");
    initBoard(board, mode);

    int generation = 0;
    while (1) {
        gotoxy(0, 0);
        displayBoard(board, generation);
        updateBoard(board);
        generation++;
        SLEEP(DELAY);
    }

    return 0;
}

// 初始化棋盤
void initBoard(int board[ROW][COL], int mode) {
    srand(time(NULL));

    // 清空棋盤
    for (int i = 0; i < ROW; i++)
        for (int j = 0; j < COL; j++)
            board[i][j] = 0;

    if (mode == 1) {
        // 自訂模式
        int x, y;
        char line[100];
        printf("輸入活細胞座標 (x y)，輸入 -1 -1 結束：\n");
        while (1) {
            printf(">> ");
            if (!fgets(line, sizeof(line), stdin)) continue;
            if (sscanf(line, "%d %d", &x, &y) != 2) continue;
            if (x == -1 && y == -1) break;
            if (x >= 0 && x < COL && y >= 0 && y < ROW)
                board[y][x] = 1;
            else
                printf("? 座標超出範圍！（x: 0~%d, y: 0~%d）\n", COL - 1, ROW - 1);
        }
    } 
    else if (mode == 2) {
        // 隨機模式
        double density = 0.10;
        for (int i = 0; i < ROW; i++)
            for (int j = 0; j < COL; j++)
                board[i][j] = (rand() / (double)RAND_MAX < density) ? 1 : 0;
    }
	else if (mode == 3) {
        // 蜂巢模式（多種靜止蜂巢）
        int type = rand() % 2; // 0~1
        int x, y;

        switch (type) {
            case 0: // 小蜂巢
                x = 2 + rand() % (COL - 6);
                y = 2 + rand() % (ROW - 5);
                board[y][x+1] = 1; board[y][x+2] = 1;
                board[y+1][x] = 1; board[y+1][x+3] = 1;
                board[y+2][x+1] = 1; board[y+2][x+2] = 1;
                break;

        	case 1: // 網格蜂巢（多個小蜂巢間隔均勻）
        	    x = 2 + rand() % (COL - 20);
        	    y = 2 + rand() % (ROW - 10);
        	    for (int r = 0; r < 2; r++) {
        	        for (int c = 0; c < 3; c++) {
        	            int ox = x + c * 6;
        	            int oy = y + r * 4;
        	            board[oy][ox+1] = 1; board[oy][ox+2] = 1;
        	            board[oy+1][ox] = 1; board[oy+1][ox+3] = 1;
        	            board[oy+2][ox+1] = 1; board[oy+2][ox+2] = 1;
        	        }
        	    }
        	    break;
        }
    }
	else if (mode == 4) {
	    // 振盪器模式
	    // 放置 Blinker、Toad、Beacon、Pulsar 四種振盪器
	    int margin = 4;
	
	    // 1 Blinker (左上角)
	    int bx = margin + 2;
	    int by = margin + 2;
	    board[bx][by] = 1;
	    board[bx][by + 1] = 1;
	    board[bx][by + 2] = 1;

    // 2 Toad (右上角)
    	int tx = margin + 2;
    	int ty = COL - margin - 6;
    	board[tx][ty] = 1;
    	board[tx][ty + 1] = 1;
    	board[tx][ty + 2] = 1;
    	board[tx + 1][ty - 1] = 1;
    	board[tx + 1][ty] = 1;
    	board[tx + 1][ty + 1] = 1;
	
	    // 3 Beacon (右下角)
	    int bx2 = ROW - margin - 6;
	    int by2 = COL - margin - 6;
	    board[bx2][by2] = 1; 
	    board[bx2][by2 + 1] = 1;
	    board[bx2 + 1][by2] = 1; 
		board[bx2 + 1][by2 + 1] = 1;
   		board[bx2 + 2][by2 + 2] = 1; 
   		board[bx2 + 2][by2 + 3] = 1;
    	board[bx2 + 3][by2 + 2] = 1; 
    	board[bx2 + 3][by2 + 3] = 1;
	
	    // 4 Pulsar (中間)
	    int x = ROW / 2 - 6;
	    int y = COL / 2 - 6;
	    int pulsar[][2] = {
	        {x+2, y+0}, {x+3, y+0}, {x+4, y+0}, {x+8, y+0}, {x+9, y+0}, {x+10, y+0},
	        {x+0, y+2}, {x+5, y+2}, {x+7, y+2}, {x+12, y+2},
	        {x+0, y+3}, {x+5, y+3}, {x+7, y+3}, {x+12, y+3},
	        {x+0, y+4}, {x+5, y+4}, {x+7, y+4}, {x+12, y+4},
	        {x+2, y+5}, {x+3, y+5}, {x+4, y+5}, {x+8, y+5}, {x+9, y+5}, {x+10, y+5},

	        {x+2, y+7}, {x+3, y+7}, {x+4, y+7}, {x+8, y+7}, {x+9, y+7}, {x+10, y+7},
	        {x+0, y+8}, {x+5, y+8}, {x+7, y+8}, {x+12, y+8},
	        {x+0, y+9}, {x+5, y+9}, {x+7, y+9}, {x+12, y+9},
	        {x+0, y+10}, {x+5, y+10}, {x+7, y+10}, {x+12, y+10},
	        {x+2, y+12}, {x+3, y+12}, {x+4, y+12}, {x+8, y+12}, {x+9, y+12}, {x+10, y+12}
	    };
	
		int n = sizeof(pulsar) / sizeof(pulsar[0]);
    	for (int i = 0; i < n; i++) {
   	    	int px = pulsar[i][0], py = pulsar[i][1];
        	if (px >= 0 && px < ROW && py >= 0 && py < COL)
        	    board[px][py] = 1;
    	}
	}

    else if (mode == 5) {
        // 滑翔機槍 (Gosper Glider Gun)
        int x = 1, y = 1;
        int pattern[][2] = {
            {x+24, y+0}, {x+22, y+1}, {x+24, y+1},
            {x+12, y+2}, {x+13, y+2}, {x+20, y+2}, {x+21, y+2}, {x+34, y+2}, {x+35, y+2},
            {x+11, y+3}, {x+15, y+3}, {x+20, y+3}, {x+21, y+3}, {x+34, y+3}, {x+35, y+3},
            {x+0, y+4}, {x+1, y+4}, {x+10, y+4}, {x+16, y+4}, {x+20, y+4}, {x+21, y+4},
            {x+0, y+5}, {x+1, y+5}, {x+10, y+5}, {x+14, y+5}, {x+16, y+5}, {x+17, y+5}, {x+22, y+5}, {x+24, y+5},
            {x+10, y+6}, {x+16, y+6}, {x+24, y+6},
            {x+11, y+7}, {x+15, y+7},
            {x+12, y+8}, {x+13, y+8}
        };
        int n = sizeof(pattern) / sizeof(pattern[0]);
        for (int i = 0; i < n; i++) {
            int px = pattern[i][0], py = pattern[i][1];
            if (px < COL && py < ROW)
                board[py][px] = 1;
        }
    }
}

// 顯示棋盤
void displayBoard(int board[ROW][COL], int generation) {
    printf("第 %d 代\n", generation);
    for (int i = 0; i < ROW; i++) {
        for (int j = 0; j < COL; j++)
            putchar(board[i][j] ? '#' : '.');
        putchar('\n');
    }
    fflush(stdout);
}

// 計算鄰居數
int countNeighbors(int board[ROW][COL], int x, int y) {
    int count = 0;
    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            if (i == 0 && j == 0) continue;
            int nx = x + i, ny = y + j;
            if (nx >= 0 && nx < ROW && ny >= 0 && ny < COL)
                count += board[nx][ny];
        }
    }
    return count;
}

// 更新棋盤
void updateBoard(int board[ROW][COL]) {
    int newBoard[ROW][COL] = {0};
    for (int i = 0; i < ROW; i++) {
        for (int j = 0; j < COL; j++) {
            int neighbors = countNeighbors(board, i, j);
            if (board[i][j]) {
                if (neighbors == 2 || neighbors == 3)
                    newBoard[i][j] = 1;
            } else {
                if (neighbors == 3)
                    newBoard[i][j] = 1;
            }
        }
    }
    for (int i = 0; i < ROW; i++)
        for (int j = 0; j < COL; j++)
            board[i][j] = newBoard[i][j];
}



